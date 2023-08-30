#include "Objects.hpp"
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

float Particle::getRadius()
{
	return radius;
}

double Particle::getMass()
{
	return mass;
}

Eigen::Vector2d Particle::getPosition()
{
	return position;
}

Eigen::Vector2d Particle::getVelocity()
{
	return velocity;
}

Eigen::Vector2d Particle::getAcceleration()
{
	return acceleration;
}

Eigen::Vector2d Particle::getForce()
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

void ParticleSystem::addTestParticle(Particle particle)
{
	testParticles.push_back(particle);
}

void ParticleSystem::draw(sf::RenderWindow& window)
{
	for (Particle& particle : particles)
	{
		particle.draw(window);
	}

	for (Particle& particle : testParticles)
	{
		particle.draw(window);
	}
}

void ParticleSystem::update(float dt)
{
	for (Particle& particle : particles)
	{
		particle.update(dt);
	}
}

void ParticleSystem::lazyUpdate(float dt)
{
	for (Particle& particle : testParticles)
	{
		particle.update(dt);
	}

	for (Particle& particle : particles)
	{
		particle.updateTrail();
	}
}

void ParticleSystem::calculateForces()
{
	// Iterate over all pairs of particles, calculate the force between them, and apply it to each particle
	for (size_t i = 0; i < particles.size(); i++)
	{
		for (size_t j = i + 1; j < particles.size(); j++)
		{
			Particle& particle1 = particles[i];
			Particle& particle2 = particles[j];

			Eigen::Vector2d direction = particle2.getPosition() - particle1.getPosition();
			float distance = direction.norm();
			double forceMagnitude = constants::G * particle1.getMass() * particle2.getMass() / (distance * distance);
			Eigen::Vector2d force = forceMagnitude * direction.normalized();

			particle1.applyForce(force);
			particle2.applyForce(-force);
		}

		// std::cout << "Force on particle " << i << ": " << particles[i].getForce().transpose() << std::endl;
	}
}

void ParticleSystem::calculateTestForces()
{
	// Iterate over all test particles and calculate the force on each test particle due to particles in the system
	for (Particle& testParticle : testParticles)
	{
		Eigen::Vector2d force = Eigen::Vector2d::Zero();

		for (Particle& particle : particles)
		{
			Eigen::Vector2d direction = particle.getPosition() - testParticle.getPosition();
			float distance = direction.norm();
			double forceMagnitude = constants::G * particle.getMass() * testParticle.getMass() / (distance * distance);
			force += forceMagnitude * direction.normalized();
		}

		testParticle.applyForce(force);
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

	for (Particle& particle : testParticles)
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
	return particles.size() + testParticles.size();
}

void ParticleSystem::destroyTestParticlesNear(Particle particle)
{
	// Iterate over all test particles and if they are within 10 radii of the given particle, add them to the destroyedParticles vector
	for (Particle& testParticle : testParticles)
	{
		if ((testParticle.getPosition() - particle.getPosition()).norm() < (double)10 * particle.getRadius())
		{
			destroyedParticles.push_back(testParticle);
		}
	}
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