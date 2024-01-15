#version 460 core

layout(location = 0) in vec3 vertex_pos;

layout(location = 0) uniform mat4 proj_view;
layout(location = 1) uniform mat4 model;

void main()
{
    gl_Position = proj_view * model * vec4(vertex_pos, 1.0f);
}