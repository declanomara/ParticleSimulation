#include "Objects.hpp"
#include "Platform/Platform.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

const float WINDOW_WIDTH = 800.0f;
const float WINDOW_HEIGHT = 600.0f;

ParticleSystem initializeSimulation()
{
	Particle sun = Particle(constants::solarRadius, constants::solarMass, Eigen::Vector2d(0.0f, 0.0f), Eigen::Vector2d(0.0f, 0.0f));
	sun.setColor(sf::Color::Yellow);
	sun.setMinimumRenderRadius(10);

	Particle jupiter = Particle(constants::jupiterRadius, constants::jupiterMass, Eigen::Vector2d(constants::jupiterOrbitRadius, 0.0f), Eigen::Vector2d(0.0f, 13070.0f));
	jupiter.setColor(sf::Color::Red);

	ParticleSystem particleSystem;
	particleSystem.addParticle(sun);
	particleSystem.addParticle(jupiter);

	// Scatter particles along jupiter's orbit
	int numParticles = 100;
	for (int i = 0; i < numParticles; i++)
	{
		Eigen::Vector2d position = Eigen::Vector2d(constants::jupiterOrbitRadius + 1e9, 0.0f);
		Eigen::Vector2d velocity = Eigen::Vector2d(0.0f, 13070.0f);
		position = Eigen::Rotation2Dd(i * 2 * M_PI / numParticles + .1) * position;
		velocity = Eigen::Rotation2Dd(i * 2 * M_PI / numParticles + .1) * velocity;
		Particle particle = Particle(1.0f, (double)1.0f, position, velocity);
		particle.setMinimumRenderRadius(2);
		particle.setColor(sf::Color::White);
		particleSystem.addTestParticle(particle);
	}

	return particleSystem;
}

