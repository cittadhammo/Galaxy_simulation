#version 430
precision mediump float;

in float v_brightness;
flat in int v_starType; // Add this line to receive star type

out vec4 frag_color;

void main()
{
    float brightness = v_brightness / 5.0;
    float brightness2 = v_brightness / 1.0;

    if (v_starType > 0)
    {
        frag_color = vec4(3., 0.0, 0.0, 1.0); // gives blue stars for positive stars
    }
    else
    {
        frag_color = vec4(1.0, 1.0, 0.0, 1.0); // gives brigther blue stars for positive stars
    }
}