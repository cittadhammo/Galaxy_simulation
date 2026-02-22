#include <dim/dimension3D.hpp>
#include "Simulator.hpp"
#include <GL/glew.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <exception>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdint>


// Function to print OpenGL context information
void print_opengl_info()
{
    const GLubyte* renderer = glGetString(GL_RENDERER); // Get renderer string
    const GLubyte* vendor = glGetString(GL_VENDOR); // Get vendor string
    const GLubyte* version = glGetString(GL_VERSION); // Version as a string
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // GLSL version string

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    std::cout << "OpenGL Renderer: " << renderer << std::endl;
    std::cout << "OpenGL Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Version: " << version << std::endl;
    std::cout << "GLSL Version: " << glslVersion << std::endl;
    std::cout << "OpenGL Version: " << major << "." << minor << std::endl;
}

struct BatchOptions
{
	bool enabled = false;
	bool physics_only = false;
	int steps = 0;
	int snapshots = 3;
	int snapshot_width = 0;
	int snapshot_height = 0;
	std::string output_dir = "outputs";
	std::string config_path;
	std::string batch_config_path;
	std::string state_in_path;
};

static int parse_int(const char* value, int fallback)
{
	if (value == nullptr)
		return fallback;
	try
	{
		return std::stoi(value);
	}
	catch (const std::exception&)
	{
		return fallback;
	}
}

static BatchOptions parse_args(int argc, char** argv)
{
	BatchOptions options;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--batch-steps" && i + 1 < argc)
		{
			options.steps = std::max(0, parse_int(argv[++i], 0));
			options.enabled = (options.steps > 0);
		}
		else if (arg == "--snapshots" && i + 1 < argc)
			options.snapshots = std::max(1, parse_int(argv[++i], 3));
		else if (arg == "--output-dir" && i + 1 < argc)
			options.output_dir = argv[++i];
		else if (arg == "--config" && i + 1 < argc)
			options.config_path = argv[++i];
		else if (arg == "--batch-config" && i + 1 < argc)
			options.batch_config_path = argv[++i];
		else if (arg == "--state-in" && i + 1 < argc)
			options.state_in_path = argv[++i];
		else if (arg == "--physics-only")
			options.physics_only = true;
		else if (arg == "--snapshot-width" && i + 1 < argc)
			options.snapshot_width = std::max(0, parse_int(argv[++i], 0));
		else if (arg == "--snapshot-height" && i + 1 < argc)
			options.snapshot_height = std::max(0, parse_int(argv[++i], 0));
	}

	return options;
}

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

static bool parse_camera_view(const std::string& text, Simulator::CameraView& value)
{
	if (text == "isometric" || text == "iso" || text == "0") { value = Simulator::CameraView::Isometric; return true; }
	if (text == "top" || text == "1") { value = Simulator::CameraView::Top; return true; }
	return false;
}

using ConfigKV = std::unordered_map<std::string, std::string>;

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

static std::vector<ConfigKV> load_config_jobs(const std::string& path)
{
	std::ifstream file(path);
	std::vector<ConfigKV> jobs;

	if (!file.is_open())
		return jobs;

	std::string line;
	while (std::getline(file, line))
	{
		ConfigKV kv = parse_config_line(line);
		if (!kv.empty())
			jobs.push_back(std::move(kv));
	}

	return jobs;
}

