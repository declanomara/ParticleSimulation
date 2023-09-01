#include "Objects.hpp"
#include "QuadTree.hpp"
#include "utils.hpp"

Particle::Particle(float radius, double mass, Eigen::Vector2d position, Eigen::Vector2d velocity) :
	radius(radius),
	mass(mass),
	position(position),
	velocity(velocity),
	acceleration(Eigen::Vector2d::Zero()),
	force(Eigen::Vector2d::Zero()),
	color(sf::Color::White)
{
}

float Particle::getRenderRadiusWorld(sf::RenderWindow& window)
{
	float renderRadius = radius;
	float unitsPerPixel = window.mapPixelToCoords(sf::Vector2i(1, 0)).x - window.mapPixelToCoords(sf::Vector2i(0, 0)).x;
	float minimumRenderRadius = minimumRenderRadiusPx * unitsPerPixel;
	if (minimumRenderRadius > radius)
	{
		renderRadius = minimumRenderRadius;
	}

	return renderRadius;
}

bool Particle::visiblyContains(Eigen::Vector2d position, sf::RenderWindow& window)
{
	return (position - this->position).norm() < getRenderRadiusWorld(window);
}

void Particle::draw(sf::RenderWindow& window)
{
	float renderRadius = getRenderRadiusWorld(window);
	sf::CircleShape circle(renderRadius);
	circle.setOrigin(renderRadius, renderRadius);
	circle.setPosition(position.x(), position.y());
	circle.setFillColor(color);
	window.draw(circle);
}

void Particle::drawTrail(sf::RenderWindow& window)
{
	// 50% transparent green
	sf::Color trailColor = sf::Color(0, 255, 0, 128);
	sf::VertexArray lines(sf::LineStrip, trail.size());
	int i = 0;
	for (Eigen::Vector2d point : trail)
	{
		lines[i].position = util::toSFML(point.cast<float>());
		lines[i].color = trailColor;
		i++;
	}
	window.draw(lines);
}

double Particle::calculatePotentialEnergy(Eigen::Vector2f position)
{
	double distance = (position.cast<double>() - this->position).norm();
	return -constants::G * mass / distance;
}

void Particle::applyForce(Eigen::Vector2d force)
{
	this->force += force;
}

void Particle::update(float dt)
{
	// Integrates the particle's position, velocity, and acceleration using Euler's method
	acceleration = force / mass;
	velocity += acceleration * dt;
	Eigen::Vector2d dr_euler = velocity * dt;
	position += dr_euler;

	// Reset the force
	force = Eigen::Vector2d::Zero();
}

void Particle::updateTrail()
{
	trail.push_back(position);
	if (trail.size() > MAX_TRAIL_LENGTH)
	{
		trail.pop_front();
	}
}

int Particle::getMinimumRenderRadius()
{
	return minimumRenderRadiusPx;
}

void Particle::setMinimumRenderRadius(int minimumRenderRadiusPx)
{
	this->minimumRenderRadiusPx = minimumRenderRadiusPx;
}

float Particle::getRadius() const
{
	return radius;
}

double Particle::getMass() const
{
	return mass;
}

Eigen::Vector2d Particle::getPosition() const
{
	return position;
}

Eigen::Vector2d Particle::getVelocity() const
{
	return velocity;
}

Eigen::Vector2d Particle::getAcceleration() const
{
	return acceleration;
}

Eigen::Vector2d Particle::getForce() const
{
	return force;
}

void Particle::setColor(sf::Color color)
{
	this->color = color;
}

// Particle system
ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::addParticle(Particle particle)
{
	particles.push_back(particle);
}

void ParticleSystem::draw(sf::RenderWindow& window)
{
	for (Particle& particle : particles)
	{
		particle.draw(window);
	}
}

void ParticleSystem::update(float dt)
{
	for (Particle& particle : particles)
	{
		particle.update(dt);
		particle.updateTrail();
	}
}

void ParticleSystem::calculateForcesBarnesHut()
{
	if (particles.empty()) return;

	// Calculate bounds for all particles with some padding
	Eigen::Vector2d minBounds = particles[0].getPosition();
	Eigen::Vector2d maxBounds = particles[0].getPosition();

	for (const Particle& particle : particles)
	{
		const Eigen::Vector2d& pos = particle.getPosition();
		minBounds.x() = std::min(minBounds.x(), pos.x());
		minBounds.y() = std::min(minBounds.y(), pos.y());
		maxBounds.x() = std::max(maxBounds.x(), pos.x());
		maxBounds.y() = std::max(maxBounds.y(), pos.y());
	}

	// Add padding (10% on each side)
	Eigen::Vector2d size = maxBounds - minBounds;
	double padding = std::max(size.x(), size.y()) * 0.1;
	if (padding < 1e9) padding = 1e9; // Minimum padding

	minBounds.x() -= padding;
	minBounds.y() -= padding;
	maxBounds.x() += padding;
	maxBounds.y() += padding;

	// Build the quadtree
	QuadTree tree(minBounds, maxBounds);
	tree.build(particles);

	// Calculate forces for each particle using the tree
	for (Particle& particle : particles)
	{
		Eigen::Vector2d force = tree.calculateForce(particle);
		particle.applyForce(force);
	}
}

double ParticleSystem::calculatePotentialEnergy(Eigen::Vector2f position)
{
	double potentialEnergy = 0.0;

	for (Particle& particle : particles)
	{
		potentialEnergy += particle.calculatePotentialEnergy(position);
	}

	return potentialEnergy;
}

// "close" means within 10 radii of a particle
bool ParticleSystem::isNearParticle(Eigen::Vector2f position)
{
	for (Particle& particle : particles)
	{
		double characteristicDistance = particle.getMass() / 4e18;
		if ((particle.getPosition() - position.cast<double>()).norm() < characteristicDistance)
		{
			return true;
		}
	}

	return false;
}

std::vector<Particle>& ParticleSystem::getParticles()
{
	return particles;
}

Particle* ParticleSystem::particleVisibleAt(Eigen::Vector2f position, sf::RenderWindow& window)
{
	// Iterate over all particles and return the first one that is visible at the given position
	for (Particle& particle : particles)
	{
		if (particle.visiblyContains(position.cast<double>(), window))
		{
			return &particle;
		}
	}

	return nullptr;
}

int ParticleSystem::getParticleCount()
{
	return particles.size();
}

int ParticleSystem::getDestroyedParticleCount()
{
	return destroyedParticles.size();
}

GUI::GUI()
{
	font.loadFromFile("arial.ttf");
}

void GUI::draw(sf::RenderWindow& window)
{
	// Draw the GUI background
	sf::RectangleShape background(sf::Vector2f(window.getSize().x, 50));
	background.setFillColor(backgroundColor);
	background.setOutlineColor(strokeColor);
	window.draw(background);

	// Draw elapsed time
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(fontSize);
	text.setString("Elapsed time: " + std::to_string(elapsedYears) + " years");
	text.setFillColor(textColor);
	text.setPosition(10, 10);
	window.draw(text);
}

void GUI::setElapsedYears(int elapsedYears)
{
	this->elapsedYears = elapsedYears;
}