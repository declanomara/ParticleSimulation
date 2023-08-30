#include "utils.hpp"

Eigen::Vector2f util::toEigen(sf::Vector2f vector)
{
	return Eigen::Vector2f(vector.x, vector.y);
}

sf::Vector2f util::toSFML(Eigen::Vector2f vector)
{
	return sf::Vector2f(vector.x(), vector.y());
}

sf::Color util::blueToRed(double value)
{
	double r = std::min(1.0, 2.0 * value);
	double g = std::min(1.0, 2.0 * (1.0 - value));
	double b = 0.0;

	return sf::Color(r * 255, g * 255, b * 255);
}

// Implementation of the template functions

template <typename T>
sf::Vector2<T> util::toSFML(Eigen::Vector2<T> vector)
{
	return sf::Vector2<T>(vector.x(), vector.y());
}

template <typename T>
Eigen::Vector2<T> util::toEigen(sf::Vector2<T> vector)
{
	return Eigen::Vector2<T>(vector.x, vector.y);
}