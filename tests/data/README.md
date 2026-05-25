# Test fixtures

## `golden_10k_tick.sha256`

Expected SHA-256 hex digest of entity states after **10,000** headless ticks of `circular_leo` at **60 Hz**.

Regenerate (requires Python 3):

```bash
py -3 scripts/gen_golden_hash.py
```

After changing physics, integration order, or hash serialization, regenerate and commit this file.
