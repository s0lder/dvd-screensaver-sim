#version 330 core

out vec4 frag_color;

in vec2 tex_coords;
in vec3 vertex_color;
uniform vec3 u_color;

uniform sampler2D texture1;

void main()
{
    frag_color = (u_color != vec3(0.0, 0.0, 0.0)) ? (texture(texture1, tex_coords) * vec4(u_color, 1.0)) : (texture(texture1, tex_coords) * (vec4(vertex_color, 1.0)));
}