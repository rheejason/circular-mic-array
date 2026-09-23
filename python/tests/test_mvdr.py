"""Superdirective / MVDR weight tests."""

import numpy as np
import pytest

from arraydsp import geometry


@pytest.mark.xfail(raises=ImportError, strict=True, reason="arraydsp.weights not written yet")
@pytest.mark.parametrize("n", [4, 8])
def test_mvdr_large_loading_equals_das(n):
    """With eps -> inf, (Gamma + eps I)^-1 -> I / eps, so MVDR weights must collapse to d / N.

    Catches conjugate, orientation and normalisation errors in the weight design.
    """
    from arraydsp.weights import mvdr_weights

    pos = geometry.uca_positions(n, 0.055)
    for f in (300.0, 1000.0, 3000.0):
        d = geometry.farfield_steering(pos, f, np.deg2rad(37.0))
        gamma = geometry.diffuse_coherence(pos, f)
        w = mvdr_weights(gamma, d, eps=1e12)
        assert np.allclose(w, d / n, atol=1e-9)
