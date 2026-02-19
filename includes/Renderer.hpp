#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "SimulationData.hpp"
#include "libraries.hpp"

/**
 * @brief Takes care of the simulation display.
 */
class Renderer
{
public:

	static GLuint				vbo;			// The OpenGL vertex buffer object id.
	static GLuint				vao;			// The OpenGL vertex array object id.
	static dim::VertexBuffer	blur_vbo;		// The VBO of the blur shader.
	static dim::VertexBuffer	post_vbo;		// The VBO of the post processing shader.
	static dim::FrameBuffer		galaxy_fbo_1;	// The FBO of the stars.
	static dim::FrameBuffer		galaxy_fbo_2;	// The FBO of the stars to be blured.
	static dim::FrameBuffer		blur_fbo_1;		// The FBO of the first step of the blur.
	static dim::FrameBuffer		blur_fbo_2;		// The FBO of the second step of the blur.
	static GLuint				ruler_vbo;		// The VBO of the measurement ruler.
	static GLuint				ruler_vao;		// The VAO of the measurement ruler.

	/**
	 * @brief Create the OpenGL vertex buffer object.
	 */
	static void init_vbo(const SimulationState& state);

	/**
	 * @brief Update the vertices.
	 */
	static void update_vbo(const SimulationState& state);

	/**
	 * @brief Draw the VBO.
	 */
	static void draw_vbo(const SimulationState& state);

	/**
	 * @brief Bind the VBO.
	 */
	static void bind_vbo();

	/**
	 * @brief Unbind the VBO.
	 */
	static void unbind_vbo();

	/**
	 * @brief Initialize the renderer.
	 */
	static void init(const SimulationState& state);

	/**
	 * @brief Check the events of the renderer.
	 *
	 * @param sf_event the SFML event object
	 */
	static void check_events(const sf::Event& sf_event);

	/**
	 * @brief Clear the FBOs.
	 */
	static void clear();

	/**
	 * @brief Draw the simulation.
	 */
	static void draw(const SimulationState& state);
};

#endif
