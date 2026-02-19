#include "Computer.hpp"
#include "ComputeShader.hpp"
#include "Menu.hpp"
#include "SimulationData.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

bool			Menu::visible = false;
bool			Menu::active = false;
bool			Menu::pause = false;
SimulationType	Menu::simulation_type = SimulationType::Galaxy;
float			Menu::step = 0.001f;
float			Menu::smoothing_length = 1.f;
float			Menu::interaction_rate = 0.05f;
int				Menu::nb_stars = 20000;
float			Menu::galaxy_diameter = 100.f;
float			Menu::galaxy_thickness = 5.f;
float			Menu::galaxies_distance = 75.f;
float			Menu::stars_speed = 20.f;
float			Menu::black_hole_mass = 1000.f;
float			Menu::negative_attraction_constant = 1.0f;
float			Menu::repulsion_constant = 1.0f;
float			Menu::type_diameter = 50.0f;
MatterDistribution Menu::matter_distribution = MatterDistribution::CoreHalo;
float			Menu::positive_ratio = 0.5f;
float			Menu::core_extra_negative_density = 0.0f;

static void set_default_values_headless()
{
	Menu::step = 0.001f;
	Menu::smoothing_length = 1.f;
	Menu::galaxies_distance = 75.f;
	Menu::black_hole_mass = 1000.f;
	Menu::negative_attraction_constant = 1.0f;
	Menu::repulsion_constant = 1.0f;
	Menu::type_diameter = 50.0f;
	Menu::matter_distribution = MatterDistribution::CoreHalo;
	Menu::positive_ratio = 0.5f;
	Menu::core_extra_negative_density = 0.0f;

	switch (Menu::simulation_type)
	{
	case SimulationType::Galaxy:
		Menu::interaction_rate = 0.05f;
		Menu::nb_stars = 20000;
		Menu::galaxy_diameter = 100.f;
		Menu::galaxy_thickness = 5.f;
		Menu::stars_speed = 20.f;
		break;

	case SimulationType::Collision:
		Menu::interaction_rate = 0.05f;
		Menu::nb_stars = 20000;
		Menu::galaxy_diameter = 50.f;
		Menu::galaxy_thickness = 2.5f;
		Menu::stars_speed = 20.f;
		break;

	case SimulationType::Universe:
		Menu::interaction_rate = 0.005f;
		Menu::nb_stars = 100000;
		Menu::galaxy_diameter = 10.f;
		Menu::galaxy_thickness = 10.f;
		Menu::stars_speed = 185.f;
		break;

	default:
		break;
	}
}

struct PhysicsOptions
{
	int steps = 2000;
	std::string config_path;
	std::string state_out;
};

using ConfigKV = std::unordered_map<std::string, std::string>;

static bool to_float(const std::string& text, float& value)
{
	try { value = std::stof(text); return true; } catch (...) { return false; }
}

static bool to_int(const std::string& text, int& value)
{
	try { value = std::stoi(text); return true; } catch (...) { return false; }
}

static bool parse_simulation_type(const std::string& text, SimulationType& value)
{
	if (text == "Galaxy" || text == "0") { value = SimulationType::Galaxy; return true; }
	if (text == "Collision" || text == "1") { value = SimulationType::Collision; return true; }
	if (text == "Universe" || text == "2") { value = SimulationType::Universe; return true; }
	return false;
}

static bool parse_matter_distribution(const std::string& text, MatterDistribution& value)
{
	if (text == "CoreHalo" || text == "Core+halo" || text == "0") { value = MatterDistribution::CoreHalo; return true; }
	if (text == "RandomMix" || text == "1") { value = MatterDistribution::RandomMix; return true; }
	if (text == "SplitX" || text == "2") { value = MatterDistribution::SplitX; return true; }
	return false;
}

static ConfigKV parse_config_line(const std::string& raw)
{
	std::string line = raw;
	const size_t comment_pos = line.find('#');
	if (comment_pos != std::string::npos)
		line = line.substr(0, comment_pos);

	std::istringstream in(line);
	std::string token;
	ConfigKV values;

	while (in >> token)
	{
		if (token == "SIMCFG" || token == "BATCHCFG")
			continue;

		const size_t eq = token.find('=');
		if (eq == std::string::npos || eq == 0 || eq + 1 >= token.size())
			continue;

		values[token.substr(0, eq)] = token.substr(eq + 1);
	}

	return values;
}

static ConfigKV load_first_config(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		return {};

	std::string line;
	while (std::getline(file, line))
	{
		ConfigKV kv = parse_config_line(line);
		if (!kv.empty())
			return kv;
	}

	return {};
}

