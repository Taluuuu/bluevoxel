#version 460 core

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec4 color;

layout(location = 0) uniform mat4 view_projection_mat;

out vec4 pass_color;

void main()
{
    gl_Position = view_projection_mat * vec4(vertex_pos, 1.0f);
    pass_color = color;
}