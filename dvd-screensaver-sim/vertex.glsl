#version 330 core
layout (position = 0) in vec3 a_pos;
layout (position = 1) in vec3 a_color;

out vec3 vertex_color;

void main()
{
    vertex_color = a_color;
    gl_Position = vec4(a_pos, 1.0);
}