static void apply_config_kv(const ConfigKV& kv, PhysicsOptions& options)
{
	auto get = [&](const char* key) -> const std::string*
	{
		const auto it = kv.find(key);
		return (it == kv.end()) ? nullptr : &it->second;
	};

	if (const std::string* v = get("simulation_type"))
	{
		SimulationType type = Menu::simulation_type;
		if (parse_simulation_type(*v, type))
			Menu::simulation_type = type;
	}
	if (const std::string* v = get("matter_distribution"))
	{
		MatterDistribution distribution = Menu::matter_distribution;
		if (parse_matter_distribution(*v, distribution))
			Menu::matter_distribution = distribution;
	}

	auto set_float = [&](const char* key, float& target)
	{
		if (const std::string* v = get(key))
		{
			float parsed = target;
			if (to_float(*v, parsed))
				target = parsed;
		}
	};
	auto set_int = [&](const char* key, int& target)
	{
		if (const std::string* v = get(key))
		{
			int parsed = target;
			if (to_int(*v, parsed))
				target = parsed;
		}
	};

	set_float("step", Menu::step);
	set_float("smoothing_length", Menu::smoothing_length);
	set_float("interaction_rate", Menu::interaction_rate);
	set_int("nb_stars", Menu::nb_stars);
	set_float("galaxy_diameter", Menu::galaxy_diameter);
	set_float("galaxy_thickness", Menu::galaxy_thickness);
	set_float("galaxies_distance", Menu::galaxies_distance);
	set_float("stars_speed", Menu::stars_speed);
	set_float("black_hole_mass", Menu::black_hole_mass);
	set_float("negative_attraction_constant", Menu::negative_attraction_constant);
	set_float("repulsion_constant", Menu::repulsion_constant);
	set_float("type_diameter", Menu::type_diameter);
	set_float("positive_ratio", Menu::positive_ratio);
	set_float("core_extra_negative_density", Menu::core_extra_negative_density);

	if (const std::string* v = get("batch_steps"))
	{
		int parsed = options.steps;
		if (to_int(*v, parsed))
			options.steps = std::max(0, parsed);
	}
}

static PhysicsOptions parse_args(int argc, char** argv)
{
	PhysicsOptions options;

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];
		if (arg == "--steps" && i + 1 < argc)
		{
			int parsed = options.steps;
			if (to_int(argv[++i], parsed))
				options.steps = std::max(0, parsed);
		}
		else if (arg == "--config" && i + 1 < argc)
			options.config_path = argv[++i];
		else if (arg == "--state-out" && i + 1 < argc)
			options.state_out = argv[++i];
	}

	return options;
}

static bool write_state_file(const std::string& path, const SimulationState& state)
{
	struct Header
	{
		uint32_t magic = 0x47414C58; // GALX
		uint32_t version = 1;
		uint32_t nb_stars = 0;
	};

	std::ofstream out(path, std::ios::binary);
	if (!out.is_open())
		return false;

	const Header header{0x47414C58u, 1u, static_cast<uint32_t>(state.positions.size())};
	out.write(reinterpret_cast<const char*>(&header), sizeof(header));
	out.write(reinterpret_cast<const char*>(state.positions.data()), static_cast<std::streamsize>(state.positions.size() * sizeof(dim::Vector4)));
	out.write(reinterpret_cast<const char*>(state.speeds.data()), static_cast<std::streamsize>(state.speeds.size() * sizeof(dim::Vector4)));
	out.write(reinterpret_cast<const char*>(state.star_types.data()), static_cast<std::streamsize>(state.star_types.size() * sizeof(int)));
	return out.good();
}

int main(int argc, char** argv)
{
	PhysicsOptions options = parse_args(argc, argv);

	set_default_values_headless();
	if (!options.config_path.empty())
	{
		const ConfigKV kv = load_first_config(options.config_path);
		if (kv.empty())
			std::cerr << "Warning: no valid config line found in " << options.config_path << std::endl;
		else
			apply_config_kv(kv, options);
	}

	SimulationConfig config;
	config.simulation_type = Menu::simulation_type;
	config.step = Menu::step;
	config.smoothing_length = Menu::smoothing_length;
	config.interaction_rate = Menu::interaction_rate;
	config.nb_stars = Menu::nb_stars;
	config.galaxy_diameter = Menu::galaxy_diameter;
	config.galaxy_thickness = Menu::galaxy_thickness;
	config.galaxies_distance = Menu::galaxies_distance;
	config.stars_speed = Menu::stars_speed;
	config.black_hole_mass = Menu::black_hole_mass;

	SimulationState state;

	ComputeShader::init("shaders/compute/cl_compute_shader.cl");
	Computer::init(config, state);

	std::cout << "[Physics] running " << options.steps << " steps, stars=" << config.nb_stars << std::endl;
	for (int i = 0; i < options.steps; ++i)
		Computer::compute(config, state);
	std::cout << "[Physics] done" << std::endl;

	if (!options.state_out.empty())
	{
		if (write_state_file(options.state_out, state))
			std::cout << "[Physics] wrote state: " << options.state_out << std::endl;
		else
		{
			std::cerr << "Error: failed to write state file: " << options.state_out << std::endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
