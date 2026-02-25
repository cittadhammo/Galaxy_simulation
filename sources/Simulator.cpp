#include "Simulator.hpp"
#include <dim/controllers/OrbitController.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

std::atomic<bool>	Simulator::computation_done;
std::atomic<bool>	Simulator::reload;
std::atomic<bool>	Simulator::waiting;
SimulationType		Simulator::simulation_type;
float				Simulator::step;
float				Simulator::smoothing_length;
float				Simulator::interaction_rate;
int					Simulator::nb_stars;
float				Simulator::galaxy_diameter;
float				Simulator::galaxy_thickness;
float				Simulator::galaxies_distance;
float				Simulator::stars_speed;
float				Simulator::positive_stars_speed;
float				Simulator::negative_stars_speed;
float				Simulator::black_hole_mass;
SimulationConfig	Simulator::config;
SimulationState		Simulator::state;
bool				Simulator::renderer_enabled = true;
Simulator::CameraView Simulator::camera_view = Simulator::CameraView::Isometric;
float Simulator::camera_angle = -1.0f;

void Simulator::init(bool enable_renderer)
{
	renderer_enabled = enable_renderer;
	if (renderer_enabled)
	{
		dim::PerspectiveCamera cam(45.f, 1.f, 10000.f);
		cam.set_position(dim::Vector3(0.f, 0.f, 130.f));
		dim::Window::set_camera(cam);
		dim::Window::set_controller(dim::OrbitController(dim::Vector3::null, dim::OrbitController::default_sensitivity, 2.f));
		{
			dim::Controller& controller = dim::Window::get_controller();
			if (controller.get_type() == dim::Controller::Type::Orbit)
				static_cast<dim::OrbitController&>(controller).set_pan_multiplier(Menu::camera_pan_speed);
		}
		apply_camera_view();

		dim::Shader::add("galaxy", "shaders/galaxy.vert", "shaders/galaxy.frag");
		dim::Shader::add("blur", "shaders/blur.vert", "shaders/blur.frag");
		dim::Shader::add("post", "shaders/post.vert", "shaders/post.frag");
		dim::Shader::add("ruler", "shaders/ruler.vert", "shaders/ruler.frag");
	}

	ComputeShader::init("shaders/compute/cl_compute_shader.cl");
	Menu::set_default_values();
	Simulator::restart();
}

void Simulator::restart()
{
	simulation_type = Menu::simulation_type;
	step = Menu::step;
	smoothing_length = Menu::smoothing_length;
	interaction_rate = Menu::interaction_rate;
	nb_stars = Menu::nb_stars;
	galaxy_diameter = Menu::galaxy_diameter;
	galaxy_thickness = Menu::galaxy_thickness;
	galaxies_distance = Menu::galaxies_distance;
	stars_speed = Menu::stars_speed;
	positive_stars_speed = Menu::positive_stars_speed;
	negative_stars_speed = Menu::negative_stars_speed;
	black_hole_mass = Menu::black_hole_mass;
	config.simulation_type = simulation_type;
	config.step = step;
	config.smoothing_length = smoothing_length;
	config.interaction_rate = interaction_rate;
	config.nb_stars = nb_stars;
	config.galaxy_diameter = galaxy_diameter;
	config.galaxy_thickness = galaxy_thickness;
	config.galaxies_distance = galaxies_distance;
	config.stars_speed = stars_speed;
	config.positive_stars_speed = positive_stars_speed;
	config.negative_stars_speed = negative_stars_speed;
	config.black_hole_mass = black_hole_mass;

	Computer::init(config, state);
	if (renderer_enabled)
		Renderer::init(state);
	print_configuration();

	computation_done = false;
	waiting = false;
	reload = false;
}

void Simulator::print_configuration()
{
	std::cout << configuration_line() << std::endl;
}

