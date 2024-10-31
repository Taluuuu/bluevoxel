#version 460 core

const uint MAX_BLOCK_COORD_VALUE = 16; // Max possible value for each block coordinate
const uint MAX_TEXTURE_COORD_VALUE = 16; // Max possible value for each texture coord
const uint MAX_PITCH_VALUE = 8;
const uint MAX_YAW_VALUE = 16;
const uint CHUNK_SIZE = 32;       // Number of blocks in a chunk on each axis

const float PI = 3.14159265359f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = PI / 2.0f;

layout(location = 0) in uint vertex_pos;
layout(location = 1) in uint vertex_data_0;
layout(location = 2) in uint vertex_data_1;

layout(location = 0) uniform mat4 view_projection_mat;
layout(location = 1) uniform ivec3 chunk_pos;

out vec2 pass_uv;
out vec3 pass_normal;
out float pass_light_level;
flat out uint pass_tex_index;

void main()
{
    // Vertex position
    const vec3 world_pos = ivec3(
        bitfieldExtract(vertex_pos, 0,  10),
        bitfieldExtract(vertex_pos, 10, 10),
        bitfieldExtract(vertex_pos, 20, 10)
    ) / float(MAX_BLOCK_COORD_VALUE) + chunk_pos * float(CHUNK_SIZE);
    gl_Position = view_projection_mat * vec4(world_pos, 1.0f);

    // Texture uv
    pass_uv = vec2(
        bitfieldExtract(vertex_data_0, 0, 5),
        bitfieldExtract(vertex_data_0, 5, 5)
    ) / float(MAX_TEXTURE_COORD_VALUE);

    pass_tex_index = bitfieldExtract(vertex_data_0, 10, 11);

    // Calculate face normal
    const uint packed_pitch = bitfieldExtract(vertex_data_1, 0, 4);
    const uint packed_yaw = bitfieldExtract(vertex_data_1, 4, 5);

    const float pitch = (packed_pitch / float(MAX_PITCH_VALUE)) * PI - HALF_PI;
    const float yaw = (packed_yaw / float(MAX_YAW_VALUE)) * TWO_PI - PI;

    const float cos_pitch = cos(pitch);
    const float sin_pitch = sin(pitch);
    const float cos_yaw = cos(yaw);
    const float sin_yaw = sin(yaw);

    pass_normal = vec3(
        cos_pitch * sin_yaw,
        sin_pitch,
        cos_pitch * cos_yaw
    );

    pass_light_level = float(bitfieldExtract(vertex_data_1, 9, 4)) / 15.0f;
}