static bool apply_config_kv(const ConfigKV& kv, BatchOptions& batch, bool* output_dir_overridden = nullptr)
{
	if (output_dir_overridden != nullptr)
		*output_dir_overridden = false;

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
	if (const std::string* v = get("camera_view"))
	{
		Simulator::CameraView view = Simulator::camera_view;
		if (parse_camera_view(*v, view))
			Simulator::camera_view = view;
	}
	if (const std::string* v = get("camera_angle"))
	{
		float parsed = 0.0f;
		if (to_float(*v, parsed))
			Simulator::camera_angle = parsed;
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
	if (const std::string* v = get("stars_speed"))
	{
		float parsed = Menu::stars_speed;
		if (to_float(*v, parsed))
		{
			Menu::stars_speed = parsed;
			Menu::positive_stars_speed = parsed;
			Menu::negative_stars_speed = parsed;
		}
	}
	set_float("positive_stars_speed", Menu::positive_stars_speed);
	set_float("negative_stars_speed", Menu::negative_stars_speed);
	// Compatibility with older color naming for matter types.
	set_float("blue_stars_speed", Menu::positive_stars_speed);
	set_float("red_stars_speed", Menu::negative_stars_speed);
	Menu::stars_speed = 0.5f * (Menu::positive_stars_speed + Menu::negative_stars_speed);
	set_float("black_hole_mass", Menu::black_hole_mass);
	set_float("negative_attraction_constant", Menu::negative_attraction_constant);
	set_float("repulsion_constant", Menu::repulsion_constant);
	set_float("red_bloom_intensity", Menu::red_bloom_intensity);
	set_float("blue_bloom_intensity", Menu::blue_bloom_intensity);
	set_float("type_diameter", Menu::type_diameter);
	set_float("positive_ratio", Menu::positive_ratio);
	set_float("core_extra_negative_density", Menu::core_extra_negative_density);

	if (const std::string* v = get("batch_steps"))
	{
		int parsed = batch.steps;
		if (to_int(*v, parsed))
		{
			batch.steps = std::max(0, parsed);
			batch.enabled = (batch.steps > 0);
		}
	}
	if (const std::string* v = get("snapshots"))
	{
		int parsed = batch.snapshots;
		if (to_int(*v, parsed))
			batch.snapshots = std::max(1, parsed);
	}
	if (const std::string* v = get("output_dir"))
	{
		batch.output_dir = *v;
		if (output_dir_overridden != nullptr)
			*output_dir_overridden = true;
	}
	if (const std::string* v = get("snapshot_width"))
	{
		int parsed = batch.snapshot_width;
		if (to_int(*v, parsed))
			batch.snapshot_width = std::max(0, parsed);
	}
	if (const std::string* v = get("snapshot_height"))
	{
		int parsed = batch.snapshot_height;
		if (to_int(*v, parsed))
			batch.snapshot_height = std::max(0, parsed);
	}

	return true;
}

static bool save_window_snapshot(const std::string& path)
{
	sf::RenderWindow& window = dim::Window::get_window();
	const sf::Vector2u size = window.getSize();
	sf::Texture texture;

	if (!texture.create(size.x, size.y))
		return false;

	texture.update(window);
	const sf::Image image = texture.copyToImage();
	return image.saveToFile(path);
}

static bool load_state_file(const std::string& path, SimulationState& state)
{
	struct StateHeader
	{
		uint32_t magic;
		uint32_t version;
		uint32_t nb_stars;
	};

	std::ifstream in(path, std::ios::binary);
	if (!in.is_open())
		return false;

	StateHeader header{};
	in.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!in.good())
		return false;

	if (header.magic != 0x47414C58u || header.version != 1u)
		return false;

	const size_t n = static_cast<size_t>(header.nb_stars);
	state.positions.resize(n);
	state.speeds.resize(n);
	state.star_types.resize(n);
	state.accelerations.assign(n, dim::Vector4::null);

	in.read(reinterpret_cast<char*>(state.positions.data()), static_cast<std::streamsize>(n * sizeof(dim::Vector4)));
	in.read(reinterpret_cast<char*>(state.speeds.data()), static_cast<std::streamsize>(n * sizeof(dim::Vector4)));
	in.read(reinterpret_cast<char*>(state.star_types.data()), static_cast<std::streamsize>(n * sizeof(int)));
	return in.good();
}

static float estimate_snapshot_radius(const SimulationState& state)
{
	float max_norm = 0.0f;
	for (const dim::Vector4& p : state.positions)
		max_norm = std::max(max_norm, static_cast<float>(p.get_norm()));

	// Keep a sane default for compact states and add margin for larger ones.
	return std::max(130.0f, max_norm * 1.2f);
}

static void set_snapshot_camera_angle(float theta, float radius, float phi = dim::pi / 3.f)
{
	dim::Camera& camera = dim::Window::get_camera();
	const dim::Vector3 position = dim::Vector3::Spherical(radius, theta, phi);
	camera.set_position(position);
	camera.set_direction(-position);
}

