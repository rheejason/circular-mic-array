"""YAML config -> typed dataclasses, plus quantities derived from the geometry.

Every algorithm takes its geometry and parameters from a Config, so nothing assumes a
particular mic count. Derived quantities are computed here at load time and logged; they
are never stored in the YAML.
"""

from __future__ import annotations

import dataclasses
import logging
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import yaml

from arraydsp import geometry

log = logging.getLogger(__name__)

ARRAY_TYPES = ("uca", "custom")
WINDOWS = ("hann", "sqrt_hann", "hamming")
BEAMFORMER_TYPES = ("das", "superdirective", "mvdr_adaptive")
DOA_METHODS = ("gcc_phat", "srp_phat")
FFT_MIN, FFT_MAX = 32, 4096   # arm_rfft_fast_f32 supported sizes


@dataclass(frozen=True)
class ArrayConfig:
    n_mics: int
    type: str = "uca"
    radius_m: float | None = None
    angle_offset_deg: float = 0.0
    positions_m: list | None = None
    channel_map: list[int] | None = None   # channel_map[ch] = mic index; None = identity


@dataclass(frozen=True)
class AudioConfig:
    fs_beamformer: int = 48000
    fs_doa: int = 16000
    bit_depth: int = 16
    c: float = 343.0


@dataclass(frozen=True)
class StftConfig:
    fft_size: int = 512
    hop: int = 256
    window: str = "sqrt_hann"


@dataclass(frozen=True)
class BeamformerConfig:
    type: str = "superdirective"
    wng_floor_db: float = -10.0
    band_min_hz: float = 100.0
    band_max_hz: float | None = None   # None ("auto") = aliasing onset
    crossfade_ms: float = 30.0


@dataclass(frozen=True)
class DoaConfig:
    method: str = "srp_phat"
    azimuth_grid_deg: float = 5.0
    freq_min_hz: float = 300.0
    freq_max_hz: float | None = None   # None ("auto") = min(aliasing onset, fs_doa / 2)
    update_rate_hz: float = 10.0


@dataclass(frozen=True)
class SourceConfig:
    azimuth_deg: float
    distance_m: float
    height_m: float = 1.2


@dataclass(frozen=True)
class SimConfig:
    room_dim_m: list[float] = dataclasses.field(default_factory=lambda: [6.0, 5.0, 3.0])
    rt60_s: float = 0.4
    array_center_m: list[float] = dataclasses.field(default_factory=lambda: [3.0, 2.5, 0.75])
    snr_db: float = 20.0
    sources: list[SourceConfig] = dataclasses.field(default_factory=list)


@dataclass(frozen=True, eq=False)
class Derived:
    """Quantities computed from the config. Never written back to YAML."""
    positions: np.ndarray          # (N, 3) metres, mic-index order
    adjacent_spacing_m: float
    aliasing_hz: float
    aperture_m: float
    max_delay_s: float
    max_delay_samples_bf: float    # at fs_beamformer
    max_delay_samples_doa: float   # at fs_doa
    ka1_hz: float
    bf_band_hz: tuple[float, float]
    doa_band_hz: tuple[float, float]
    das_wng_db: float              # 10 log10(N), the WNG ceiling for any beamformer

    def summary(self) -> str:
        n = len(self.positions)
        return (
            f"N={n}  spacing={self.adjacent_spacing_m * 1e3:.1f} mm  "
            f"aliasing={self.aliasing_hz:.0f} Hz  aperture={self.aperture_m * 1e3:.1f} mm  "
            f"max delay={self.max_delay_s * 1e6:.0f} us "
            f"({self.max_delay_samples_bf:.1f} smp bf, {self.max_delay_samples_doa:.1f} smp doa)  "
            f"ka=1 at {self.ka1_hz:.0f} Hz  "
            f"bf band={self.bf_band_hz[0]:.0f}-{self.bf_band_hz[1]:.0f} Hz  "
            f"doa band={self.doa_band_hz[0]:.0f}-{self.doa_band_hz[1]:.0f} Hz"
        )


@dataclass(frozen=True, eq=False)
class Config:
    array: ArrayConfig
    audio: AudioConfig
    stft: StftConfig
    beamformer: BeamformerConfig
    doa: DoaConfig
    sim: SimConfig
    derived: Derived

    @property
    def n_mics(self) -> int:
        return self.array.n_mics

    @property
    def positions(self) -> np.ndarray:
        return self.derived.positions


# ---------------------------------------------------------------------------
# Loading
# ---------------------------------------------------------------------------

def load_config(path: str | Path) -> Config:
    """Load a YAML config file, validate it and compute derived quantities."""
    with open(path) as f:
        raw = yaml.safe_load(f)
    cfg = config_from_dict(raw)
    log.info("%s: %s", Path(path).name, cfg.derived.summary())
    return cfg


