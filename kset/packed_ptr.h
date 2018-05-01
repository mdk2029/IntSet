#pragma once

#include <cstdint>
#include "errors.h"

// In x86_64, top 16 bits of a 64 bit pointer are not used and can
// be used to store other data.
namespace Kset
{

class PackedPtr {
    std::uint64_t packedWord_{0};
public:
    template<class T>
    T* getPtr() const {
        return reinterpret_cast<T *>(packedWord_ & std::uint64_t(0x0000FFFFFFFFFFFF));
    }

    template<class T>
    void setPtr(T* ptr) {
        ASSERT(!((reinterpret_cast<uint64_t>(ptr)) >> 48));
        packedWord_ &= uint64_t(0xFFFF000000000000);
        packedWord_ |= reinterpret_cast<uint64_t>(ptr);
    }

    std::uint16_t getData() const {
        return packedWord_ >> 48;
    }

    void setData(std::uint16_t val) {
        packedWord_ &= uint64_t(0x0000FFFFFFFFFFFF);
        packedWord_ |= (uint64_t(val) << 48);
    }

    std::uint64_t packedWord() const {
        return packedWord_;
    }
};

static_assert(sizeof(PackedPtr) == sizeof(void *), "unexpected sizeof(PackedPtr)");
}
