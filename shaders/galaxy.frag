#version 430
precision mediump float;

in float v_brightness;
flat in int v_starType; // Add this line to receive star type

out vec4 frag_color;

void main()
{
    float brightness = v_brightness / 5.0;

    if (v_starType > 0)
    {
        frag_color = vec4(0.0, 0.0, brightness, 1.0); // regular mass (positive): blue
    }
    else
    {
        frag_color = vec4(brightness, 0.0, 0.0, 1.0); // negative mass: red
    }
}
