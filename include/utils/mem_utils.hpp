#pragma once

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "bcrypt.lib")
#include <windows.h>
#include <bcrypt.h>
#include <dpapi.h>
// VirtualLock/Unlock are in windows.h (via memoryapi.h)
#elif defined(__linux__)
#include <sys/random.h> // For getrandom()
#include <sys/mman.h>   // For mlock() and munlock()
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <stdexcept>
#include <cstdint>
#include <cstring>

namespace CryptoHelper {
	// Memory helpers

    inline void secure_zero_memory(void* ptr, size_t len) {
        if (!ptr || len == 0) return;
#if defined(_WIN32) || defined(_WIN64)
        SecureZeroMemory(ptr, len); // Windows-native secure wipe
#elif defined(__linux__) || defined(__GLIBC__)
        explicit_bzero(ptr, len);   // Linux-native secure wipe
#else
        // Generic secure wipe
        volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
        while (len--) { *p++ = 0; }
#endif
    }


    inline bool lock_memory(void* ptr, size_t len) {
#if defined(_WIN32) || defined(_WIN64)
        return (VirtualLock(ptr, len) != 0); // Non-zero is success
#elif defined(__linux__)
        return (mlock(ptr, len) == 0);      // Zero is success
#else
        std::cerr << "Locking not supported" << std::endl;
        return false;
#endif
    }

    inline bool unlock_memory(void* ptr, size_t len) {
#if defined(_WIN32) || defined(_WIN64)
        return (VirtualUnlock(ptr, len) != 0); // Non-zero is success
#elif defined(__linux__)
        return (munlock(ptr, len) == 0);      // Zero is success
#else
        std::cerr << "Unlocking not supported" << std::endl;
        return false;
#endif
    }

	// Random byte generation

    inline void gen_secure_random_bytes(uint8_t* buffer, size_t length) {
#if defined(_WIN32) || defined(_WIN64)
        if (!BCRYPT_SUCCESS(BCryptGenRandom(NULL, buffer, static_cast<ULONG>(length), BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
            throw std::runtime_error("BCryptGenRandom failed");
        }
#elif defined(__linux__)
        size_t total_read = 0;
        while (total_read < length) {
            ssize_t result = getrandom(buffer + total_read, length - total_read, 0);

            if (result == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("getrandom failed: " + std::to_string(errno));
            }
            total_read += result;
        }
#else
        throw std::runtime_error("Platform not supported");
#endif
    }
}
