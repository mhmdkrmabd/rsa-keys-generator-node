#include "platform_utils.h"
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/utsname.h>
#endif

namespace KeysGen {

Platform PlatformUtils::getPlatform() {
#ifdef _WIN32
    return Platform::Windows;
#elif defined(__linux__)
    return Platform::Linux;
#elif defined(__APPLE__)
    return Platform::macOS;
#else
    return Platform::Unknown;
#endif
}

std::string PlatformUtils::getPlatformString() {
    switch (getPlatform()) {
        case Platform::Windows:
            return "Windows";
        case Platform::Linux:
            return "Linux";
        case Platform::macOS:
            return "macOS";
        default:
            return "Unknown";
    }
}

int PlatformUtils::getRSAKeyLength() {
    const char* envVar = std::getenv("RSA_KEY_LENGTH");
    if (envVar != nullptr) {
        try {
            return std::stoi(envVar);
        } catch (...) {
            // Fall through to default
        }
    }

    // Default key length, matches Python version behavior
    return 2048;
}

} // namespace KeysGen