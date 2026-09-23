# Python prototype

Verifies the beamforming and DOA math in simulation and on bench captures, before the C port.

```
configs/        array configs (proto_4mic.yaml, board_8mic.yaml); every algorithm reads these
arraydsp/       the Python package
  config.py       YAML -> dataclasses + derived quantities (spacing, aliasing, max delay)
  geometry.py     mic positions, steering vectors, coherence matrices
tests/          pytest: checks the math against known answers
data/           captures and sim outputs (gitignored)
```

Code that gets ported to C will go in `arraydsp/core/`: block-based, float32, explicit state.
Everything else (weight design, simulation, plots) stays in plain numpy/scipy.

## Setup

Run from this folder:

```
pip install -e ".[test]"
pytest
```
