#ifndef BLIMP_INCLUDE_GUARD_STORAGE_LOCATION_HPP
#define BLIMP_INCLUDE_GUARD_STORAGE_LOCATION_HPP

#include <cstdint>
#include <string>

struct StorageLocation {
    std::string location;
    std::int64_t offset;
    std::int64_t size;
    std::int64_t part_number;
};

#endif
