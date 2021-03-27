#ifndef BLIMP_INCLUDE_GUARD_UUID_HPP
#define BLIMP_INCLUDE_GUARD_UUID_HPP

#include <blimp_plugin_sdk.h>

#include <gbBase/Assert.hpp>

#include <charconv>
#include <string>
#include <string_view>

inline std::string to_string(BlimpUUID const& uuid)
{
    std::string ret;
    ret.resize(36);
    // {8E287E92-9A1A-4916-829F-0D3AB185A125}
    char* out = ret.data();
    // {XXXXXXXX-0000-0000-0000-000000000000}
    std::to_chars(&(out[0]), &(out[8]), uuid.d1, 16);
    out[8] = '-';
    // {00000000-XXXX-0000-0000-000000000000}
    std::to_chars(&(out[9]), &(out[13]), uuid.d2, 16);
    out[13] = '-';
    // {00000000-0000-XXXX-0000-000000000000}
    std::to_chars(&(out[14]), &(out[18]), uuid.d3, 16);
    out[18] = '-';
    // {00000000-0000-0000-XX00-000000000000}
    std::to_chars(&(out[19]), &(out[21]), uuid.d4[0], 16);
    // {00000000-0000-0000-00XX-000000000000}
    std::to_chars(&(out[21]), &(out[23]), uuid.d4[1], 16);
    out[23] = '-';
    // {00000000-0000-0000-0000-XX0000000000}
    std::to_chars(&(out[24]), &(out[26]), uuid.d4[2], 16);
    // {00000000-0000-0000-0000-00XX00000000}
    std::to_chars(&(out[26]), &(out[28]), uuid.d4[3], 16);
    // {00000000-0000-0000-0000-0000XX000000}
    std::to_chars(&(out[28]), &(out[30]), uuid.d4[4], 16);
    // {00000000-0000-0000-0000-000000XX0000}
    std::to_chars(&(out[30]), &(out[32]), uuid.d4[5], 16);
    // {00000000-0000-0000-0000-00000000XX00}
    std::to_chars(&(out[32]), &(out[34]), uuid.d4[6], 16);
    // {00000000-0000-0000-0000-0000000000XX}
    std::to_chars(&(out[34]), &(out[36]), uuid.d4[7], 16);
    return ret;
}

inline BlimpUUID uuid_from_string(std::string_view str)
{
    GHULBUS_PRECONDITION(str.size() >= 36);
    GHULBUS_PRECONDITION(str[8] == '-');
    GHULBUS_PRECONDITION(str[13] == '-');
    GHULBUS_PRECONDITION(str[18] == '-');
    GHULBUS_PRECONDITION(str[23] == '-');
    BlimpUUID ret{};
    std::from_chars(&(str[0]), &(str[8]), ret.d1, 16);
    std::from_chars(&(str[9]), &(str[13]), ret.d2, 16);
    std::from_chars(&(str[14]), &(str[18]), ret.d3, 16);
    std::from_chars(&(str[19]), &(str[21]), ret.d4[0], 16);
    std::from_chars(&(str[21]), &(str[23]), ret.d4[1], 16);
    std::from_chars(&(str[24]), &(str[26]), ret.d4[2], 16);
    std::from_chars(&(str[26]), &(str[28]), ret.d4[3], 16);
    std::from_chars(&(str[28]), &(str[30]), ret.d4[4], 16);
    std::from_chars(&(str[30]), &(str[32]), ret.d4[5], 16);
    std::from_chars(&(str[32]), &(str[34]), ret.d4[6], 16);
    std::from_chars(&(str[34]), &(str[36]), ret.d4[7], 16);
    return ret;
}

#endif
