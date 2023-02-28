#version 460 core

in vec4 pass_frag_color;

out vec4 out_frag_color;

void main()
{
    out_frag_color = pass_frag_color;
}