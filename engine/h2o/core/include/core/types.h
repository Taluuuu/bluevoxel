#pragma once

#include <cinttypes>
#include <glm/glm.hpp> // TODO: Smaller includes as this file is included everywhere
#include <glm/gtc/quaternion.hpp>
#include <string_view>
#include <array>

using v2   = glm::vec2;
using v2i  = glm::ivec2;
using v2u  = glm::uvec2;
using v3   = glm::vec3;
using v3i  = glm::ivec3;
using v4   = glm::vec4;
using quat = glm::quat;

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

//class Name
//{
//private:
//
//    using DataType = u8;
//    static constexpr std::string_view table = "?abcdefghijklmnopqrstuvwxyz_-.";
//    static constexpr size_t data_array_bytes = 16;
//    static constexpr size_t data_array_len = data_array_bytes / sizeof(DataType);
//
//    using Data = std::array<DataType, data_array_len>;
//
//    consteval explicit Name(const Data& data)
//        : m_data(data) {}
//
//public:
//
//    constexpr Name() = delete;
//
//    template<size_t N>
//    static consteval Name create(const char (&str)[N])
//    {
//        size_t bits_per_char = ceil(log2l(table.size()));
//        size_t max_str_len   = (data_array_bytes * 8) / bits_per_char;
//
//        static_assert(N <= max_str_len, "Name string lengths must be smaller or equal to max_str_len.");
//
//        std::array<DataType, data_array_bytes / sizeof(DataType)> result{0};
//        //for (size_t i = 0; i < N - 1 /* ignore null terminator */; i++)
//        //{
//        //    auto idx = table.find(str[i]);
//        //    static_assert(idx != std::string_view::npos, "Missing character in Name table.");
//        //
//        //
//        //
//        //    result.data() (idx << (i * bits_per_char))
//        //}
//
//        return Name(result);
//    }
//
//    constexpr bool operator==(const Name& other) const
//    {
//        return m_data == other.m_data;
//    }
//
//    constexpr bool operator<(const Name& other) const
//    {
//        return m_data < other.m_data;
//    }
//
//private:
//
//    Data m_data;
//
//};