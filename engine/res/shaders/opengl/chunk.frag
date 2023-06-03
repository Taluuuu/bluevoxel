#version 460 core

in vec2 pass_uv;
flat in uint pass_tex_index;
flat in uint pass_face_index;

layout(location = 2) uniform sampler2DArray block_textures;

out vec4 frag_albedo;

void main()
{
    frag_albedo = texture(block_textures, vec3(pass_uv, pass_tex_index));
}