#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec2 a_tex_coords;

uniform vec2 velocity;
out vec2 tex_coords;
out vec3 vertex_color;

void main()
{
    gl_Position = vec4(a_pos.x + velocity.x, a_pos.y + velocity.y, a_pos.z, 1.0);
    tex_coords = a_tex_coords;
    vertex_color = a_color;
}