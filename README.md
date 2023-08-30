# Particle Simulation

This repo is a basic collisionless gravitational particle simulation built using Eigen for vector algebra, and SFML for rendering. The Chalet build system is used to build the project.
Although this is a general purpose simulator, it was built in order to simulate the formation of asteroid clusters around Jupiter,
known as Trojans, as a my final project for *EPSS 155: Planetary Physics* at UCLA.

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
Press `space` to pause the simulation. Press `r` to reset the simulation.
To pan the camera, right click and drag. To zoom in and out, use `+` and `-` respectively (zoom is centered on the mouse pointer).
Press `g` to toggle the gravitational field visualization.



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

The simulation is written in C++ and simply uses Newton’s law of universal gravitation to
calculate the force between pairs of objects. To improve run time, gravitational effects between
asteroids will not be considered. Furthermore, collisions between objects will not be taken into
account either as they have minimal effect on the aggregate behavior of the Jupiter Trojans.
The results are expected to show that Jupiter’s Trojans do form under heavily simplified
gravitational effects, supporting existing scientific beliefs. Furthermore, it is expected that
roughly two thirds of asteroids scattered around Jupiter’s orbit will be captured near Jupiter’s
Lagrange points, while the last third will be ejected from the solar system or captured by the Sun.

These results would imply that the initial mass of asteroids required to form Jupiter’s Trojans is
roughly 50% greater than the present day mass of Jupiter’s Trojans.