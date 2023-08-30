#pragma once
#include "Eigen/Dense"

namespace constants
{
const double G = 6.67408e-11;

const double solarMass = 1.98847e30;
const double solarRadius = 6.957e8;

const double earthMass = 5.9722e24;
const double earthRadius = 6.371e6;
const double earthOrbitRadius = 1.496e11;

const double moonMass = 7.342e22;
const double moonRadius = 1.737e6;
const double moonOrbitRadius = 3.844e8;

const double jupiterMass = 1.8982e27;
const double jupiterRadius = 6.9911e7;
const double jupiterOrbitRadius = 7.785472e11;
}

namespace util
{
Eigen::Vector2f toEigen(sf::Vector2f vector);
sf::Vector2f toSFML(Eigen::Vector2f vector);

sf::Color blueToRed(double value);

// A template function to convert eigen vectors to sfml vectors
template <typename T>
sf::Vector2<T> toSFML(Eigen::Vector2<T> vector);

// A template function to convert sfml vectors to eigen vectors
template <typename T>
Eigen::Vector2<T> toEigen(sf::Vector2<T> vector);
}