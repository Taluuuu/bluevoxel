#version 460 core

in vec2 pass_tex_coord;

layout(location = 2) uniform sampler2D tex;

out vec4 out_frag_color;

void main()
{
    out_frag_color = texture(tex, pass_tex_coord);
}