static void set_top_snapshot_camera(float radius)
{
	dim::Camera& camera = dim::Window::get_camera();
	const dim::Vector3 position(0.0f, radius, 0.001f);
	camera.set_position(position);
	camera.set_direction(-position);
}

static int run_batch_mode(const BatchOptions& options, bool close_window, bool compute_steps = true)
{
	if (compute_steps)
		std::cout << "Running batch mode for " << options.steps << " steps..." << std::endl;
	else
		std::cout << "Running batch mode from loaded state (no physics stepping)." << std::endl;

	if (compute_steps)
	{
		Menu::pause = false;
		for (int i = 0; i < options.steps && dim::Window::running; ++i)
		{
			Simulator::menu_update();
			Computer::compute(Simulator::config, Simulator::state);
		}
	}
	else
		Menu::pause = true;

	Renderer::update_vbo(Simulator::state);

	std::error_code ec;
	std::filesystem::create_directories(options.output_dir, ec);
	if (ec)
		std::cerr << "Warning: failed to create output directory: " << options.output_dir << std::endl;

	const float snapshot_radius = estimate_snapshot_radius(Simulator::state);
	std::cerr << "[DEBUG] camera_angle = " << Simulator::camera_angle << ", snapshots = " << options.snapshots << std::endl;
	for (int i = 0; i < options.snapshots; ++i)
	{
		if (Simulator::camera_angle >= 0.0f || options.snapshots > 1)
		{
			const float theta = (options.snapshots > 1) ? 
				(2.f * dim::pi * static_cast<float>(i)) / static_cast<float>(options.snapshots) : 0.f;
			float phi = dim::pi / 3.f;
			if (Simulator::camera_angle >= 0.0f)
				phi = (90.0f - Simulator::camera_angle) * dim::pi / 180.0f;
			set_snapshot_camera_angle(theta, snapshot_radius, phi);
		}
		else
		{
			set_top_snapshot_camera(snapshot_radius);
		}

		// Warm up one frame before capture so post-process buffers are populated.
		for (int pass = 0; pass < 2; ++pass)
		{
			dim::Window::clear(dim::Color::black);
			dim::Window::update();
			Renderer::clear();
			Simulator::draw();
			dim::Window::display();
		}

		const std::string path = options.output_dir + "/snapshot_" + std::to_string(i) + ".png";
		if (save_window_snapshot(path))
			std::cout << "Saved " << path << std::endl;
		else
			std::cerr << "Failed to save " << path << std::endl;
	}

	if (close_window)
		dim::Window::close();
	return EXIT_SUCCESS;
}

