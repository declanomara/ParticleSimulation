# Particle Simulation

This repo is a collisionless gravitational N-body particle simulation built using Eigen for vector algebra, and SFML for rendering. The Chalet build system is used to build the project.
Although this is a general purpose simulator, it was built in order to simulate the formation of asteroid clusters around Jupiter,
known as Trojans, as a my final project for *EPSS 155: Planetary Physics* at UCLA.

The simulation uses the Barnes-Hut algorithm with a quadtree data structure to efficiently compute gravitational forces between all particles, reducing complexity from O(n²) to O(n log n). This allows for realistic asteroid-asteroid interactions without sacrificing performance.

# Installation
## Dependencies
* Eigen
* SFML
* Chalet

Eigen is a small library which I included in this repo. SFML and Chalet must be installed separately.

## Building
To build the project, simply run `chalet build` in the root directory of the project.

## Running
To run the project, run `chalet run` in the root directory of the project.

# Usage
## Controls
- `Space` - Pause/unpause the simulation
- `R` - Reset the simulation
- `T` - Toggle particle trails (off by default)
- `G` - Toggle gravitational field visualization
- `+` / `-` - Zoom in/out (centered on mouse pointer)
- Right-click + drag - Pan the camera
- Left-click - Select and follow a particle



# Abstract
The Jupiter Trojans are a collection of asteroids that share an orbit with Jupiter. These asteroids
orbit one of Jupiter’s stable Lagrange points, either the L4 or L5 point. It is believed that these
asteroid clusters formed during the formation of Jupiter. In the late stages of Jupiter’s formation,
its mass increased ten fold in about 10,000 years. This rapid increase in gravitational potential
drew the Jupiter Trojans in towards Jupiter, during which they were captured at the L4 and L5
points.

The purpose of this project is to simulate the behavior of the Jupiter Trojans and verify that they
could be a result of purely gravitational effects. Furthermore, this project hopes to determine
various properties of the Jupiter Trojans such as the survival rate of asteroids scattered randomly
along Jupiter’s orbit. If it can be determined what fraction of these asteroids are eventually
captured, it is possible to estimate the initial amount of mass required to form Jupiter’s Trojans.
Further distinctions can be made between asteroids ejected into outer space and asteroids that fall
into the Sun.

The simulation is written in C++ and uses Newton's law of universal gravitation to
calculate the force between all objects. The Barnes-Hut algorithm approximates distant particle
clusters as single bodies, dramatically improving performance while maintaining accuracy.
Collisions between objects are not modeled as they have minimal effect on the aggregate behavior
of the Jupiter Trojans.
The results are expected to show that Jupiter’s Trojans do form under heavily simplified
gravitational effects, supporting existing scientific beliefs. Furthermore, it is expected that
roughly two thirds of asteroids scattered around Jupiter’s orbit will be captured near Jupiter’s
Lagrange points, while the last third will be ejected from the solar system or captured by the Sun.

These results would imply that the initial mass of asteroids required to form Jupiter’s Trojans is
roughly 50% greater than the present day mass of Jupiter’s Trojans.