int main()
{
	util::Platform platform;

#if defined(_DEBUG)
	std::cout << "Hello World!" << std::endl;
#endif

	sf::RenderWindow window;
	// in Windows at least, this must be called before creating the window
	float screenScale = platform.getScreenScalingFactor(window.getSystemHandle());

	sf::VideoMode mode {
		static_cast<sf::Uint32>(WINDOW_WIDTH * screenScale),
		static_cast<sf::Uint32>(WINDOW_HEIGHT * screenScale),
	};

	sf::ContextSettings settings;
#if !defined(SFML_SYSTEM_MACOS)
// settings.majorVersion = 4;
// settings.minorVersion = 1;
// settings.depthBits = 24;
// settings.stencilBits = 8;
// settings.attributeFlags = sf::ContextSettings::Attribute::Core;
#endif
	window.create(mode, "SFML works!", sf::Style::Default, settings);
	platform.initialize(window.getSystemHandle());

	[](const sf::ContextSettings& inSettings) {
		std::cout << "OpenGL context created with version: "
				  << inSettings.majorVersion << "." << inSettings.minorVersion
				  << " with " << inSettings.depthBits << " depth bits, "
				  << inSettings.stencilBits << " stencil bits, "
				  << (inSettings.attributeFlags == sf::ContextSettings::Attribute::Core ? "Core Profile" : "Compatibility Profile")
				  << std::endl;
	}(window.getSettings());

	// End of SFML setup

	GUI gui;
	ParticleSystem particleSystem = initializeSimulation();

	// Create a view with the same size as the window
	float initialViewScale = 1e10;
	sf::View simView(sf::FloatRect(0.0f, 0.0f, WINDOW_WIDTH * initialViewScale, WINDOW_HEIGHT * initialViewScale));
	simView.setCenter(0.0f, 0.0f);
	window.setView(simView);

	// Panning variables
	bool panning = false;
	sf::Vector2f mousePos;

	// Timing variables
	const int FRAME_RATE = 30;
	const float FRAME_TIME = 1.0f / FRAME_RATE;
	const int TIME_SCALE = 60 * 60 * 24 * 365;			  // 50 years per second
	const float INTEGRATION_TIME_STEP = 60 * 60 * 24 * 7; // 1 month

	sf::Clock clock;
	u_long elapsedTime = 0;
	bool paused = false;

	int prevElapsedYears = 0;

	// Render variables
	bool drawGravityField = false;
	Particle* selectedParticle = nullptr;

	// Main loop
	sf::Event event;

	window.setFramerateLimit(FRAME_RATE);
	window.clear();
	window.display();

	while (window.isOpen())
	{
		// Handle events
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			// Panning
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
			{
				panning = true;
				mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right)
			{
				panning = false;
			}

			if (event.type == sf::Event::MouseMoved && panning)
			{
				// If a particle is focused, unfocus it
				if (selectedParticle != nullptr)
				{
					selectedParticle = nullptr;
				}

				// Determine the new position in world coordinates
				const sf::Vector2f newPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
				const sf::Vector2f deltaPos = mousePos - newPos;

				// Move our view accordingly and update the window
				simView.setCenter(simView.getCenter() + deltaPos);
				window.setView(simView);

				// Save the new position as the old one
				// We're recalculating this, since we've changed the view
				mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			}

			// Focus on a particle when clicked
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				Eigen::Vector2f mousePos = util::toEigen(window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y)));
				selectedParticle = particleSystem.particleVisibleAt(mousePos, window);
			}

			// If + or - is pressed, zoom in or out with the mouse as the center point
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Dash || event.key.code == sf::Keyboard::Equal))
			{
				float zoomFactor;
				float zoomSpeed = 3.0f;
				if (event.key.code == sf::Keyboard::Dash)
				{
					zoomFactor = zoomSpeed;
				}

				if (event.key.code == sf::Keyboard::Equal)
				{
					zoomFactor = 1 / zoomSpeed;
				}

				sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
				sf::Vector2f oldCenter = simView.getCenter();
				sf::Vector2f newCenter = oldCenter + (mousePos - oldCenter) * (1 - zoomFactor);

				simView.setCenter(newCenter);
				simView.setSize(simView.getSize() * zoomFactor);
				window.setView(simView);
			}

			// If right arrow is pressed, simulate one frame
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Right)
			{
				double dt = 1.0 / 30.0;
				particleSystem.update(dt);
			}

			// If G is pressed, toggle gravity field drawing
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G)
			{
				drawGravityField = !drawGravityField;
			}

			// If space is pressed, pause or unpause the simulation
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
			{
				paused = !paused;
			}

			// If R is pressed, reset the simulation
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
			{
				particleSystem = initializeSimulation();
				selectedParticle = nullptr;
				elapsedTime = 0;
				prevElapsedYears = 0;
			}
		}

		// Calculate array of cells to draw filling the screen
		const int cellSize = 20; // pixels
		const int numCellsX = static_cast<int>(WINDOW_WIDTH / cellSize) + 1;
		const int numCellsY = static_cast<int>(WINDOW_HEIGHT / cellSize) + 1;

		const sf::Vector2f cellSizeWorld = window.mapPixelToCoords(sf::Vector2i(cellSize, cellSize)) - window.mapPixelToCoords(sf::Vector2i(0, 0));

		std::vector<sf::RectangleShape> cells;
		cells.reserve(numCellsX * numCellsY);

		for (int i = 0; i < numCellsX; ++i)
		{
			for (int j = 0; j < numCellsY; ++j)
			{
				// Create a cell
				sf::RectangleShape cell(cellSizeWorld);
				cell.setFillColor(sf::Color::Transparent);
				cell.setOutlineColor(sf::Color::White);

				// Make the outline 1 pixel thick regardless of zoom level
				sf::Vector2f outlineThicknessVector = window.mapPixelToCoords(sf::Vector2i(1, 0)) - window.mapPixelToCoords(sf::Vector2i(0, 0));
				cell.setOutlineThickness(outlineThicknessVector.x * 0);

				// Set the position in world coordinates
				cell.setPosition(window.mapPixelToCoords(sf::Vector2i(i * cellSize, j * cellSize)));
				cells.push_back(cell);
			}
		}

		// Calculate the potential energy for each cell
		std::vector<double> cellPotentials;
		for (auto& cell : cells)
		{
			const sf::Vector2f cellPos = cell.getPosition();
			const sf::Vector2f cellCenter = cellPos + cellSizeWorld / 2.0f;

			// If the cell is "close" to a particle, set the potential to 0
			if (particleSystem.isNearParticle(util::toEigen(cellCenter)))
			{
				cellPotentials.push_back(0.0);
				continue;
			}

			double cellPotential = -1 * particleSystem.calculatePotentialEnergy(util::toEigen(cellCenter));
			cellPotentials.push_back(cellPotential);
		}

		// Take the log of the potential energy
		for (auto& cellPotential : cellPotentials)
		{
			if (cellPotential != 0.0)
			{
				cellPotential = std::log(cellPotential);
			}
		}

		// Find the min and max potential (ignoring 0)
		double minPotential = std::numeric_limits<double>::max();
		double maxPotential = std::numeric_limits<double>::min();
		for (auto& cellPotential : cellPotentials)
		{
			if (cellPotential == 0.0)
			{
				continue;
			}

			if (cellPotential < minPotential)
			{
				minPotential = cellPotential;
			}

			if (cellPotential > maxPotential)
			{
				maxPotential = cellPotential;
			}
		}

		// Color the cells
		for (size_t i = 0; i < cells.size(); ++i)
		{
			const double cellPotential = cellPotentials[i];
			const double cellPotentialNormalized = (cellPotential - minPotential) / (maxPotential - minPotential);

			// If the cell potential was not calculated, set it to black
			if (cellPotential == 0.0)
			{
				cells[i].setFillColor(sf::Color::Black);
				continue;
			}

			const sf::Color cellColor = util::blueToRed(cellPotentialNormalized);
			cells[i].setFillColor(cellColor);
		}

		float dt = clock.restart().asSeconds();
		if (dt > 2 * FRAME_TIME)
		{
			std::cout << "Simulation cannot keep up! dt: " << dt << std::endl;
			std::cout << "Consider increasing integration time step, or reduce time scale." << std::endl;
			dt = 2 * FRAME_TIME;
		}

		else if (dt < FRAME_TIME / 2)
		{
			std::cout << "Low dt: " << dt << std::endl;
		}

		float majorTimeStep = dt * TIME_SCALE;
		int substeps = std::ceil(majorTimeStep / INTEGRATION_TIME_STEP);
		float subTimeStep = majorTimeStep / substeps;

		// Update the simulation
		if (!paused)
		{
			for (int i = 0; i < substeps; i++)
			{
				elapsedTime += subTimeStep;
				particleSystem.calculateForces();
				particleSystem.calculateTestForces();
				particleSystem.update(subTimeStep);
				particleSystem.lazyUpdate(subTimeStep);
			}
		}

		// Draw everything
		window.clear();

		// Draw gravity field
		if (drawGravityField)
		{
			for (const auto& cell : cells)
			{
				window.draw(cell);
			}
		}

		// Draw the particles
		particleSystem.draw(window);

		// Draw the trails
		for (auto& particle : particleSystem.getParticles())
		{
			particle.drawTrail(window);
		}

		// Draw the UI
		window.setView(window.getDefaultView());
		gui.draw(window);
		window.setView(simView);

		// If a particle is selected, center the view on it
		if (selectedParticle)
		{
			sf::Vector2f selectedParticlePos = util::toSFML(selectedParticle->getPosition().cast<float>());
			simView.setCenter(selectedParticlePos);

			selectedParticle->updateTrail();
			selectedParticle->drawTrail(window);
		}

		window.display();

		int elapsedYears = static_cast<int>(elapsedTime / (365.25 * 24 * 60 * 60));
		gui.setElapsedYears(elapsedYears);

		if (elapsedYears > prevElapsedYears)
		{
			prevElapsedYears = elapsedYears;

			if (elapsedYears % 100 == 0)
			{
				float fps = 1.0f / dt;
				std::cout << "Elapsed years: " << elapsedYears << " | FPS: " << fps << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}
