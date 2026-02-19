#include "Computer.hpp"
#include <iostream>

cl::Buffer					Computer::positions_buffer;
cl::Buffer					Computer::speeds_buffer;
cl::Buffer					Computer::accelerations_buffer;
cl::Buffer					Computer::step_buffer;
cl::Buffer					Computer::smoothing_length_buffer;
cl::Buffer					Computer::interaction_rate_buffer;
cl::Buffer					Computer::black_hole_mass_buffer;
cl::Buffer 					Computer::types_buffer;
cl::Buffer					Computer::negative_attraction_constant_buffer;
cl::Buffer					Computer::repulsion_constant_buffer;

dim::Vector3 Computer::random_sphere(const SimulationConfig& config)
{
	dim::Vector3 result = dim::Vector3::null;

	do
	{
		result.x = dim::random_float(-config.galaxy_diameter / 2.f, config.galaxy_diameter / 2.f);
		result.y = dim::random_float(-config.galaxy_diameter / 2.f, config.galaxy_diameter / 2.f);
		result.z = dim::random_float(-config.galaxy_diameter / 2.f, config.galaxy_diameter / 2.f);
	}
	while (result.get_norm() > config.galaxy_diameter / 2.f);

	return result;
}

void Computer::create_galaxy(int i, const SimulationConfig& config, SimulationState& state)
{
	state.positions[i].set_norm(static_cast<float>(pow(state.positions[i].get_norm() / (config.galaxy_diameter / 2.f), 5)) * (config.galaxy_diameter / 2.f));
	state.positions[i].y *= config.galaxy_thickness / config.galaxy_diameter;
	state.speeds[i] = dim::Vector4(dim::normalize(dim::Vector3(state.positions[i]) ^ dim::Vector3(0.f, 1.f, 0.f)) * config.stars_speed, 0.f);
}

void Computer::create_collision(int i, const SimulationConfig& config, SimulationState& state)
{
	create_galaxy(i, config, state);

	if (i % 2)
		state.positions[i].x -= config.galaxies_distance / 2.f;

	else
	{
		state.positions[i].x += config.galaxies_distance / 2.f;
		std::swap(state.positions[i].y, state.positions[i].z);
		std::swap(state.speeds[i].y, state.speeds[i].z);
	}

}

void Computer::create_universe(int i, const SimulationConfig& config, SimulationState& state)
{
	state.speeds[i] = state.positions[i];
	state.speeds[i].set_norm((state.speeds[i].get_norm() / (config.galaxy_diameter / 2.f)) * config.stars_speed);
}

void Computer::initialize_star_types_and_positions(
	const SimulationConfig& config,
	SimulationState& state,
	int num_stars,
	float type_diameter)
{
	state.star_types.resize(num_stars);
	state.positions.resize(num_stars);
	state.speeds.resize(num_stars);
	state.accelerations.resize(num_stars, dim::Vector4::null);

	switch (Menu::matter_distribution)
	{
	case MatterDistribution::CoreHalo:
	{
		// Interpretation:
		// - Positive core diameter defines the positive sphere.
		// - Halo stars are always negative.
		// - core_extra_negative_density injects extra negatives inside core in [0, 1].
		const float core_radius = std::max(0.0f, type_diameter / 2.0f);
		const float extra_core_negative = std::max(0.0f, std::min(Menu::core_extra_negative_density, 1.0f));

		for (int i = 0; i < num_stars; ++i)
		{
				const dim::Vector3 position = random_sphere(config);
				state.positions[i] = dim::Vector4(position, 0.0f);
				const bool in_core = position.get_norm() <= core_radius;

				if (in_core)
					state.star_types[i] = (dim::random_float(0.f, 1.f) <= extra_core_negative) ? -1 : 1;
				else
					state.star_types[i] = -1;
			}
			break;
		}

	case MatterDistribution::RandomMix:
		for (int i = 0; i < num_stars; ++i)
		{
			const dim::Vector3 position = random_sphere(config);
			state.positions[i] = dim::Vector4(position, 0.0f);
			state.star_types[i] = (dim::random_float(0.f, 1.f) <= Menu::positive_ratio) ? 1 : -1;
		}
		break;

	case MatterDistribution::SplitX:
		for (int i = 0; i < num_stars; ++i)
		{
			const dim::Vector3 position = random_sphere(config);
			state.positions[i] = dim::Vector4(position, 0.0f);
			state.star_types[i] = (state.positions[i].x <= 0.0f) ? 1 : -1;
		}
		break;
	}
}

