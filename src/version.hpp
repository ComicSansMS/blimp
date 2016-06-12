#ifndef BLIMP_INCLUDE_GUARD_VERSION_HPP
#define BLIMP_INCLUDE_GUARD_VERSION_HPP

struct BlimpVersion
{
    static inline constexpr int major() { return 1; }
    static inline constexpr int minor() { return 0; }
    static inline constexpr int patch() { return 0; }
    static inline constexpr int version() { return major() * 10000 + minor() * 100 + patch(); }
};

#endif
