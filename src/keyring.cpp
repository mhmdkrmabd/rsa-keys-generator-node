#include "keyring.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <wincred.h>
#endif

#if defined(__linux__) && defined(HAVE_LIBSECRET)
#include <libsecret/secret.h>
#endif

#ifdef __APPLE__
#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace KeysGen {

bool Keyring::isAvailable() {
#ifdef _WIN32
    return true;
#elif defined(__linux__) && defined(HAVE_LIBSECRET)
    return true;
#elif defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

std::optional<std::string> Keyring::getPassword(const std::string& service, const std::string& account) {
#ifdef _WIN32
    return getPasswordWindows(service, account);
#elif defined(__linux__) && defined(HAVE_LIBSECRET)
    return getPasswordLinux(service, account);
#elif defined(__APPLE__)
    return getPasswordMacOS(service, account);
#else
    return std::nullopt;
#endif
}

bool Keyring::setPassword(const std::string& service, const std::string& account, const std::string& password) {
#ifdef _WIN32
    return setPasswordWindows(service, account, password);
#elif defined(__linux__) && defined(HAVE_LIBSECRET)
    return setPasswordLinux(service, account, password);
#elif defined(__APPLE__)
    return setPasswordMacOS(service, account, password);
#else
    return false;
#endif
}

bool Keyring::deletePassword(const std::string& service, const std::string& account) {
#ifdef _WIN32
    return deletePasswordWindows(service, account);
#elif defined(__linux__) && defined(HAVE_LIBSECRET)
    return deletePasswordLinux(service, account);
#elif defined(__APPLE__)
    return deletePasswordMacOS(service, account);
#else
    return false;
#endif
}

#ifdef _WIN32
std::optional<std::string> Keyring::getPasswordWindows(const std::string& service, const std::string& account) {
    PCREDENTIALW credential = nullptr;

    int wideTargetSize = MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, nullptr, 0);
    std::wstring wideTarget(wideTargetSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, &wideTarget[0], wideTargetSize);

    if (CredReadW(wideTarget.c_str(), CRED_TYPE_GENERIC, 0, &credential)) {
        std::string password(reinterpret_cast<char*>(credential->CredentialBlob), credential->CredentialBlobSize);
        CredFree(credential);
        return password;
    }

    return std::nullopt;
}

bool Keyring::setPasswordWindows(const std::string& service, const std::string& account, const std::string& password) {
    int wideTargetSize = MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, nullptr, 0);
    std::wstring wideTarget(wideTargetSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, &wideTarget[0], wideTargetSize);

    int wideUsernameSize = MultiByteToWideChar(CP_UTF8, 0, account.c_str(), -1, nullptr, 0);
    std::wstring wideUsername(wideUsernameSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, account.c_str(), -1, &wideUsername[0], wideUsernameSize);

    CREDENTIALW credential = {};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<LPWSTR>(wideTarget.c_str());
    credential.UserName = const_cast<LPWSTR>(wideUsername.c_str());
    credential.CredentialBlobSize = static_cast<DWORD>(password.size());
    credential.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<char*>(password.c_str()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;

    return CredWriteW(&credential, 0) != 0;
}

bool Keyring::deletePasswordWindows(const std::string& service, const std::string& account) {
    int wideTargetSize = MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, nullptr, 0);
    std::wstring wideTarget(wideTargetSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, service.c_str(), -1, &wideTarget[0], wideTargetSize);

    return CredDeleteW(wideTarget.c_str(), CRED_TYPE_GENERIC, 0) != 0;
}
#endif

#ifdef __linux__
#ifdef HAVE_LIBSECRET

