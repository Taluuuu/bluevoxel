#version 460 core

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) uniform mat4 proj_view;
layout(location = 1) uniform mat4 model;

out vec2 pass_tex_coord;

void main()
{
    gl_Position = proj_view * model * vec4(vertex_pos, 1.0f);
    pass_tex_coord = tex_coord;
}