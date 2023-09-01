#include "QuadTree.hpp"
#include "Objects.hpp"
#include "utils.hpp"
#include <iostream>
#include <cmath>

// QuadTreeNode Implementation

QuadTreeNode::QuadTreeNode(const Eigen::Vector2d& min, const Eigen::Vector2d& max)
    : boundsMin(min), boundsMax(max), centerOfMass(Eigen::Vector2d::Zero()),
      totalMass(0.0), isLeaf(true) {
    for (int i = 0; i < 4; i++) {
        children[i] = nullptr;
    }
}

double QuadTreeNode::getSize() const {
    // Return the maximum dimension (width or height)
    Eigen::Vector2d size = boundsMax - boundsMin;
    return std::max(size.x(), size.y());
}

int QuadTreeNode::getChildIndex(const Eigen::Vector2d& pos) const {
    Eigen::Vector2d center = (boundsMin + boundsMax) / 2.0;

    // Determine quadrant:
    // 0: NW (top-left), 1: NE (top-right), 2: SW (bottom-left), 3: SE (bottom-right)
    // Note: In screen coordinates, y increases downward
    bool west = pos.x() < center.x();
    bool north = pos.y() < center.y();

    if (north && west) return 0; // NW
    if (north && !west) return 1; // NE
    if (!north && west) return 2; // SW
    return 3; // SE
}

void QuadTreeNode::subdivide() {
    if (!isLeaf) return;

    Eigen::Vector2d center = (boundsMin + boundsMax) / 2.0;

    // Create four children
    // NW (0)
    children[0] = std::make_unique<QuadTreeNode>(
        Eigen::Vector2d(boundsMin.x(), boundsMin.y()),
        Eigen::Vector2d(center.x(), center.y())
    );

    // NE (1)
    children[1] = std::make_unique<QuadTreeNode>(
        Eigen::Vector2d(center.x(), boundsMin.y()),
        Eigen::Vector2d(boundsMax.x(), center.y())
    );

    // SW (2)
    children[2] = std::make_unique<QuadTreeNode>(
        Eigen::Vector2d(boundsMin.x(), center.y()),
        Eigen::Vector2d(center.x(), boundsMax.y())
    );

    // SE (3)
    children[3] = std::make_unique<QuadTreeNode>(
        Eigen::Vector2d(center.x(), center.y()),
        Eigen::Vector2d(boundsMax.x(), boundsMax.y())
    );

    // Redistribute particles to children
    for (Particle* particle : particles) {
        int childIdx = getChildIndex(particle->getPosition());
        children[childIdx]->insert(particle);
    }

    particles.clear();
    isLeaf = false;
}

void QuadTreeNode::insert(Particle* particle) {
    // Check if particle is within bounds (with some tolerance)
    const Eigen::Vector2d& pos = particle->getPosition();
    const double EPSILON = 1e-10;

    if (pos.x() < boundsMin.x() - EPSILON || pos.x() > boundsMax.x() + EPSILON ||
        pos.y() < boundsMin.y() - EPSILON || pos.y() > boundsMax.y() + EPSILON) {
        // Particle is out of bounds - this shouldn't happen if bounds are set correctly
        return;
    }

    if (isLeaf) {
        // Add particle to this leaf
        particles.push_back(particle);

        // If we exceed capacity, subdivide
        if (particles.size() > MAX_PARTICLES_PER_NODE) {
            subdivide();
        }
    } else {
        // Insert into appropriate child
        int childIdx = getChildIndex(pos);
        children[childIdx]->insert(particle);
    }
}

void QuadTreeNode::computeMassDistribution() {
    if (isLeaf) {
        // Compute center of mass for particles in this leaf
        totalMass = 0.0;
        centerOfMass = Eigen::Vector2d::Zero();

        for (Particle* particle : particles) {
            double mass = particle->getMass();
            totalMass += mass;
            centerOfMass += particle->getPosition() * mass;
        }

        if (totalMass > 0.0) {
            centerOfMass /= totalMass;
        }
    } else {
        // Recursively compute for children
        totalMass = 0.0;
        centerOfMass = Eigen::Vector2d::Zero();

        for (int i = 0; i < 4; i++) {
            if (children[i]) {
                children[i]->computeMassDistribution();
                double childMass = children[i]->totalMass;
                if (childMass > 0.0) {
                    totalMass += childMass;
                    centerOfMass += children[i]->centerOfMass * childMass;
                }
            }
        }

        if (totalMass > 0.0) {
            centerOfMass /= totalMass;
        }
    }
}

Eigen::Vector2d QuadTreeNode::calculateForce(const Particle& particle) const {
    // If this node has no mass, no force
    if (totalMass == 0.0) {
        return Eigen::Vector2d::Zero();
    }

    Eigen::Vector2d particlePos = particle.getPosition();
    Eigen::Vector2d direction = centerOfMass - particlePos;
    double distance = direction.norm();

    // Avoid self-interaction and division by zero
    const double MIN_DISTANCE = 1e3; // Minimum distance to avoid singularity
    if (distance < MIN_DISTANCE) {
        return Eigen::Vector2d::Zero();
    }

    // Barnes-Hut criterion: s/d < theta
    double s = getSize();
    double ratio = s / distance;

    if (isLeaf || ratio < THETA) {
        // Either this is a leaf with single particle, or we're far enough away
        // Treat as single body at center of mass
        double forceMagnitude = constants::G * totalMass * particle.getMass() / (distance * distance);
        return forceMagnitude * direction.normalized();
    } else {
        // We're too close - need to check children
        Eigen::Vector2d totalForce = Eigen::Vector2d::Zero();
        for (int i = 0; i < 4; i++) {
            if (children[i]) {
                totalForce += children[i]->calculateForce(particle);
            }
        }
        return totalForce;
    }
}

// QuadTree Implementation

QuadTree::QuadTree(const Eigen::Vector2d& min, const Eigen::Vector2d& max)
    : boundsMin(min), boundsMax(max) {
    root = std::make_unique<QuadTreeNode>(min, max);
}

void QuadTree::build(std::vector<Particle>& particles) {
    // Clear and rebuild the tree
    root = std::make_unique<QuadTreeNode>(boundsMin, boundsMax);

    // Insert all particles
    for (Particle& particle : particles) {
        root->insert(&particle);
    }

    // Compute mass distribution
    root->computeMassDistribution();
}

Eigen::Vector2d QuadTree::calculateForce(const Particle& particle) const {
    if (!root) {
        return Eigen::Vector2d::Zero();
    }
    return root->calculateForce(particle);
}

