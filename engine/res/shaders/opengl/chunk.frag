#version 460 core

in vec2 pass_uv;
in vec3 pass_normal;
flat in uint pass_tex_index;

layout(location = 2) uniform sampler2DArray block_textures;

out vec4 frag_albedo;

void main()
{
    frag_albedo = vec4(0.5f * (pass_normal + vec3(1.0f, 1.0f, 1.0f)), 1.0f);
//    frag_albedo = texture(block_textures, vec3(pass_uv, pass_tex_index));
}