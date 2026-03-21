# Interaction Rule Concept

This simulation uses a simple pairwise interaction rule between particles. Each particle belongs to a class, and interactions are driven by a class-to-class attraction matrix plus a global strength. For every particle, we accumulate a force from nearby neighbors and use that to update velocity and position.

## Core Idea

For each particle `i`:

1. Find neighbors within a fixed interaction radius.
2. For each neighbor `j`, compute a direction from `i` to `j`.
3. Look up the attraction value for the (class of `i`, class of `j`) pair.
4. Scale that attraction by distance falloff and global strength.
5. Sum these contributions to get the acceleration applied to particle `i`.

This produces emergent behavior ranging from clustering to dispersion depending on the sign and magnitude of the attraction matrix.

## Components

- Interaction radius: Limits which neighbors contribute to the force.
- Attraction matrix: A `class_count x class_count` grid of floats.
  - Positive values attract.
  - Negative values repel.
  - Zero means no interaction.
- Strength: A global scalar multiplier applied to the attraction matrix.
- Distance falloff: Linear falloff with distance inside the radius.

## Rule (Summary)

For a particle `i` with class `ci` and a neighbor `j` with class `cj`:

- Direction: `d = (pos[j] - pos[i])`
- Distance: `dist = |d|`
- Influence: `1 - dist / radius`
- Force scalar: `attraction[ci][cj] * strength`
- Acceleration contribution: `normalize(d) * force * influence`

Acceleration is then integrated into velocity and position during the simulation step.

## Notes

- Only neighbors within the interaction radius contribute.
- If attraction is disabled, no interaction force is applied.
- The interaction is asymmetric if `attraction[ci][cj] != attraction[cj][ci]`.

## Where It Lives

The interaction rule is implemented in the simulation update loop in `src/simulation/simulation.cpp`.
