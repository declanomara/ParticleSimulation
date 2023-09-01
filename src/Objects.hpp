#pragma once
#include "Eigen/Dense"

class Particle
{

private:
	int MAX_TRAIL_LENGTH = 200;

	int minimumRenderRadiusPx = 5;

	float radius;
	double mass;
	Eigen::Vector2d position;
	Eigen::Vector2d velocity;
	Eigen::Vector2d acceleration;
	Eigen::Vector2d force;
	sf::Color color;

	std::deque<Eigen::Vector2d> trail;

public:
	Particle(float radius, double mass, Eigen::Vector2d position, Eigen::Vector2d velocity);

	void update(float dt);
	void updateTrail();
	void draw(sf::RenderWindow& window);
	void drawTrail(sf::RenderWindow& window);
	void applyForce(Eigen::Vector2d force);

	int getMinimumRenderRadius();
	void setMinimumRenderRadius(int radius);

	float getRenderRadiusWorld(sf::RenderWindow& window);

	bool visiblyContains(Eigen::Vector2d position, sf::RenderWindow& window);

	float getRadius() const;
	double getMass() const;

	Eigen::Vector2d getPosition() const;
	Eigen::Vector2d getVelocity() const;
	Eigen::Vector2d getAcceleration() const;
	Eigen::Vector2d getForce() const;

	double calculatePotentialEnergy(Eigen::Vector2f position);
	void setColor(sf::Color color);
};

class ParticleSystem
{
private:
	std::vector<Particle> particles;
	std::vector<Particle> destroyedParticles;

public:
	ParticleSystem();
	void update(float dt);
	void calculateForcesBarnesHut();
	void draw(sf::RenderWindow& window);

	void addParticle(Particle particle);

	double calculatePotentialEnergy(Eigen::Vector2f position);

	bool isNearParticle(Eigen::Vector2f position);

	int getParticleCount();
	int getDestroyedParticleCount();

	std::vector<Particle>& getParticles();
	Particle* particleVisibleAt(Eigen::Vector2f position, sf::RenderWindow& window);
};

class GUI
{
private:
	const int fontSize = 18;
	sf::Font font;
	sf::Text text;

	int objectsInView;
	int objectsInSun;
	int objectsInSystem;

	int elapsedYears;

	// Grey background
	sf::Color backgroundColor = sf::Color(50, 50, 50, 100);

	// Black stroke
	sf::Color strokeColor = sf::Color(0, 0, 0, 255);

	// White text
	sf::Color textColor = sf::Color(255, 255, 255, 255);

public:
	GUI();
	void draw(sf::RenderWindow& window);

	void setElapsedYears(int elapsedYears);
};
