#ifndef BLIMP_INCLUDE_GUARD_STORAGE_HPP
#define BLIMP_INCLUDE_GUARD_STORAGE_HPP

class Storage {
public:
    struct Location {

    };

    Storage() = default;
    Storage(Storage const&) = delete;
    Storage& operator=(Storage const&) = delete;

    virtual ~Storage() = default;


    virtual Location storeFile() = 0;
};

#endif
