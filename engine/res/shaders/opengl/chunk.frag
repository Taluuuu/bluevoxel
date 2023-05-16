#version 460 core

in vec2 pass_uv;
flat in uint pass_tex_index;
flat in uint pass_face_index;

layout(location = 3) uniform sampler2DArray block_texture_array;
layout(location = 4) uniform sampler2D light_depth_tex;

layout(location = 0) out vec4 frag_albedo;

void main()
{
//    vec4 tex_color = texture(block_texture_array, vec3(pass_uv, pass_tex_index));
//
//    // Discard fragment if it is invisible
//    if (tex_color.a < 0.01f)
//        discard;
//
//    frag_albedo = tex_color;

    frag_albedo = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}