void Computer::init(const SimulationConfig& config, SimulationState& state)
{
	state.positions.clear();
	state.speeds.clear();
	state.accelerations.clear();
	state.star_types.clear();

	initialize_star_types_and_positions(config, state, config.nb_stars, Menu::type_diameter);

	for (int i = 0; i < config.nb_stars; i++)
	{
		switch (config.simulation_type)
		{
		case SimulationType::Galaxy: create_galaxy(i, config, state); break;
		case SimulationType::Collision: create_collision(i, config, state); break;
		case SimulationType::Universe: create_universe(i, config, state); break;
		default: break;
		}
	}

	// Core/halo typing must be applied after initial placement shaping.
	// Otherwise stars moved by create_galaxy/create_collision can cross the core boundary.
	if (Menu::matter_distribution == MatterDistribution::CoreHalo)
	{
		const float core_radius = std::max(0.0f, Menu::type_diameter / 2.0f);
		const float extra_core_negative = std::max(0.0f, std::min(Menu::core_extra_negative_density, 1.0f));
		for (int i = 0; i < config.nb_stars; ++i)
		{
			if (state.positions[i].get_norm() <= core_radius)
				state.star_types[i] = (dim::random_float(0.f, 1.f) <= extra_core_negative) ? -1 : 1;
			else
				state.star_types[i] = -1;
		}
	}

	int positive_count = 0;
	int negative_count = 0;
	int core_count = 0;
	int halo_count = 0;
	const float core_radius = std::max(0.0f, Menu::type_diameter / 2.0f);
	for (int i = 0; i < config.nb_stars; ++i)
	{
		if (state.star_types[i] > 0) positive_count++;
		else negative_count++;

		if (state.positions[i].get_norm() <= core_radius) core_count++;
		else halo_count++;
	}
	std::cout << "[Init] stars+=" << positive_count
		<< " stars-=" << negative_count
		<< " core=" << core_count
		<< " halo=" << halo_count
		<< " mode=" << static_cast<int>(Menu::matter_distribution)
		<< std::endl;

	positions_buffer = ComputeShader::Buffer(state.positions, Permissions::All);
	speeds_buffer = ComputeShader::Buffer(state.speeds, Permissions::All);
	accelerations_buffer = ComputeShader::Buffer(state.accelerations, Permissions::All);
	float step = config.step;
	float smoothing_length = config.smoothing_length;
	float interaction_rate = config.interaction_rate;
	float black_hole_mass = config.black_hole_mass;
	step_buffer = ComputeShader::Buffer(step, Permissions::Read);
	smoothing_length_buffer = ComputeShader::Buffer(smoothing_length, Permissions::Read);
	interaction_rate_buffer = ComputeShader::Buffer(interaction_rate, Permissions::Read);
	black_hole_mass_buffer = ComputeShader::Buffer(black_hole_mass, Permissions::Read);
	types_buffer = ComputeShader::Buffer(state.star_types, Permissions::Read);
	negative_attraction_constant_buffer = ComputeShader::Buffer(Menu::negative_attraction_constant, Permissions::Read);
	repulsion_constant_buffer = ComputeShader::Buffer(Menu::repulsion_constant, Permissions::Read);
}

void Computer::compute(const SimulationConfig& config, SimulationState& state)
{
	float step = config.step;
	float smoothing_length = config.smoothing_length;
	float interaction_rate = config.interaction_rate;
	float black_hole_mass = config.black_hole_mass;
	ComputeShader::set_data(step_buffer, step);
	ComputeShader::set_data(smoothing_length_buffer, smoothing_length);
	ComputeShader::set_data(interaction_rate_buffer, interaction_rate);
	ComputeShader::set_data(black_hole_mass_buffer, black_hole_mass);
	ComputeShader::set_data(negative_attraction_constant_buffer, Menu::negative_attraction_constant);
	ComputeShader::set_data(repulsion_constant_buffer, Menu::repulsion_constant);

	// The interactions computations.
	ComputeShader::launch("interactions", { &positions_buffer, &accelerations_buffer, &interaction_rate_buffer,
		&smoothing_length_buffer, &black_hole_mass_buffer, &types_buffer, &negative_attraction_constant_buffer, &repulsion_constant_buffer },
		cl::NDRange(state.accelerations.size()));

	// The integration computation.
	ComputeShader::launch("integration", { &positions_buffer, &speeds_buffer, &accelerations_buffer, &step_buffer }, cl::NDRange(state.speeds.size()));
	ComputeShader::get_data(positions_buffer, state.positions);
	ComputeShader::get_data(speeds_buffer, state.speeds);
}
