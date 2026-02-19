#include "Menu.hpp"
#include "Simulator.hpp"
#include <dim/controllers/OrbitController.hpp>
#include <filesystem>
#include <fstream>
#include <exception>
#include <limits>
#include <cmath>

bool			Menu::visible			= true;
bool			Menu::active			= false;
bool			Menu::pause				= false;
SimulationType	Menu::simulation_type	= SimulationType::Galaxy;
float			Menu::step;
float			Menu::smoothing_length;
float			Menu::interaction_rate;
int				Menu::nb_stars;
float			Menu::galaxy_diameter;
float			Menu::galaxy_thickness;
float			Menu::galaxies_distance;
float			Menu::stars_speed;
float			Menu::black_hole_mass;
float			Menu::negative_attraction_constant = 1.0f;
float			Menu::repulsion_constant = 1.0f;
float			Menu::red_bloom_intensity = 1.0f;
float			Menu::blue_bloom_intensity = 1.0f;
float			Menu::type_diameter = 50.0f;
MatterDistribution	Menu::matter_distribution = MatterDistribution::CoreHalo;
float			Menu::positive_ratio = 0.5f;
float			Menu::core_extra_negative_density = 0.0f;
float			Menu::camera_pan_speed = 16.0f;
bool			Menu::measurement_enabled = false;
Menu::MeasurementAxis Menu::measurement_axis = Menu::MeasurementAxis::X;
float			Menu::measurement_marker_a = -50.0f;
float			Menu::measurement_marker_b = 50.0f;
float			Menu::measurement_axis_min = -100.0f;
float			Menu::measurement_axis_max = 100.0f;
dim::Vector3		Menu::measurement_center = dim::Vector3::null;
float			Menu::measurement_tick_size = 5.0f;
bool			Menu::measurement_auto_bounds = true;
float			Menu::measurement_value = 100.0f;

static bool refresh_measurement_bounds(bool preserve_marker_positions)
{
	const SimulationState& state = Simulator::state;
	if (state.positions.empty())
		return false;

	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float min_z = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();
	float max_z = std::numeric_limits<float>::lowest();

	for (const dim::Vector4& p : state.positions)
	{
		min_x = std::min(min_x, p.x);
		min_y = std::min(min_y, p.y);
		min_z = std::min(min_z, p.z);
		max_x = std::max(max_x, p.x);
		max_y = std::max(max_y, p.y);
		max_z = std::max(max_z, p.z);
	}

	Menu::measurement_center = dim::Vector3(
		0.5f * (min_x + max_x),
		0.5f * (min_y + max_y),
		0.5f * (min_z + max_z));

	const float span_x = std::max(0.001f, max_x - min_x);
	const float span_y = std::max(0.001f, max_y - min_y);
	const float span_z = std::max(0.001f, max_z - min_z);
	const float diagonal = std::sqrt(span_x * span_x + span_y * span_y + span_z * span_z);
	Menu::measurement_tick_size = std::max(0.5f, diagonal * 0.02f);

	const float previous_min = Menu::measurement_axis_min;
	const float previous_max = Menu::measurement_axis_max;
	float new_min = min_x;
	float new_max = max_x;

	switch (Menu::measurement_axis)
	{
	case Menu::MeasurementAxis::Y:
		new_min = min_y;
		new_max = max_y;
		break;
	case Menu::MeasurementAxis::Z:
		new_min = min_z;
		new_max = max_z;
		break;
	case Menu::MeasurementAxis::X:
	default:
		new_min = min_x;
		new_max = max_x;
		break;
	}

	Menu::measurement_axis_min = new_min;
	Menu::measurement_axis_max = new_max;

	if (!preserve_marker_positions || previous_max <= previous_min)
	{
		Menu::measurement_marker_a = new_min;
		Menu::measurement_marker_b = new_max;
	}
	else
	{
		Menu::measurement_marker_a = std::clamp(Menu::measurement_marker_a, new_min, new_max);
		Menu::measurement_marker_b = std::clamp(Menu::measurement_marker_b, new_min, new_max);
	}

	Menu::measurement_value = std::fabs(Menu::measurement_marker_b - Menu::measurement_marker_a);
	return true;
}


void Menu::check_events(const sf::Event& sf_event)
{
	if (sf_event.type == sf::Event::KeyReleased && sf_event.key.code == sf::Keyboard::Key::F1)
		visible = !visible;
}

void Menu::title(const std::string& text)
{
	float window_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.data()).x;

	ImGui::NewLine();
	ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
	ImGui::Text(text.data());
	ImGui::Separator();
}

