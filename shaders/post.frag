#version 430
precision mediump float;

in vec2 v_texcoord;

out vec4 frag_color;

uniform sampler2D u_galaxy;
uniform sampler2D u_blur;

void main()
{
    vec4 galaxy_data = texture(u_galaxy, v_texcoord);
    float stars = galaxy_data.r;
    float star_type = galaxy_data.g; // Assuming star type is stored in the green channel
    float glow = texture(u_blur, v_texcoord).r * 0.2;
    vec3 color_1;
    vec3 color_2;

    if (star_type > 0.5) // Positive stars
    {
        color_1 = vec3(0.3, 0.05, 0.05); // Dark red
        color_2 = vec3(0.3, 0.3, 0.05); // Yellow
        frag_color = vec4(0.7 * mix(color_1, color_2, 0.1 * (stars + glow)) * (stars + glow), 1.);
    }
    else // Negative stars
    {
        color_1 = vec3(0.05, 0.05, 0.3); // Dark blue
        color_2 = vec3(0.05, 0.3, 0.3); // Cyan
        frag_color = vec4(0.7 * mix(color_1, color_2, 0.05 * (stars + glow)) * (stars + glow), 1.);
    }
}