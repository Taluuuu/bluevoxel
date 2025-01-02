#version 460 core

in vec2 pass_uv;
in vec3 pass_normal;
in float pass_light_level;
in float pass_sunlight_level;
flat in uint pass_tex_index;

layout(location = 2) uniform sampler2DArray block_textures;
layout(location = 3) uniform vec3 light_dir;
layout(location = 4) uniform vec3 light_color;
layout(location = 5) uniform float ambient_strength;
layout(location = 6) uniform float sun_brightness;

out vec4 frag_color;

void main()
{
    const vec4 albedo = texture(block_textures, vec3(pass_uv, pass_tex_index));
    if (albedo.a < 0.1f)
        discard;

    const vec3 ambient = light_color * ambient_strength;

    const vec3 diffuse = light_color * max(dot(pass_normal, -light_dir), 0.0f);

    frag_color = vec4(clamp(ambient + diffuse, 0.0f, 1.0f), 0.0f) * albedo * max(pass_light_level, pass_sunlight_level * sun_brightness);
}