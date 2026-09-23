"""Array geometry, steering vectors and noise-field coherence matrices.

Design-side (float64) code: none of this is ported to C directly. Its outputs
(steering vectors, weight tables) are precomputed and exported.

Conventions
-----------
- Positions are (N, 3) in metres, relative to the array centre, in mic-index order.
- Azimuth is measured CCW from +x in the array plane; elevation is up from that plane.
- STFT sign convention is X(f) = sum_t x[t] exp(-j 2 pi f t).
- A steering vector d is the per-mic transfer function relative to the array centre,
  so a plane wave S arriving from direction u gives X_n = S * d_n. For a delay-and-sum
  beamformer w = d / N, y = w^H x = S (unit gain toward the look direction).
"""

from __future__ import annotations

import numpy as np


def uca_positions(n_mics: int, radius_m: float, angle_offset_deg: float = 0.0) -> np.ndarray:
    """Positions of a uniform circular array in the z = 0 plane, mic 0 at angle_offset_deg."""
    phi = np.deg2rad(angle_offset_deg) + 2.0 * np.pi * np.arange(n_mics) / n_mics
    return np.stack([radius_m * np.cos(phi), radius_m * np.sin(phi), np.zeros(n_mics)], axis=1)


def as_positions(positions_m) -> np.ndarray:
    """Coerce an [[x, y], ...] or [[x, y, z], ...] list into an (N, 3) float64 array."""
    pos = np.asarray(positions_m, dtype=np.float64)
    if pos.ndim != 2 or pos.shape[1] not in (2, 3):
        raise ValueError(f"positions must be (N, 2) or (N, 3), got shape {pos.shape}")
    if pos.shape[1] == 2:
        pos = np.hstack([pos, np.zeros((pos.shape[0], 1))])
    return pos


def pairwise_distances(positions: np.ndarray) -> np.ndarray:
    """(N, N) matrix of |p_i - p_j|."""
    diff = positions[:, None, :] - positions[None, :, :]
    return np.linalg.norm(diff, axis=-1)


# ---------------------------------------------------------------------------
# Derived quantities. All limits come from the geometry, never hard-coded.
# ---------------------------------------------------------------------------

def adjacent_spacing(positions: np.ndarray) -> float:
    """Largest nearest-neighbour distance. For a UCA this is 2 r sin(pi / N)."""
    dist = pairwise_distances(positions)
    np.fill_diagonal(dist, np.inf)
    return float(dist.min(axis=1).max())


def aliasing_onset_hz(positions: np.ndarray, c: float) -> float:
    """Spatial aliasing onset c / (2 d), with d from adjacent_spacing()."""
    return c / (2.0 * adjacent_spacing(positions))


def aperture(positions: np.ndarray) -> float:
    """Largest mic-to-mic distance (2 r for a UCA with even N)."""
    return float(pairwise_distances(positions).max())


def max_delay_s(positions: np.ndarray, c: float) -> float:
    """Largest possible inter-mic delay (end-fire across the aperture)."""
    return aperture(positions) / c


def array_radius(positions: np.ndarray) -> float:
    """Largest mic distance from the array centroid."""
    return float(np.linalg.norm(positions - positions.mean(axis=0), axis=1).max())


def ka1_hz(positions: np.ndarray, c: float) -> float:
    """Frequency where k * r = 1, roughly where DAS starts having real directivity."""
    return c / (2.0 * np.pi * array_radius(positions))


# ---------------------------------------------------------------------------
# Steering vectors
# ---------------------------------------------------------------------------

def direction_vector(azimuth_rad, elevation_rad=0.0) -> np.ndarray:
    """Unit vector(s) pointing from the array toward the source, shape (..., 3)."""
    az, el = np.broadcast_arrays(np.asarray(azimuth_rad, float), np.asarray(elevation_rad, float))
    return np.stack([np.cos(el) * np.cos(az), np.cos(el) * np.sin(az), np.sin(el)], axis=-1)


def source_position(azimuth_rad, distance_m, elevation_rad=0.0) -> np.ndarray:
    """Cartesian source position(s) relative to the array centre, shape (..., 3)."""
    return np.asarray(distance_m, float)[..., None] * direction_vector(azimuth_rad, elevation_rad)


def farfield_steering(positions: np.ndarray, freq_hz, azimuth_rad, elevation_rad=0.0,
                      c: float = 343.0) -> np.ndarray:
    """Plane-wave steering vectors d_n = exp(+j k u . p_n).

    freq_hz and the angles broadcast against each other; the result has shape
    broadcast(freq, angle).shape + (N,). For a (F, A) grid pass freq[:, None], az[None, :].
    """
    u = direction_vector(azimuth_rad, elevation_rad)          # (..., 3)
    proj = u @ positions.T                                    # (..., N)
    k = 2.0 * np.pi * np.asarray(freq_hz, float)[..., None] / c
    return np.exp(1j * k * proj)


def nearfield_steering(positions: np.ndarray, freq_hz, source_pos, c: float = 343.0,
                       attenuation: bool = True) -> np.ndarray:
    """Spherical-wave steering vectors for a point source, referenced to the array centre.

    d_n = (R / R_n) exp(-j k (R_n - R)), with R = |s| and R_n = |s - p_n|. Reduces to
    farfield_steering() as R -> inf. Use for close-range bench tests.
    source_pos has shape (..., 3) and broadcasts against freq_hz like farfield_steering().
    """
    s = np.asarray(source_pos, float)
    r_n = np.linalg.norm(s[..., None, :] - positions, axis=-1)   # (..., N)
    r_0 = np.linalg.norm(s, axis=-1)[..., None]                 # (..., 1)
    k = 2.0 * np.pi * np.asarray(freq_hz, float)[..., None] / c
    d = np.exp(-1j * k * (r_n - r_0))
    if attenuation:
        d = d * (r_0 / r_n)
    return d


# ---------------------------------------------------------------------------
# Noise-field models
# ---------------------------------------------------------------------------

def diffuse_coherence(positions: np.ndarray, freq_hz, c: float = 343.0) -> np.ndarray:
    """Spherically isotropic (diffuse) noise coherence Gamma_ij = sin(k d_ij) / (k d_ij).

    Result has shape freq.shape + (N, N). np.sinc is the normalised sinc, hence the / pi.
    """
    dist = pairwise_distances(positions)
    k = 2.0 * np.pi * np.asarray(freq_hz, float)[..., None, None] / c
    return np.sinc(k * dist / np.pi)
