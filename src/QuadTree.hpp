#pragma once
#include "Eigen/Dense"
#include <vector>
#include <memory>

class Particle; // Forward declaration

class QuadTreeNode {
public:
    // Barnes-Hut threshold parameter
    // If s/d < theta, treat node as single body
    // Typical values: 0.5 (accurate) to 1.0 (fast)
    static constexpr double THETA = 0.5;

    // Maximum particles per leaf node before subdivision
    static constexpr int MAX_PARTICLES_PER_NODE = 1;

private:
    // Spatial bounds [min, max]
    Eigen::Vector2d boundsMin;
    Eigen::Vector2d boundsMax;

    // Center of mass and total mass for this node
    Eigen::Vector2d centerOfMass;
    double totalMass;

    // Particles in this node (only used if isLeaf)
    std::vector<Particle*> particles;

    // Four children: [NW, NE, SW, SE]
    std::unique_ptr<QuadTreeNode> children[4];

    bool isLeaf;

    // Helper to get child index for a position
    int getChildIndex(const Eigen::Vector2d& pos) const;

    // Subdivide this node into 4 children
    void subdivide();

public:
    QuadTreeNode(const Eigen::Vector2d& min, const Eigen::Vector2d& max);

    // Insert a particle into the tree
    void insert(Particle* particle);

    // Compute center of mass for this node and all children
    void computeMassDistribution();

    // Calculate force on a particle using Barnes-Hut approximation
    Eigen::Vector2d calculateForce(const Particle& particle) const;

    // Get bounds
    Eigen::Vector2d getBoundsMin() const { return boundsMin; }
    Eigen::Vector2d getBoundsMax() const { return boundsMax; }

    // Get size (width of node)
    double getSize() const;
};

class QuadTree {
private:
    std::unique_ptr<QuadTreeNode> root;
    Eigen::Vector2d boundsMin;
    Eigen::Vector2d boundsMax;

public:
    QuadTree(const Eigen::Vector2d& min, const Eigen::Vector2d& max);

    // Build tree from particles
    void build(std::vector<Particle>& particles);

    // Calculate force on a particle using the tree
    Eigen::Vector2d calculateForce(const Particle& particle) const;

    // Get root node (for debugging/visualization)
    const QuadTreeNode* getRoot() const { return root.get(); }
};

