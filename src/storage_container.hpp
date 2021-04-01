#ifndef BLIMP_INCLUDE_GUARD_STORAGE_CONTAINER_HPP
#define BLIMP_INCLUDE_GUARD_STORAGE_CONTAINER_HPP

#include <cstdint>
#include <string>

struct StorageContainerLocation {
    std::string l;
};

struct StorageContainerId {
    int64_t i;
};

struct StorageContainer {
    StorageContainerId id;
    StorageContainerLocation location;
};

#endif
