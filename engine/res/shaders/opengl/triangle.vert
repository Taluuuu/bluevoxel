#version 460 core

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec4 frag_color;

layout(location = 0) uniform mat4 proj_view;

out vec4 pass_frag_color;

void main()
{
    gl_Position = proj_view * vec4(vertex_pos, 1.0f);
    pass_frag_color = frag_color;
}