#pragma once

#include <cinttypes>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp> // TODO: Smaller includes as this file is included everywhere
#include <glm/gtc/quaternion.hpp>
#include <string_view>
#include <array>

using v2   = glm::vec2;
using v2i  = glm::ivec2;
using v2u  = glm::uvec2;
using v2b  = glm::bvec2;

using v3   = glm::vec3;
using v3i  = glm::ivec3;
using v3u  = glm::uvec3;
using v3b  = glm::bvec3;

using v4   = glm::vec4;
using quat = glm::quat;

using m3i = glm::imat3x3;

using m2 = glm::mat2;
using m3 = glm::mat3;
using m4 = glm::mat4;

using f32 = glm::f32;
using f64 = glm::f64;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

namespace bitsery
{
    template<typename S>
    void serialize(S& s, v2i& o)
    { s(o.x, o.y); }

    template<typename S>
    void serialize(S& s, v3i& o)
    { s(o.x, o.y, o.z); }

    template<typename S>
    void serialize(S& s, v3& o)
    { s(o.x, o.y, o.z); }
}