std::vector<bool> Menu::centered_buttons(const std::vector<std::string> texts, float buttons_height, float spaces_size)
{
	float window_width = ImGui::GetWindowSize().x;
	float buttons_widths = (window_width - ((texts.size() + 2) * spaces_size)) / texts.size();
	std::vector<bool> result(texts.size());

	ImGui::NewLine();
	ImGui::NewLine();

	for (int i = 0; i < texts.size(); i++)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(spaces_size + buttons_widths * i + spaces_size * i);
		ImGui::Text("");
		ImGui::SameLine();
		result[i] = ImGui::Button(texts[i].data(), ImVec2(buttons_widths, buttons_height));
	}

	return result;
}

void Menu::set_default_values()
{
	step				= 0.001f;
	smoothing_length	= 1.f;
	galaxies_distance	= 75.f;
	black_hole_mass		= 1000.f;
	negative_attraction_constant = 1.0f;
	repulsion_constant = 1.0f;
	red_bloom_intensity = 1.0f;
	blue_bloom_intensity = 1.0f;
	type_diameter = 50.0f;
	matter_distribution = MatterDistribution::CoreHalo;
	positive_ratio = 0.5f;
	core_extra_negative_density = 0.0f;
	camera_pan_speed = 16.0f;


	switch (simulation_type)
	{
	case SimulationType::Galaxy:
		interaction_rate	= 0.05f;
		nb_stars			= 20000;
		galaxy_diameter		= 100.f;
		galaxy_thickness	= 5.f;
		stars_speed			= 20.f;
		break;

	case SimulationType::Collision:
		interaction_rate	= 0.05f;
		nb_stars			= 20000;
		galaxy_diameter		= 50.f;
		galaxy_thickness	= 2.5f;
		stars_speed			= 20.f;
		break;

	case SimulationType::Universe:
		interaction_rate	= 0.005f;
		nb_stars			= 100000;
		galaxy_diameter		= 10.f;
		galaxy_thickness	= 10.f;
		stars_speed			= 185.f;
		break;

	default:
		break;
	}
}

void Menu::galaxy()
{
	ImGui::Text("The number of stars");
	ImGui::SliderInt("##nb_stars", &nb_stars, 1000, 1000000, NULL, ImGuiSliderFlags_Logarithmic);

	ImGui::NewLine();

	ImGui::Text("The diameter of the galaxy");
	ImGui::SliderFloat("##galaxy_diameter", &galaxy_diameter, 10.f, 1000.f, "%.0f");

	ImGui::NewLine();

	ImGui::Text("The thickness of the galaxy");
	ImGui::SliderFloat("##galaxy_thickness", &galaxy_thickness, 1.f, 100.f, "%.0f");

	ImGui::NewLine();

	ImGui::Text("The initial speed of the stars");
	ImGui::SliderFloat("##stars_speed", &stars_speed, 0.f, 500.f, "%.1f");

	ImGui::NewLine();

	ImGui::Text("The mass of the black hole");
	ImGui::SliderFloat("##black_hole_mass", &black_hole_mass, 100.f, 1000000.f, "%.0f", ImGuiSliderFlags_Logarithmic);
}

void Menu::collision()
{
	ImGui::Text("The number of stars");
	ImGui::SliderInt("##nb_stars", &nb_stars, 1000, 1000000, NULL, ImGuiSliderFlags_Logarithmic);

	ImGui::NewLine();

	ImGui::Text("The diameter of the galaxies");
	ImGui::SliderFloat("##galaxy_diameter", &galaxy_diameter, 10.f, 1000.f, "%.0f");

	ImGui::NewLine();

	ImGui::Text("The thickness of the galaxies");
	ImGui::SliderFloat("##galaxy_thickness", &galaxy_thickness, 1.f, 100.f, "%.0f");

	ImGui::NewLine();

	ImGui::Text("The distance between the galaxies");
	ImGui::SliderFloat("##galaxies_distance", &galaxies_distance, 10.f, 1000.f, "%.0f");

	ImGui::NewLine();

	ImGui::Text("The initial speed of the stars");
	ImGui::SliderFloat("##stars_speed", &stars_speed, 0.f, 500.f, "%.1f");
}

void Menu::universe()
{
	ImGui::Text("The number of galaxies");
	ImGui::SliderInt("##nb_stars", &nb_stars, 1000, 1000000, NULL, ImGuiSliderFlags_Logarithmic);

	ImGui::NewLine();

	ImGui::Text("Ihe initial diameter of the universe");
	ImGui::SliderFloat("##galaxy_diameter", &galaxy_diameter, 1.f, 100.f, "%.1f");

	ImGui::NewLine();

	ImGui::Text("The initial speed of the galaxies");
	ImGui::SliderFloat("##stars_speed", &stars_speed, 0.f, 500.f, "%.1f");
}

