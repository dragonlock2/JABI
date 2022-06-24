#ifndef LIBJABI_BYTEORDER_H
#define LIBJABI_BYTEORDER_H

#include <array>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

template<typename T>
constexpr T htole(T x) { return x; }

template<typename T>
constexpr T letoh(T x) { return x; }

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

template<typename T>
constexpr T htole(T x) {
    auto arr = reinterpret_cast<std::array<std::byte, sizeof(T)>&>(x);
    std::reverse(arr.begin(), arr.end());
    return reinterpret_cast<T&>(arr);
}

template<typename T>
constexpr T letoh(T x) { return htole<T>(x); }

#else
#error "Unknown byte order"
#endif

#endif // LIBJABI_BYTEORDER_H
