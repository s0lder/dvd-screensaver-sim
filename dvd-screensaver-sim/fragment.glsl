#version 330 core

out vec4 frag_color;

in vec2 tex_coords;
uniform vec3 u_color;

uniform sampler2D texture1;

void main()
{
    frag_color = texture(texture1, tex_coords) * vec4(u_color, 1.0);
}