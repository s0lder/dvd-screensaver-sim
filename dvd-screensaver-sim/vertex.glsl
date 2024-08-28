#version 330 core
layout (location = 0) in vec3 a_pos;

uniform vec2 velocity;

void main()
{
    gl_Position = vec4(a_pos.x + velocity.x, a_pos.y + velocity.y, a_pos.z, 1.0);
}