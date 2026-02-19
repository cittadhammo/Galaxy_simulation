#ifndef SIMULATION_DATA_HPP
#define SIMULATION_DATA_HPP

#include "Menu.hpp"

struct SimulationConfig
{
	SimulationType simulation_type = SimulationType::Galaxy;
	float step = 0.0f;
	float smoothing_length = 0.0f;
	float interaction_rate = 0.0f;
	int nb_stars = 0;
	float galaxy_diameter = 0.0f;
	float galaxy_thickness = 0.0f;
	float galaxies_distance = 0.0f;
	float stars_speed = 0.0f;
	float positive_stars_speed = 0.0f;
	float negative_stars_speed = 0.0f;
	float black_hole_mass = 0.0f;
};

struct SimulationState
{
	std::vector<dim::Vector4> positions;
	std::vector<dim::Vector4> speeds;
	std::vector<dim::Vector4> accelerations;
	std::vector<int> star_types;
};

#endif
