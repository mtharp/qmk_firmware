#!/usr/bin/env python
import json, sys
info = json.load(sys.stdin)
ncols = len(info["matrix_pins"]["cols"])
nrows = len(info["matrix_pins"]["rows"])
layout = sorted(info["layouts"])[0]
keys = set(tuple(x["matrix"]) for x in info["layouts"][layout]["layout"])
dead = [0] * nrows
for row in range(nrows):
    for col in range(ncols):
        if (row, col) not in keys:
            dead[row] |= 1 << col
print(", ".join(f"0x{x:x}" for x in dead))
