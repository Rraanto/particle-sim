# Per-Frame Velocity Update Rules

This document explains where the per-frame velocity update happens and how to
set or customize the rule that updates particle velocities every frame.

## Where The Velocity Update Happens

The update rule is implemented in `Simulation::step(float dt)`:

- File: `src/simulation/simulation.cpp`
- Function: `Simulation::step(float dt)`

At a high level, each call to `step(dt)` does the following, for every particle:

1. Compute acceleration (`ax`, `ay`) from nearby particles using `ForceParams`.
2. Integrate acceleration into velocity: `v += a * dt`.
3. Apply damping: `v *= damping`.
4. Clamp speed to `max_speed`.
5. Integrate velocity into position: `x += v * dt`.
6. Apply boundary rules (wrap or bounce).

The specific code that updates velocity is the block inside the per-particle
loop in `Simulation::step(float dt)`:

- `ax`, `ay` are accumulated from the force rule.
- `v += a * dt` applies the per-frame acceleration.
- `v *= damping` applies the global damping rule.
- `limit_speed(i)` caps the speed.

## How "Per-Frame" Maps To The Main Loop

The simulation runs at a fixed timestep in `src/main.cpp`:

- The render loop accumulates real frame time (`frame_dt`).
- It calls `simulation.step(fixed_dt)` as many times as needed to catch up.
- Default `fixed_dt` is `1.0 / 60.0` seconds.

That means the **velocity rule is evaluated once per simulation step**, not
once per render frame. If a frame is slow, multiple steps may run and the rule
will be applied multiple times in that frame.

## Setting The Rule Without Code Changes

You can adjust the *existing* velocity rule via `SimulationParams` and
`ForceParams`:

- `SimulationParams::damping` controls velocity decay each step.
- `SimulationParams::max_speed` caps the speed.
- `ForceParams::attraction` and `ForceParams::strength` control acceleration.
- `SimulationParams::interaction_radius` and `grid_cell_size` control neighbor
  lookup, which changes the forces applied.

Typical workflow:

1. Create a `SimulationParams` instance and set `damping`, `max_speed`,
   `interaction_radius`, and bounds.
2. Create a `ForceParams` instance with `class_count` and `attraction` values.
3. Construct the `Simulation` with these params.
4. Call `step(dt)` each simulation tick.

## Custom Rule: Modify `Simulation::step`

If you need a different per-frame velocity update rule (e.g., gravity, noise,
user-defined forces), edit `Simulation::step(float dt)` and replace or extend
how `ax` and `ay` are computed and applied.

A minimal custom rule pattern looks like this:

```cpp
// Inside Simulation::step(float dt), per-particle loop
float ax = 0.0f;
float ay = 0.0f;

// Example: constant gravity
ay += -9.81f;

// Example: wind pushing to the right
ax += 2.0f;

// Integrate
_vel_x[i] += ax * dt;
_vel_y[i] += ay * dt;
_vel_x[i] *= _params.damping;
_vel_y[i] *= _params.damping;
limit_speed(i);
```

If you keep the neighbor-force rule, you can add to `ax`/`ay` after the neighbor
loop, or replace it entirely.

## Custom Rule: Add A Reusable Hook

If you want a reusable hook instead of editing the loop every time, create a
helper that computes acceleration and call it from `step()`.

Suggested pattern:

1. Add a private method in `Simulation`:

```cpp
void compute_acceleration(size_t i, float &ax, float &ay);
```

2. Implement it in `simulation.cpp` (using any rule you want).
3. Call it inside the per-particle loop:

```cpp
float ax = 0.0f;
float ay = 0.0f;
compute_acceleration(i, ax, ay);
```

This keeps the per-frame velocity rule isolated and easier to iterate on.

## Notes

- `dt` must be positive; `step(dt)` returns early if `dt <= 0`.
- Damping is applied *after* acceleration, so values close to `1.0f` preserve
  velocity and smaller values add stronger drag.
- `max_speed <= 0` disables speed limiting.
- Boundary handling can flip velocity when `wrap_bounds` is `false`.

## Quick Checklist

- Decide if you can express your rule using `ForceParams` and
  `SimulationParams`.
- If not, modify or hook into `Simulation::step(float dt)`.
- Keep the update in terms of `dt` so the rule is stable under different
  frame rates.

