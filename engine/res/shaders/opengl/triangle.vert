#version 460 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec4 frag_color;

out vec4 out_frag_color;

void main()
{
    gl_Position = vec4(vertex_pos, 0.0f, 0.0f);
    out_frag_color = frag_color;
}