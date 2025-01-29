#include <dim/dimension3D.hpp>
#include "Simulator.hpp"
#include <GL/glew.h>
#include <iostream>


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

int main()
{
	dim::Window::open("Galaxy simulation", 0.75f, "resources/icons/icon.png");
	Simulator::init();

	// Print OpenGL context information
    print_opengl_info();

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

		dim::Window::get_controller().enable(!Menu::active || !Menu::visible, dim::Controller::Action::Look);

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