std::string Simulator::configuration_line()
{
	const char* simulation_type_name = "Galaxy";
	switch (Menu::simulation_type)
	{
	case SimulationType::Galaxy: simulation_type_name = "Galaxy"; break;
	case SimulationType::Collision: simulation_type_name = "Collision"; break;
	case SimulationType::Universe: simulation_type_name = "Universe"; break;
	}

	const char* matter_distribution_name = "CoreHalo";
	switch (Menu::matter_distribution)
	{
	case MatterDistribution::CoreHalo: matter_distribution_name = "CoreHalo"; break;
	case MatterDistribution::RandomMix: matter_distribution_name = "RandomMix"; break;
	case MatterDistribution::SplitX: matter_distribution_name = "SplitX"; break;
	}

	std::ostringstream out;
	out << std::fixed << std::setprecision(6);
	out
		<< "SIMCFG "
		<< "simulation_type=" << simulation_type_name << " "
		<< "step=" << Menu::step << " "
		<< "smoothing_length=" << Menu::smoothing_length << " "
		<< "interaction_rate=" << Menu::interaction_rate << " "
		<< "nb_stars=" << Menu::nb_stars << " "
		<< "galaxy_diameter=" << Menu::galaxy_diameter << " "
		<< "galaxy_thickness=" << Menu::galaxy_thickness << " "
		<< "galaxies_distance=" << Menu::galaxies_distance << " "
		<< "stars_speed=" << Menu::stars_speed << " "
		<< "positive_stars_speed=" << Menu::positive_stars_speed << " "
		<< "negative_stars_speed=" << Menu::negative_stars_speed << " "
		<< "black_hole_mass=" << Menu::black_hole_mass << " "
		<< "negative_attraction_constant=" << Menu::negative_attraction_constant << " "
		<< "repulsion_constant=" << Menu::repulsion_constant << " "
		<< "red_bloom_intensity=" << Menu::red_bloom_intensity << " "
		<< "blue_bloom_intensity=" << Menu::blue_bloom_intensity << " "
		<< "matter_distribution=" << matter_distribution_name << " "
		<< "type_diameter=" << Menu::type_diameter << " "
		<< "positive_ratio=" << Menu::positive_ratio << " "
		<< "core_extra_negative_density=" << Menu::core_extra_negative_density << " "
		<< "camera_view=" << (camera_view == CameraView::Top ? "top" : "isometric") << " "
		<< "batch_steps=2000 "
		<< "snapshots=4 "
		<< "output_dir=outputs";

	return out.str();
}

void Simulator::menu_update()
{
	step = Menu::step;
	smoothing_length = Menu::smoothing_length;
	interaction_rate = Menu::interaction_rate;
	config.step = step;
	config.smoothing_length = smoothing_length;
	config.interaction_rate = interaction_rate;
}

void Simulator::compute_update()
{
	if (!Menu::pause && (!computation_done || waiting) && !reload)
	{
		menu_update();
		Computer::compute(config, state);
		computation_done = true;
	}

	if (Menu::pause)
		computation_done = true;
}

void Simulator::render_update()
{
	waiting = true;

	if (0.01f - dim::Window::get_elapsed_time() > 0.f)
		sf::sleep(sf::seconds(0.01f - dim::Window::get_elapsed_time()));

	waiting = false;

	if (computation_done)
	{
		if (reload)
			restart();

		else
		{
			Renderer::update_vbo(state);
			computation_done = false;
		}
	}
}

void Simulator::check_events(const sf::Event& sf_event)
{
	Renderer::check_events(sf_event);
}

void Simulator::draw()
{
	Renderer::clear();
	Renderer::draw(state);
}

void Simulator::apply_camera_view()
{
	dim::Camera& camera = dim::Window::get_camera();
	const float radius = std::max(1.0f, camera.get_position().get_norm());
	dim::Vector3 center = dim::Vector3::null;
	dim::Controller& controller = dim::Window::get_controller();
	if (controller.get_type() == dim::Controller::Type::Orbit)
	{
		dim::OrbitController& orbit = static_cast<dim::OrbitController&>(controller);
		center = orbit.get_center();
	}

	if (camera_angle >= 0.0f)
	{
		dim::Vector3 position(0.f, 0.f, radius);
		position.set_phi((90.0f - camera_angle) * dim::pi / 180.0f);
		position += center;
		camera.set_position(position);
		camera.set_direction(center - position);
		return;
	}

	switch (camera_view)
	{
	case CameraView::Top:
	{
		const dim::Vector3 position = center + dim::Vector3(0.0f, radius, 0.001f);
		camera.set_position(position);
		camera.set_direction(center - position);
		break;
	}

	case CameraView::Front:
	{
		const dim::Vector3 position = center + dim::Vector3(radius, 0.001f, 0.0f);
		camera.set_position(position);
		camera.set_direction(center - position);
		break;
	}

	case CameraView::Isometric:
	default:
	{
		dim::Vector3 position(0.f, 0.f, radius);
		position.set_phi(dim::pi / 3.f);
		position += center;
		camera.set_position(position);
		camera.set_direction(center - position);
		break;
	}
	}
}
