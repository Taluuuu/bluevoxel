#version 460 core

#define VERTEX_POS_DETAIL 8 // Max possible value for each block coordinate
#define TEX_COORD_DETAIL 16 // Max possible value for each texture coord
#define CHUNK_SIZE 32       // Number of blocks in a chunk on each axis

layout(location = 0) in uint vertex_pos;
layout(location = 1) in uint vertex_data;

layout(location = 0) uniform mat4 view_projection_mat;
layout(location = 1) uniform ivec3 chunk_pos;

out vec2 pass_uv;
flat out uint pass_tex_index;

void main()
{
    // Vertex position
    vec3 world_pos = ivec3(
        bitfieldExtract(vertex_pos, 0,  10),
        bitfieldExtract(vertex_pos, 10, 10),
        bitfieldExtract(vertex_pos, 20, 10)
    ) / 8.0f + chunk_pos * CHUNK_SIZE;
    gl_Position = view_projection_mat * vec4(world_pos, 1.0f);

    // Texture uv
    pass_uv = vec2(
        bitfieldExtract(vertex_data, 0, 5),
        bitfieldExtract(vertex_data, 5, 5)
    ) / float(TEX_COORD_DETAIL);

    pass_tex_index  = bitfieldExtract(vertex_data, 10, 11);
}