def config_from_dict(raw: dict) -> Config:
    """Build a Config from a plain dict (the parsed YAML). Useful for tests and sweeps."""
    raw = dict(raw)
    unknown = set(raw) - {"array", "audio", "stft", "beamformer", "doa", "sim"}
    if unknown:
        raise ValueError(f"unknown config sections: {sorted(unknown)}")
    if "array" not in raw:
        raise ValueError("config needs an 'array' section")

    array = _build(ArrayConfig, raw["array"], "array")
    audio = _build(AudioConfig, raw.get("audio", {}), "audio")
    stft = _build(StftConfig, raw.get("stft", {}), "stft")
    beamformer = _build(BeamformerConfig, raw.get("beamformer", {}), "beamformer")
    doa = _build(DoaConfig, raw.get("doa", {}), "doa")
    sim_raw = dict(raw.get("sim", {}))
    sim_raw["sources"] = [_build(SourceConfig, s, "sim.sources") for s in sim_raw.get("sources", [])]
    sim = _build(SimConfig, sim_raw, "sim")

    _validate(array, audio, stft, beamformer, doa)
    derived = _derive(array, audio, beamformer, doa)
    return Config(array, audio, stft, beamformer, doa, sim, derived)


def _build(cls, section: dict | None, name: str):
    section = {k: (None if v == "auto" else v) for k, v in (section or {}).items()}
    fields = {f.name for f in dataclasses.fields(cls)}
    unknown = set(section) - fields
    if unknown:
        raise ValueError(f"unknown keys in '{name}': {sorted(unknown)}")
    return cls(**section)


def _validate(array: ArrayConfig, audio: AudioConfig, stft: StftConfig,
              bf: BeamformerConfig, doa: DoaConfig) -> None:
    n = array.n_mics
    if n < 2:
        raise ValueError(f"n_mics must be >= 2, got {n}")
    if array.type not in ARRAY_TYPES:
        raise ValueError(f"array.type must be one of {ARRAY_TYPES}, got {array.type!r}")
    if array.type == "uca":
        if array.radius_m is None or array.radius_m <= 0:
            raise ValueError("array.type 'uca' needs a positive radius_m")
        if array.positions_m is not None:
            raise ValueError("array.positions_m is only used with type 'custom'")
    if array.type == "custom":
        if array.positions_m is None or len(array.positions_m) != n:
            raise ValueError(f"array.type 'custom' needs {n} entries in positions_m")
    if array.channel_map is not None and sorted(array.channel_map) != list(range(n)):
        raise ValueError(f"channel_map must be a permutation of 0..{n - 1}, got {array.channel_map}")

    if audio.fs_beamformer % audio.fs_doa != 0:
        raise ValueError("fs_beamformer must be an integer multiple of fs_doa")

    f = stft.fft_size
    if f < FFT_MIN or f > FFT_MAX or f & (f - 1):
        raise ValueError(f"stft.fft_size must be a power of 2 in [{FFT_MIN}, {FFT_MAX}], got {f}")
    if not 0 < stft.hop <= f:
        raise ValueError(f"stft.hop must be in (0, fft_size], got {stft.hop}")
    if stft.window not in WINDOWS:
        raise ValueError(f"stft.window must be one of {WINDOWS}, got {stft.window!r}")

    if bf.type not in BEAMFORMER_TYPES:
        raise ValueError(f"beamformer.type must be one of {BEAMFORMER_TYPES}, got {bf.type!r}")
    if doa.method not in DOA_METHODS:
        raise ValueError(f"doa.method must be one of {DOA_METHODS}, got {doa.method!r}")
    if doa.azimuth_grid_deg <= 0 or 360.0 % doa.azimuth_grid_deg:
        raise ValueError(f"doa.azimuth_grid_deg must divide 360, got {doa.azimuth_grid_deg}")


def _derive(array: ArrayConfig, audio: AudioConfig, bf: BeamformerConfig,
            doa: DoaConfig) -> Derived:
    if array.type == "uca":
        pos = geometry.uca_positions(array.n_mics, array.radius_m, array.angle_offset_deg)
    else:
        pos = geometry.as_positions(array.positions_m)

    c = audio.c
    alias = geometry.aliasing_onset_hz(pos, c)
    tau = geometry.max_delay_s(pos, c)

    bf_hi = alias if bf.band_max_hz is None else bf.band_max_hz
    doa_hi = min(alias, audio.fs_doa / 2) if doa.freq_max_hz is None else doa.freq_max_hz
    if not 0 <= bf.band_min_hz < bf_hi <= audio.fs_beamformer / 2:
        raise ValueError(f"bad beamformer band {bf.band_min_hz}-{bf_hi} Hz")
    if not 0 <= doa.freq_min_hz < doa_hi <= audio.fs_doa / 2:
        raise ValueError(f"bad DOA band {doa.freq_min_hz}-{doa_hi} Hz")

    das_wng_db = 10.0 * np.log10(array.n_mics)
    if bf.wng_floor_db > das_wng_db:
        raise ValueError(f"wng_floor_db {bf.wng_floor_db} dB exceeds the DAS ceiling {das_wng_db:.2f} dB")

    return Derived(
        positions=pos,
        adjacent_spacing_m=geometry.adjacent_spacing(pos),
        aliasing_hz=alias,
        aperture_m=geometry.aperture(pos),
        max_delay_s=tau,
        max_delay_samples_bf=tau * audio.fs_beamformer,
        max_delay_samples_doa=tau * audio.fs_doa,
        ka1_hz=geometry.ka1_hz(pos, c),
        bf_band_hz=(bf.band_min_hz, bf_hi),
        doa_band_hz=(doa.freq_min_hz, doa_hi),
        das_wng_db=das_wng_db,
    )