// Define schema that matches Python's keyring library
// Python uses a simple password schema, not network schema
static const SecretSchema* get_keyring_schema(void) {
    static const SecretSchema schema = {
        "org.freedesktop.Secret.Generic",
        SECRET_SCHEMA_NONE,
        {
            { "service", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { "username", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { "NULL", (SecretSchemaAttributeType)0 },
        },
        0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
    };
    return &schema;
}

std::optional<std::string> Keyring::getPasswordLinux(const std::string& service, const std::string& account) {
    GError* error = nullptr;

    // Try the Python-compatible schema first (Generic)
    gchar* password = secret_password_lookup_sync(
        get_keyring_schema(),
        nullptr,
        &error,
        "service", service.c_str(),
        "username", account.c_str(),
        nullptr
    );

    if (error) {
        g_error_free(error);
        error = nullptr;

        // Fallback: try network schema for backwards compatibility
        password = secret_password_lookup_sync(
            SECRET_SCHEMA_COMPAT_NETWORK,
            nullptr,
            &error,
            "server", service.c_str(),
            "user", account.c_str(),
            nullptr
        );

        if (error) {
            g_error_free(error);
            return std::nullopt;
        }
    }

    if (password) {
        std::string result(password);
        secret_password_free(password);
        return result;
    }

    return std::nullopt;
}

bool Keyring::setPasswordLinux(const std::string& service, const std::string& account, const std::string& password) {
    GError* error = nullptr;
    std::string label = "Password for '" + account + "' on '" + service + "'";

    // Use Python-compatible schema (Generic)
    bool success = secret_password_store_sync(
        get_keyring_schema(),
        SECRET_COLLECTION_DEFAULT,
        label.c_str(),
        password.c_str(),
        nullptr,
        &error,
        "service", service.c_str(),
        "username", account.c_str(),
        nullptr
    );

    if (error) {
        g_error_free(error);
        return false;
    }

    return success;
}

bool Keyring::deletePasswordLinux(const std::string& service, const std::string& account) {
    GError* error = nullptr;

    bool success = secret_password_clear_sync(
        get_keyring_schema(),
        nullptr,
        &error,
        "service", service.c_str(),
        "username", account.c_str(),
        nullptr
    );

    if (error) {
        g_error_free(error);
        return false;
    }

    return success;
}
#else
std::optional<std::string> Keyring::getPasswordLinux(const std::string& service, const std::string& account) {
    return std::nullopt;
}

bool Keyring::setPasswordLinux(const std::string& service, const std::string& account, const std::string& password) {
    return false;
}

bool Keyring::deletePasswordLinux(const std::string& service, const std::string& account) {
    return false;
}
#endif
#endif

#ifdef __APPLE__
std::optional<std::string> Keyring::getPasswordMacOS(const std::string& service, const std::string& account) {
    void* passwordData = nullptr;
    UInt32 passwordLength = 0;

    OSStatus status = SecKeychainFindGenericPassword(
        nullptr,                                    // default keychain
        static_cast<UInt32>(service.length()),     // service name length
        service.c_str(),                           // service name
        static_cast<UInt32>(account.length()),     // account name length
        account.c_str(),                           // account name
        &passwordLength,                            // password length
        &passwordData,                             // password data
        nullptr                                     // item reference (not needed)
    );

    if (status == errSecSuccess && passwordData) {
        std::string password(static_cast<char*>(passwordData), passwordLength);
        SecKeychainItemFreeContent(nullptr, passwordData);
        return password;
    }

    return std::nullopt;
}

bool Keyring::setPasswordMacOS(const std::string& service, const std::string& account, const std::string& password) {
    // First, try to find existing item
    SecKeychainItemRef itemRef = nullptr;

    OSStatus findStatus = SecKeychainFindGenericPassword(
        nullptr,                                    // default keychain
        static_cast<UInt32>(service.length()),     // service name length
        service.c_str(),                           // service name
        static_cast<UInt32>(account.length()),     // account name length
        account.c_str(),                           // account name
        nullptr,                                    // password length (not needed)
        nullptr,                                    // password data (not needed)
        &itemRef                                    // item reference
    );

    if (findStatus == errSecSuccess && itemRef) {
        // Item exists, update it
        OSStatus updateStatus = SecKeychainItemModifyAttributesAndData(
            itemRef,                                // item reference
            nullptr,                                // attributes (not changing)
            static_cast<UInt32>(password.length()), // password length
            password.c_str()                        // password data
        );

        CFRelease(itemRef);
        return updateStatus == errSecSuccess;
    }

    // Item doesn't exist, create new one
    OSStatus addStatus = SecKeychainAddGenericPassword(
        nullptr,                                    // default keychain
        static_cast<UInt32>(service.length()),     // service name length
        service.c_str(),                           // service name
        static_cast<UInt32>(account.length()),     // account name length
        account.c_str(),                           // account name
        static_cast<UInt32>(password.length()),    // password length
        password.c_str(),                          // password data
        nullptr                                     // item reference (not needed)
    );

    return addStatus == errSecSuccess;
}

bool Keyring::deletePasswordMacOS(const std::string& service, const std::string& account) {
    SecKeychainItemRef itemRef = nullptr;

    OSStatus findStatus = SecKeychainFindGenericPassword(
        nullptr,                                    // default keychain
        static_cast<UInt32>(service.length()),     // service name length
        service.c_str(),                           // service name
        static_cast<UInt32>(account.length()),     // account name length
        account.c_str(),                           // account name
        nullptr,                                    // password length (not needed)
        nullptr,                                    // password data (not needed)
        &itemRef                                    // item reference
    );

    if (findStatus == errSecSuccess && itemRef) {
        OSStatus deleteStatus = SecKeychainItemDelete(itemRef);
        CFRelease(itemRef);
        return deleteStatus == errSecSuccess;
    }

    return false;
}
#endif

} // namespace KeysGen