"""Config loading, derived constants and steering-vector sanity checks."""

from pathlib import Path

import numpy as np
import pytest

from arraydsp import geometry
from arraydsp.config import config_from_dict, load_config

CONFIGS = Path(__file__).parent.parent / "configs"


def test_board_8mic_derived_constants():
    d = load_config(CONFIGS / "board_8mic.yaml").derived
    assert len(d.positions) == 8
    assert d.adjacent_spacing_m == pytest.approx(42.1e-3, abs=0.05e-3)
    assert d.aliasing_hz == pytest.approx(4074, abs=1)
    assert d.max_delay_s == pytest.approx(321e-6, abs=0.5e-6)
    assert d.max_delay_samples_bf == pytest.approx(15.4, abs=0.05)
    assert d.max_delay_samples_doa == pytest.approx(5.1, abs=0.05)
    assert d.ka1_hz == pytest.approx(1000, abs=10)
    assert d.das_wng_db == pytest.approx(9.03, abs=0.01)
    assert d.bf_band_hz[1] == d.aliasing_hz   # band_max_hz: auto


def test_proto_4mic_loads():
    cfg = load_config(CONFIGS / "proto_4mic.yaml")
    assert cfg.n_mics == 4 and cfg.positions.shape == (4, 3)


@pytest.mark.parametrize("n", [2, 3, 4, 6, 8, 16])
def test_uca_derived_constants_scale_with_n(n):
    r, c = 0.055, 343.0
    d = config_from_dict({"array": {"n_mics": n, "type": "uca", "radius_m": r}}).derived
    assert d.adjacent_spacing_m == pytest.approx(2 * r * np.sin(np.pi / n))
    assert d.aliasing_hz == pytest.approx(c / (4 * r * np.sin(np.pi / n)))
    assert np.allclose(np.linalg.norm(d.positions, axis=1), r)


def test_custom_positions_match_uca():
    pos = geometry.uca_positions(4, 0.05)
    custom = config_from_dict({"array": {"n_mics": 4, "type": "custom", "positions_m": pos[:, :2].tolist()}})
    assert np.allclose(custom.positions, pos)


@pytest.mark.parametrize("bad", [
    {"stft": {"fft_size": 500}},
    {"stft": {"fft_size": 8192}},
    {"beamformer": {"type": "gsc"}},
    {"array": {"n_mics": 4, "type": "uca", "radius_m": 0.05, "channel_map": [0, 1, 1, 3]}},
    {"array": {"n_mics": 4, "type": "uca", "radius_m": 0.05, "raduis_m": 0.05}},   # typo
])
def test_invalid_config_raises(bad):
    raw = {"array": {"n_mics": 4, "type": "uca", "radius_m": 0.05}, **bad}
    with pytest.raises(ValueError):
        config_from_dict(raw)


def test_farfield_steering_unit_gain_toward_look_direction():
    pos = geometry.uca_positions(8, 0.055)
    freqs = np.array([300.0, 1000.0, 4000.0])
    az = np.deg2rad(np.arange(0, 360, 15))
    d = geometry.farfield_steering(pos, freqs[:, None], az[None, :])
    assert d.shape == (3, len(az), 8)
    assert np.allclose(np.abs(d), 1.0)
    w = d / 8                                         # DAS weights
    assert np.allclose(np.sum(w.conj() * d, axis=-1), 1.0)


def test_farfield_phase_sign():
    # A mic displaced toward the source hears the wave first, i.e. a phase lead.
    pos = np.array([[0.1, 0.0, 0.0]])
    d = geometry.farfield_steering(pos, 1000.0, 0.0)
    assert np.angle(d[0]) == pytest.approx(2 * np.pi * 1000.0 * 0.1 / 343.0)


def test_nearfield_approaches_farfield():
    pos = geometry.uca_positions(8, 0.055)
    az = np.deg2rad(40.0)
    far = geometry.farfield_steering(pos, 2000.0, az)
    near = geometry.nearfield_steering(pos, 2000.0, geometry.source_position(az, 1e4))
    assert np.allclose(near, far, atol=1e-4)


def test_diffuse_coherence():
    pos = geometry.uca_positions(8, 0.055)
    gamma = geometry.diffuse_coherence(pos, np.array([1e-3, 1000.0]))
    assert gamma.shape == (2, 8, 8)
    assert np.allclose(gamma[0], 1.0, atol=1e-9)              # f -> 0: fully coherent
    assert np.allclose(np.diagonal(gamma[1]), 1.0)
    assert np.allclose(gamma[1], gamma[1].T)
    dist = geometry.pairwise_distances(pos)
    k = 2 * np.pi * 1000.0 / 343.0
    assert gamma[1, 0, 1] == pytest.approx(np.sin(k * dist[0, 1]) / (k * dist[0, 1]))
