#include "Computer.hpp"
#include "Simulator.hpp"
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()

std::vector<dim::Vector4>	Computer::positions;
std::vector<dim::Vector4>	Computer::speeds;
std::vector<dim::Vector4>	Computer::accelerations;
std::vector<int> 			Computer::star_types;  // Add this line
cl::Buffer					Computer::positions_buffer;
cl::Buffer					Computer::speeds_buffer;
cl::Buffer					Computer::accelerations_buffer;
cl::Buffer					Computer::step_buffer;
cl::Buffer					Computer::smoothing_length_buffer;
cl::Buffer					Computer::interaction_rate_buffer;
cl::Buffer					Computer::black_hole_mass_buffer;
cl::Buffer 					Computer::types_buffer;  // Add this line
dim::Vector3 Computer::random_sphere()
{
	dim::Vector3 result = dim::Vector3::null;

	do
	{
		result.x = dim::random_float(-Simulator::galaxy_diameter / 2.f, Simulator::galaxy_diameter / 2.f);
		result.y = dim::random_float(-Simulator::galaxy_diameter / 2.f, Simulator::galaxy_diameter / 2.f);
		result.z = dim::random_float(-Simulator::galaxy_diameter / 2.f, Simulator::galaxy_diameter / 2.f);
	}
	while (result.get_norm() > Simulator::galaxy_diameter / 2.f);

	return result;
}

void Computer::initialize_star_types(int num_stars)
{
    star_types.resize(num_stars);
    srand(static_cast<unsigned int>(time(0)));  // Seed the random number generator

    for (int i = 0; i < num_stars; ++i)
    {
        star_types[i] = (rand() % 2 == 0) ? 1 : -1;  // Assign 1 for positive, -1 for negative
    }
}

void Computer::create_galaxy(int i)
{
	positions[i].set_norm(static_cast<float>(pow(positions[i].get_norm() / (Simulator::galaxy_diameter / 2.f), 5)) * (Simulator::galaxy_diameter / 2.f));
	positions[i].y *= Simulator::galaxy_thickness / Simulator::galaxy_diameter;
	speeds[i] = dim::Vector4(dim::normalize(dim::Vector3(positions[i]) ^ dim::Vector3(0.f, 1.f, 0.f)) * Simulator::stars_speed, 0.f);
}

void Computer::create_collision(int i)
{
	create_galaxy(i);

	if (i % 2)
		positions[i].x -= Simulator::galaxies_distance / 2.f;

	else
	{
		positions[i].x += Simulator::galaxies_distance / 2.f;
		std::swap(positions[i].y, positions[i].z);
		std::swap(speeds[i].y, speeds[i].z);
	}

}

void Computer::create_universe(int i)
{
	speeds[i] = positions[i];
	speeds[i].set_norm((speeds[i].get_norm() / (Simulator::galaxy_diameter / 2.f)) * Simulator::stars_speed);
}

void Computer::init()
{
	positions.clear();
	speeds.clear();
	accelerations.clear();
    star_types.clear();  // Add this line


	positions.resize(Simulator::nb_stars);
	speeds.resize(Simulator::nb_stars);
	accelerations.resize(Simulator::nb_stars, dim::Vector4::null);
    star_types.resize(Simulator::nb_stars);  // Add this line

    initialize_star_types(Simulator::nb_stars);  // Add this line


	for (int i = 0; i < Simulator::nb_stars; i++)
	{
		positions[i] = dim::Vector4(random_sphere(), 0.f);

		switch (Simulator::simulation_type)
		{
		case SimulationType::Galaxy: create_galaxy(i); break;
		case SimulationType::Collision: create_collision(i); break;
		case SimulationType::Universe: create_universe(i); break;
		default: break;
		}
	}

	positions_buffer = ComputeShader::Buffer(positions, Permissions::All);
	speeds_buffer = ComputeShader::Buffer(speeds, Permissions::All);
	accelerations_buffer = ComputeShader::Buffer(accelerations, Permissions::All);
	step_buffer = ComputeShader::Buffer(Simulator::step, Permissions::Read);
	smoothing_length_buffer = ComputeShader::Buffer(Simulator::smoothing_length, Permissions::Read);
	interaction_rate_buffer = ComputeShader::Buffer(Simulator::interaction_rate, Permissions::Read);
	black_hole_mass_buffer = ComputeShader::Buffer(Simulator::black_hole_mass, Permissions::Read);
	types_buffer = ComputeShader::Buffer(star_types, Permissions::Read);  // Add this line

}

void Computer::compute()
{
    step_buffer = ComputeShader::Buffer(Simulator::step, Permissions::Read);
    smoothing_length_buffer = ComputeShader::Buffer(Simulator::smoothing_length, Permissions::Read);
    interaction_rate_buffer = ComputeShader::Buffer(Simulator::interaction_rate, Permissions::Read);
    black_hole_mass_buffer = ComputeShader::Buffer(Simulator::black_hole_mass, Permissions::Read);
    types_buffer = ComputeShader::Buffer(Computer::star_types, Permissions::Read);
    cl::Buffer negative_attraction_constant_buffer = ComputeShader::Buffer(Menu::negative_attraction_constant, Permissions::Read);
    cl::Buffer repulsion_constant_buffer = ComputeShader::Buffer(Menu::repulsion_constant, Permissions::Read);

    // The interactions computations.
    ComputeShader::launch("interactions", { &positions_buffer, &accelerations_buffer, &interaction_rate_buffer,
        &smoothing_length_buffer, &black_hole_mass_buffer, &types_buffer, &negative_attraction_constant_buffer, &repulsion_constant_buffer },
        cl::NDRange(accelerations.size()));
    ComputeShader::get_data(accelerations_buffer, accelerations);

    // The integration computation.
    ComputeShader::launch("integration", { &positions_buffer, &speeds_buffer, &accelerations_buffer, &step_buffer }, cl::NDRange(speeds.size()));
    ComputeShader::get_data(positions_buffer, positions);
    ComputeShader::get_data(speeds_buffer, speeds);
}