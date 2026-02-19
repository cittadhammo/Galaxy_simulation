#include "Renderer.hpp"
#include "Menu.hpp"

GLuint				Renderer::vbo = 0;
GLuint				Renderer::vao = 0;
dim::VertexBuffer	Renderer::blur_vbo;
dim::VertexBuffer	Renderer::post_vbo;
dim::FrameBuffer	Renderer::galaxy_fbo_1;
dim::FrameBuffer	Renderer::galaxy_fbo_2;
dim::FrameBuffer	Renderer::blur_fbo_1;
dim::FrameBuffer	Renderer::blur_fbo_2;
GLuint				Renderer::ruler_vbo = 0;
GLuint				Renderer::ruler_vao = 0;

static void append_line(std::vector<float>& vertices, const dim::Vector3& a, const dim::Vector3& b)
{
	vertices.push_back(a.x);
	vertices.push_back(a.y);
	vertices.push_back(a.z);
	vertices.push_back(b.x);
	vertices.push_back(b.y);
	vertices.push_back(b.z);
}

static void draw_measurement_ruler()
{
	if (!Menu::measurement_enabled || Menu::measurement_axis_max <= Menu::measurement_axis_min)
		return;

	const float axis_min = Menu::measurement_axis_min;
	const float axis_max = Menu::measurement_axis_max;
	const float marker_a = std::clamp(Menu::measurement_marker_a, axis_min, axis_max);
	const float marker_b = std::clamp(Menu::measurement_marker_b, axis_min, axis_max);
	const float tick = std::max(0.25f, Menu::measurement_tick_size);
	const dim::Vector3 c = Menu::measurement_center;

	dim::Vector3 axis_start = c;
	dim::Vector3 axis_end = c;
	dim::Vector3 a0 = c, a1 = c, b0 = c, b1 = c, a2 = c, a3 = c, b2 = c, b3 = c;

	switch (Menu::measurement_axis)
	{
	case Menu::MeasurementAxis::Y:
		axis_start.y = axis_min;
		axis_end.y = axis_max;
		a0.y = marker_a; a0.x -= tick; a1.y = marker_a; a1.x += tick;
		b0.y = marker_b; b0.x -= tick; b1.y = marker_b; b1.x += tick;
		a2.y = marker_a; a2.z -= tick; a3.y = marker_a; a3.z += tick;
		b2.y = marker_b; b2.z -= tick; b3.y = marker_b; b3.z += tick;
		break;
	case Menu::MeasurementAxis::Z:
		axis_start.z = axis_min;
		axis_end.z = axis_max;
		a0.z = marker_a; a0.x -= tick; a1.z = marker_a; a1.x += tick;
		b0.z = marker_b; b0.x -= tick; b1.z = marker_b; b1.x += tick;
		a2.z = marker_a; a2.y -= tick; a3.z = marker_a; a3.y += tick;
		b2.z = marker_b; b2.y -= tick; b3.z = marker_b; b3.y += tick;
		break;
	case Menu::MeasurementAxis::X:
	default:
		axis_start.x = axis_min;
		axis_end.x = axis_max;
		a0.x = marker_a; a0.y -= tick; a1.x = marker_a; a1.y += tick;
		b0.x = marker_b; b0.y -= tick; b1.x = marker_b; b1.y += tick;
		a2.x = marker_a; a2.z -= tick; a3.x = marker_a; a3.z += tick;
		b2.x = marker_b; b2.z -= tick; b3.x = marker_b; b3.z += tick;
		break;
	}

	std::vector<float> vertices;
	vertices.reserve(54);
	append_line(vertices, axis_start, axis_end);
	append_line(vertices, a0, a1);
	append_line(vertices, b0, b1);
	append_line(vertices, a2, a3);
	append_line(vertices, b2, b3);

	glBindVertexArray(Renderer::ruler_vao);
	glBindBuffer(GL_ARRAY_BUFFER, Renderer::ruler_vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_DYNAMIC_DRAW);

	dim::Shader::get("ruler").bind();
	dim::Shader::get("ruler").send_uniform("u_mvp", dim::Window::get_camera().get_matrix());
	dim::Shader::get("ruler").send_uniform("u_color", dim::Vector3(0.2f, 1.0f, 0.2f));

	const GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size() / 3));
	glLineWidth(1.0f);
	if (depth_test_was_enabled)
		glEnable(GL_DEPTH_TEST);

	dim::Shader::get("ruler").unbind();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::init_vbo(const SimulationState& state)
{
    // Delete buffers
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLsizeiptr positions_size = state.positions.size() * sizeof(dim::Vector4);
    GLsizeiptr speeds_size = state.speeds.size() * sizeof(dim::Vector4);
    GLsizeiptr types_size = state.star_types.size() * sizeof(int);

    // Allocate one packed buffer: positions, speeds, star types.
    glBufferData(GL_ARRAY_BUFFER, positions_size + speeds_size + types_size, NULL, GL_DYNAMIC_DRAW);

    // Update positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, state.positions.data());

    // Update speeds
    glBufferSubData(GL_ARRAY_BUFFER, positions_size, speeds_size, state.speeds.data());

    // Update star types
    glBufferSubData(GL_ARRAY_BUFFER, positions_size + speeds_size, types_size, state.star_types.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint positions = glGetAttribLocation(dim::Shader::get("galaxy").get_id(), "a_position");
    glVertexAttribPointer(positions, 4, GL_FLOAT, GL_FALSE, sizeof(dim::Vector4), reinterpret_cast<GLvoid*>(0));
    glEnableVertexAttribArray(positions);

    GLint speeds = glGetAttribLocation(dim::Shader::get("galaxy").get_id(), "a_speed");
    glVertexAttribPointer(speeds, 4, GL_FLOAT, GL_FALSE, sizeof(dim::Vector4), reinterpret_cast<GLvoid*>(positions_size));
    glEnableVertexAttribArray(speeds);

    GLint star_types = glGetAttribLocation(dim::Shader::get("galaxy").get_id(), "a_starType");
    glVertexAttribIPointer(star_types, 1, GL_INT, sizeof(int), reinterpret_cast<GLvoid*>(positions_size + speeds_size));
    glEnableVertexAttribArray(star_types);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::update_vbo(const SimulationState& state)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLsizeiptr positions_size = state.positions.size() * sizeof(dim::Vector4);
    GLsizeiptr speeds_size = state.speeds.size() * sizeof(dim::Vector4);

    // Update positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, state.positions.data());

    // Update speeds
    glBufferSubData(GL_ARRAY_BUFFER, positions_size, speeds_size, state.speeds.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::draw_vbo(const SimulationState& state)
{
	glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(state.positions.size()));
}

void Renderer::bind_vbo()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void Renderer::unbind_vbo()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::init(const SimulationState& state)
{
	init_vbo(state);

	blur_vbo.send_data("blur", dim::Mesh::screen, dim::DataType::Positions | dim::DataType::TexCoords);
	post_vbo.send_data("post", dim::Mesh::screen, dim::DataType::Positions | dim::DataType::TexCoords);
	galaxy_fbo_1.create(dim::Window::get_size(), dim::Texture::Filtering::Linear, dim::Texture::Warpping::MirroredRepeat, dim::Texture::Type::RGB_16f);
	galaxy_fbo_2.create(dim::Window::get_size(), dim::Texture::Filtering::Linear, dim::Texture::Warpping::MirroredRepeat, dim::Texture::Type::RGB_16f);
	blur_fbo_1.create(dim::Window::get_size(), dim::Texture::Filtering::Linear, dim::Texture::Warpping::MirroredRepeat, dim::Texture::Type::RGB_16f);
	blur_fbo_2.create(dim::Window::get_size(), dim::Texture::Filtering::Linear, dim::Texture::Warpping::MirroredRepeat, dim::Texture::Type::RGB_16f);

	if (ruler_vbo == 0)
		glGenBuffers(1, &ruler_vbo);
	if (ruler_vao == 0)
		glGenVertexArrays(1, &ruler_vao);

	glBindVertexArray(ruler_vao);
	glBindBuffer(GL_ARRAY_BUFFER, ruler_vbo);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::check_events(const sf::Event& sf_event)
{
	if (sf_event.type == sf::Event::Resized)
	{
		galaxy_fbo_1.set_size(dim::Window::get_size());
		galaxy_fbo_2.set_size(dim::Window::get_size());
		blur_fbo_1.set_size(dim::Window::get_size());
		blur_fbo_2.set_size(dim::Window::get_size());
	}
}

void Renderer::clear()
{
	galaxy_fbo_1.bind();
	galaxy_fbo_1.clear();
	galaxy_fbo_1.unbind();

	galaxy_fbo_2.bind();
	galaxy_fbo_2.clear();
	galaxy_fbo_2.unbind();

	blur_fbo_1.bind();
	blur_fbo_1.clear();
	blur_fbo_1.unbind();

	blur_fbo_2.bind();
	blur_fbo_2.clear();
	blur_fbo_2.unbind();
}

void Renderer::draw(const SimulationState& state)
{
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    dim::Window::set_thickness(1.f);

    // Draw the stars.
    galaxy_fbo_1.bind();
        dim::Shader::get("galaxy").bind();
            bind_vbo();

                dim::Shader::get("galaxy").send_uniform("u_mvp", dim::Window::get_camera().get_matrix());
                draw_vbo(state);

            unbind_vbo();
        dim::Shader::get("galaxy").unbind();
    galaxy_fbo_1.unbind();

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    dim::Window::set_thickness(2.f);

    // Draw the stars to be blurred.
    galaxy_fbo_2.bind();
        dim::Shader::get("galaxy").bind();
            bind_vbo();

                dim::Shader::get("galaxy").send_uniform("u_mvp", dim::Window::get_camera().get_matrix());
                draw_vbo(state);

            unbind_vbo();
        dim::Shader::get("galaxy").unbind();
    galaxy_fbo_2.unbind();

    // The first step of the blur.
    blur_fbo_1.bind();
        dim::Shader::get("blur").bind();
            galaxy_fbo_2.get_texture().bind();
                blur_vbo.bind();

                    dim::Shader::get("blur").send_uniform("u_texture", galaxy_fbo_2.get_texture());
                    dim::Shader::get("blur").send_uniform("u_horizontal", 1);
                    blur_vbo.draw();

                blur_vbo.unbind();
            galaxy_fbo_2.get_texture().unbind();
        dim::Shader::get("blur").unbind();
    blur_fbo_1.unbind();

    // The second step of the blur.
    blur_fbo_2.bind();
        dim::Shader::get("blur").bind();
            blur_fbo_1.get_texture().bind();
                blur_vbo.bind();

                    dim::Shader::get("blur").send_uniform("u_texture", blur_fbo_1.get_texture());
                    dim::Shader::get("blur").send_uniform("u_horizontal", 0);
                    blur_vbo.draw();

                blur_vbo.unbind();
            blur_fbo_1.get_texture().unbind();
        dim::Shader::get("blur").unbind();
    blur_fbo_2.unbind();

    // Merge all the steps and set the color.
    dim::Shader::get("post").bind();
        galaxy_fbo_1.get_texture().bind();
        blur_fbo_2.get_texture().bind();
            post_vbo.bind();

                // Render stars with star type information
                dim::Shader::get("post").send_uniform("u_galaxy", galaxy_fbo_1.get_texture());
                dim::Shader::get("post").send_uniform("u_blur", blur_fbo_2.get_texture());
                dim::Shader::get("post").send_uniform("u_red_bloom_intensity", Menu::red_bloom_intensity);
                dim::Shader::get("post").send_uniform("u_blue_bloom_intensity", Menu::blue_bloom_intensity);
                post_vbo.draw();

            post_vbo.unbind();
        blur_fbo_2.get_texture().unbind();
        galaxy_fbo_1.get_texture().unbind();
    dim::Shader::get("post").unbind();

	draw_measurement_ruler();

    glDisable(GL_BLEND);
}
