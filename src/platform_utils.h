#pragma once

#include <string>

namespace KeysGen {

enum class Platform {
    Windows,
    Linux,
    macOS,
    Unknown
};

class PlatformUtils {
public:
    static Platform getPlatform();
    static std::string getPlatformString();
    static int getRSAKeyLength();
};

} // namespace KeysGen