static int run_physics_only_mode(const BatchOptions& options)
{
	const int steps = options.enabled ? options.steps : 2000;
	std::cout << "Running physics-only mode for " << steps << " steps..." << std::endl;

	Menu::pause = false;
	for (int i = 0; i < steps; ++i)
	{
		Simulator::menu_update();
		Computer::compute(Simulator::config, Simulator::state);
	}

	std::cout << "Physics-only run complete." << std::endl;
	return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	const BatchOptions batch_options = parse_args(argc, argv);
	BatchOptions mutable_batch_options = batch_options;

	if (!mutable_batch_options.physics_only)
	{
		if (mutable_batch_options.snapshot_width > 0 && mutable_batch_options.snapshot_height > 0)
			dim::Window::open("Galaxy simulation",
				static_cast<unsigned int>(mutable_batch_options.snapshot_width),
				static_cast<unsigned int>(mutable_batch_options.snapshot_height),
				"resources/icons/icon.png");
		else
			dim::Window::open("Galaxy simulation", 0.75f, "resources/icons/icon.png");
	}
	Simulator::init(!mutable_batch_options.physics_only);

	if (!mutable_batch_options.physics_only)
	{
		// Print OpenGL context information
		print_opengl_info();
	}

	if (!mutable_batch_options.config_path.empty())
	{
		const std::vector<ConfigKV> jobs = load_config_jobs(mutable_batch_options.config_path);
		if (jobs.empty())
			std::cerr << "Warning: no valid config line found in " << mutable_batch_options.config_path << std::endl;
		else
		{
			// Merge all config lines together
			ConfigKV merged;
			for (const auto& job : jobs)
			{
				for (const auto& pair : job)
					merged[pair.first] = pair.second;
			}
			apply_config_kv(merged, mutable_batch_options);
			Simulator::restart();
			Simulator::apply_camera_view();
		}
	}

	if (!mutable_batch_options.state_in_path.empty())
	{
		if (mutable_batch_options.physics_only || !mutable_batch_options.batch_config_path.empty())
		{
			std::cerr << "Error: --state-in does not support --physics-only or --batch-config." << std::endl;
			if (!mutable_batch_options.physics_only)
				dim::Window::close();
			return EXIT_FAILURE;
		}

		SimulationState loaded_state;
		if (!load_state_file(mutable_batch_options.state_in_path, loaded_state))
		{
			std::cerr << "Error: failed to load state file: " << mutable_batch_options.state_in_path << std::endl;
			dim::Window::close();
			return EXIT_FAILURE;
		}

		Simulator::state = std::move(loaded_state);
		Simulator::config.nb_stars = static_cast<int>(Simulator::state.positions.size());
		Simulator::nb_stars = Simulator::config.nb_stars;
		Menu::nb_stars = Simulator::config.nb_stars;
		Renderer::init_vbo(Simulator::state);
		Renderer::update_vbo(Simulator::state);
		Menu::pause = true;
		Simulator::computation_done = true;

		std::cout << "[State] loaded " << Simulator::state.positions.size()
			<< " stars from " << mutable_batch_options.state_in_path << std::endl;
	}

	if (!mutable_batch_options.batch_config_path.empty())
	{
		const std::vector<ConfigKV> jobs = load_config_jobs(mutable_batch_options.batch_config_path);
		if (jobs.empty())
		{
			std::cerr << "Error: no valid batch jobs found in " << mutable_batch_options.batch_config_path << std::endl;
			dim::Window::close();
			return EXIT_FAILURE;
		}

		for (size_t i = 0; i < jobs.size(); ++i)
		{
			BatchOptions job_options = mutable_batch_options;
			bool output_dir_overridden = false;
			apply_config_kv(jobs[i], job_options, &output_dir_overridden);

			if (!job_options.enabled)
			{
				std::cerr << "Skipping job " << (i + 1) << ": missing batch_steps" << std::endl;
				continue;
			}

			if (!output_dir_overridden && jobs.size() > 1)
				job_options.output_dir = mutable_batch_options.output_dir + "/run_" + std::to_string(i + 1);

			std::cout << "[Batch] Job " << (i + 1) << "/" << jobs.size() << std::endl;
			Simulator::restart();
			if (!mutable_batch_options.physics_only)
			{
				Simulator::apply_camera_view();
				run_batch_mode(job_options, false);
			}
			else
				run_physics_only_mode(job_options);
		}

		if (!mutable_batch_options.physics_only)
			dim::Window::close();
		return EXIT_SUCCESS;
	}

	if (mutable_batch_options.physics_only)
		return run_physics_only_mode(mutable_batch_options);

	if (mutable_batch_options.enabled)
		return run_batch_mode(mutable_batch_options, true, mutable_batch_options.state_in_path.empty());

	// The computation thread.
	std::thread simulation_thread([]()
	{
		while (dim::Window::running)
			Simulator::compute_update();
	});

	// The render thread.
	while (dim::Window::running)
	{
		// Check the events.
		sf::Event sf_event;
		while (dim::Window::poll_event(sf_event))
		{
			dim::Window::check_events(sf_event);
			Menu::check_events(sf_event);
			Simulator::check_events(sf_event);
		}

		const bool ui_captures_mouse = Menu::visible && (Menu::active || ImGui::GetIO().WantCaptureMouse);
		dim::Window::get_controller().enable(!ui_captures_mouse, dim::Controller::Action::All);

		dim::Window::clear(dim::Color::black);
		dim::Window::update();

		Simulator::render_update();
		Simulator::draw();

		Menu::display();
		dim::Window::display();
	}

	simulation_thread.join();
	dim::Window::close();
	return EXIT_SUCCESS;
}
