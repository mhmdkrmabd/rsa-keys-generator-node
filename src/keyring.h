#pragma once

#include <string>
#include <optional>

namespace KeysGen {

class Keyring {
public:
    static std::optional<std::string> getPassword(const std::string& service, const std::string& account);
    static bool setPassword(const std::string& service, const std::string& account, const std::string& password);
    static bool deletePassword(const std::string& service, const std::string& account);
    static bool isAvailable();

private:
#ifdef _WIN32
    static bool setPasswordWindows(const std::string& service, const std::string& account, const std::string& password);
    static std::optional<std::string> getPasswordWindows(const std::string& service, const std::string& account);
    static bool deletePasswordWindows(const std::string& service, const std::string& account);
#endif
#ifdef __linux__
    static bool setPasswordLinux(const std::string& service, const std::string& account, const std::string& password);
    static std::optional<std::string> getPasswordLinux(const std::string& service, const std::string& account);
    static bool deletePasswordLinux(const std::string& service, const std::string& account);
#endif
#ifdef __APPLE__
    static bool setPasswordMacOS(const std::string& service, const std::string& account, const std::string& password);
    static std::optional<std::string> getPasswordMacOS(const std::string& service, const std::string& account);
    static bool deletePasswordMacOS(const std::string& service, const std::string& account);
#endif
};

} // namespace KeysGen