# N-Body Simulation Engine (C++ / OpenGL)

A real-time 2D N-body gravitational simulation built from scratch in C++, featuring numerical integration, collision handling, and live diagnostics with OpenGL visualization.

---

## Features

- Newtonian gravity simulation with pairwise interactions
- Softening factor to prevent singularities at small distances

- Modular simulation engine:
  - `Simulation` class manages particles and physics
  - `Integrator` abstraction supports multiple numerical methods
  - Implemented:
    - Semi-Implicit Euler
    - Explicit Euler

- Collision handling:
  - Elastic collisions using impulse-based response
  - Overlap correction to prevent tunneling

- Real-time diagnostics:
  - Kinetic Energy
  - Potential Energy
  - Total Energy
  - Total Momentum

- OpenGL visualization:
  - Circular particle rendering using shaders
  - Motion trails
  - Color palette for multiple bodies
  - Dynamic scaling with window size

---

## What This Demonstrates

This project focuses on simulation correctness and numerical behavior:

- Conservation of momentum (validated)
- Analysis of energy drift due to numerical integration
- Understanding of time-stepping methods
- Separation of physics, rendering, and integration logic

---

## Preview

(Add a screenshot or GIF here)

---

## Tech Stack

- C++
- OpenGL (GLFW + GLAD)
- Custom vector math (Vector2)

---

## How to Run

```bash
git clone https://github.com/sohamr01/nbody-sim.git
cd nbody-sim
mkdir build && cd build
cmake ..
cmake --build .
./nbody