void Menu::display()
{
	static std::string pause_button = "Pause";
	static char config_output_path[256] = "batch_configs/user.batchcfg";
	static std::string config_output_status;
	static MeasurementAxis previous_measurement_axis = measurement_axis;
	static int measurement_refresh_counter = 0;

	if (visible)
	{
		ImGui::Begin("Simulation settings (F1 to hide)");
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.9f);

		ImGui::NewLine();

		SimulationType temp = simulation_type;

		ImGui::Text("The type of simulation");
		ImGui::Combo("##simulation_type", reinterpret_cast<int*>(&simulation_type), "Galaxy\0Collision\0Universe\0");

		if (simulation_type != temp)
		{
			set_default_values();
			Simulator::reload = true;
		}
		ImGui::NewLine();

		Simulator::CameraView previous_camera_view = Simulator::camera_view;
		ImGui::Text("Camera view");
		ImGui::Combo("##camera_view", reinterpret_cast<int*>(&Simulator::camera_view), "Isometric\0Top\0");
		if (Simulator::camera_view != previous_camera_view)
			Simulator::apply_camera_view();
		ImGui::Text("Quick view");
		if (ImGui::Button("Top"))
		{
			Simulator::camera_view = Simulator::CameraView::Top;
			Simulator::apply_camera_view();
		}
		ImGui::SameLine();
		if (ImGui::Button("Front"))
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
			const dim::Vector3 position = center + dim::Vector3(0.0f, 0.0f, radius);
			camera.set_position(position);
			camera.set_direction(center - position);
		}
		ImGui::SameLine();
		if (ImGui::Button("Iso"))
		{
			Simulator::camera_view = Simulator::CameraView::Isometric;
			Simulator::apply_camera_view();
		}
		if (ImGui::Button("Reset orbit center"))
		{
			dim::Controller& controller = dim::Window::get_controller();
			if (controller.get_type() == dim::Controller::Type::Orbit)
			{
				dim::OrbitController& orbit = static_cast<dim::OrbitController&>(controller);
				orbit.set_center(dim::Vector3::null);
			}
		}
		ImGui::Text("Pan speed");
		ImGui::SliderFloat("##camera_pan_speed", &camera_pan_speed, 0.1f, 20.0f, "%.2f");
		{
			dim::Controller& controller = dim::Window::get_controller();
			if (controller.get_type() == dim::Controller::Type::Orbit)
			{
				dim::OrbitController& orbit = static_cast<dim::OrbitController&>(controller);
				orbit.set_pan_multiplier(camera_pan_speed);
			}
		}
		ImGui::NewLine();

		title("Measurement Tool");

		ImGui::TextWrapped("Measure distances in simulation units (same units as positions in .bin state files).");
		ImGui::Checkbox("Show ruler in scene", &measurement_enabled);
		ImGui::Checkbox("Auto-fit ruler bounds to current state", &measurement_auto_bounds);
		ImGui::TextDisabled("Auto-fit keeps axis min/max synced with particle extents.");

		if (ImGui::Button("Refresh bounds now"))
		{
			refresh_measurement_bounds(true);
			measurement_refresh_counter = 30;
		}

		if (measurement_auto_bounds && measurement_refresh_counter <= 0)
		{
			refresh_measurement_bounds(true);
			measurement_refresh_counter = 30;
		}
		else
			measurement_refresh_counter = std::max(0, measurement_refresh_counter - 1);

		ImGui::Text("Axis");
		ImGui::Combo("##measurement_axis", reinterpret_cast<int*>(&measurement_axis), "X\0Y\0Z\0");
		if (measurement_axis != previous_measurement_axis)
		{
			refresh_measurement_bounds(false);
			previous_measurement_axis = measurement_axis;
		}

		if (ImGui::Button("Set markers to axis min/max"))
		{
			measurement_marker_a = measurement_axis_min;
			measurement_marker_b = measurement_axis_max;
		}

		if (measurement_axis_max <= measurement_axis_min)
			measurement_axis_max = measurement_axis_min + 0.001f;

		ImGui::Text("Marker A position");
		ImGui::SliderFloat("##measurement_marker_a", &measurement_marker_a, measurement_axis_min, measurement_axis_max, "%.3f");
		ImGui::Text("Marker B position");
		ImGui::SliderFloat("##measurement_marker_b", &measurement_marker_b, measurement_axis_min, measurement_axis_max, "%.3f");
		measurement_value = std::fabs(measurement_marker_b - measurement_marker_a);
		ImGui::Text("Distance |A-B| = %.4f sim units", measurement_value);
		ImGui::Text("Axis bounds: [%.3f, %.3f]", measurement_axis_min, measurement_axis_max);

		ImGui::NewLine();

		// Janus force multipliers (real-time).
		ImGui::Text("Negative Attraction Constant");
		ImGui::SliderFloat("##Negative_Attraction_Constant", &negative_attraction_constant, -10.0f, 10.0f);
		ImGui::Text("Repulsion Constant");
		ImGui::SliderFloat("##Repulsion_Constant", &repulsion_constant, -10.0f, 10.0f);
		ImGui::Text("Red Star Bloom Intensity");
		ImGui::SliderFloat("##Red_Star_Bloom_Intensity", &red_bloom_intensity, 0.0f, 4.0f, "%.2f");
		ImGui::Text("Blue Star Bloom Intensity");
		ImGui::SliderFloat("##Blue_Star_Bloom_Intensity", &blue_bloom_intensity, 0.0f, 4.0f, "%.2f");
		ImGui::NewLine();

		title("Real time settings");

		ImGui::NewLine();

		ImGui::Text("The time step duration");
		ImGui::SliderFloat("##step", &step, 0.0001f, 0.1f, "%.4f", ImGuiSliderFlags_Logarithmic);

		ImGui::NewLine();

		ImGui::Text("The smoothing length");
		ImGui::SliderFloat("##smoothing_length", &smoothing_length, 0.001f, 1.f, NULL, ImGuiSliderFlags_Logarithmic);

		ImGui::NewLine();

		ImGui::Text("The interaction rate");
		ImGui::SliderFloat("##interaction_rate", &interaction_rate, 0.001f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);

		ImGui::NewLine();

		title("Applies after restart");

		ImGui::NewLine();

		ImGui::Text("Matter distribution");
		ImGui::Combo("##matter_distribution", reinterpret_cast<int*>(&matter_distribution), "Core + halo\0Random mix\0Split on X\0\0");
		ImGui::NewLine();

		if (matter_distribution == MatterDistribution::CoreHalo)
		{
			ImGui::Text("Positive core diameter");
			ImGui::SliderFloat("##Type_Diameter", &type_diameter, 0.0f, galaxy_diameter);
			ImGui::NewLine();

			ImGui::Text("Extra negative density inside core");
			ImGui::SliderFloat("##Core_Extra_Negative_Density", &core_extra_negative_density, 0.0f, 1.0f, "%.2f");
			ImGui::TextDisabled("0 = core fully positive, 1 = core fully negative");
			ImGui::NewLine();
		}
		else if (matter_distribution == MatterDistribution::RandomMix)
		{
			ImGui::Text("Positive matter ratio");
			ImGui::SliderFloat("##Positive_Ratio", &positive_ratio, 0.0f, 1.0f, "%.2f");
			ImGui::NewLine();
		}

		switch (simulation_type)
		{
		case SimulationType::Galaxy: galaxy(); break;
		case SimulationType::Collision: collision(); break;
		case SimulationType::Universe: universe(); break;
		default: break;
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text("Batch config export");
		ImGui::InputText("##config_output_path", config_output_path, sizeof(config_output_path));
		if (ImGui::Button("Append current SIMCFG to file"))
		{
			try
			{
				const std::filesystem::path output_path(config_output_path);
				if (output_path.has_parent_path())
					std::filesystem::create_directories(output_path.parent_path());

				std::ofstream out(config_output_path, std::ios::app);
				if (out)
				{
					out << Simulator::configuration_line() << '\n';
					config_output_status = std::string("Appended to ") + config_output_path;
				}
				else
					config_output_status = std::string("Failed to open ") + config_output_path;
			}
			catch (const std::exception& e)
			{
				config_output_status = std::string("Export error: ") + e.what();
			}
		}
		if (!config_output_status.empty())
			ImGui::TextWrapped("%s", config_output_status.c_str());

		ImGui::NewLine();

		std::vector<bool> buttons = centered_buttons({ "Restart", pause_button }, 25.f, 20.f);

		if (buttons[1])
			pause = !pause;

		pause_button = (pause ? "Play" : "Pause");

		if (buttons[0])
			Simulator::reload = true;

		active = ImGui::IsWindowFocused();

		ImGui::End();
	}
}
