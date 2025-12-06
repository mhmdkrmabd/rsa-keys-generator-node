#include <napi.h>
#include "platform_utils.h"
#include "keyring.h"

using namespace KeysGen;

// Get a password from the keychain
Napi::Value GetPassword(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    try {
        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
            Napi::TypeError::New(env, "getPassword(service, account) requires two string parameters")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        if (!Keyring::isAvailable()) {
            return env.Null();
        }

        std::string service = info[0].As<Napi::String>().Utf8Value();
        std::string account = info[1].As<Napi::String>().Utf8Value();

        auto password = Keyring::getPassword(service, account);
        if (password.has_value()) {
            return Napi::String::New(env, password.value());
        }
    } catch (...) {
        // Silent failure
    }

    return env.Null();
}

// Set a password in the keychain
Napi::Value SetPassword(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    try {
        if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
            Napi::TypeError::New(env, "setPassword(service, account, password) requires three string parameters")
                .ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }

        if (!Keyring::isAvailable()) {
            return Napi::Boolean::New(env, false);
        }

        std::string service = info[0].As<Napi::String>().Utf8Value();
        std::string account = info[1].As<Napi::String>().Utf8Value();
        std::string password = info[2].As<Napi::String>().Utf8Value();

        bool success = Keyring::setPassword(service, account, password);
        return Napi::Boolean::New(env, success);
    } catch (...) {
        return Napi::Boolean::New(env, false);
    }
}

// Delete a password from the keychain
Napi::Value DeletePassword(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    try {
        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
            Napi::TypeError::New(env, "deletePassword(service, account) requires two string parameters")
                .ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }

        if (!Keyring::isAvailable()) {
            return Napi::Boolean::New(env, false);
        }

        std::string service = info[0].As<Napi::String>().Utf8Value();
        std::string account = info[1].As<Napi::String>().Utf8Value();

        bool success = Keyring::deletePassword(service, account);
        return Napi::Boolean::New(env, success);
    } catch (...) {
        return Napi::Boolean::New(env, false);
    }
}

// Check if keyring is available
Napi::Value IsKeychainAvailable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, Keyring::isAvailable());
}

// Get platform information
Napi::Value GetPlatform(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, PlatformUtils::getPlatformString());
}

// Initialize the module
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "getPassword"),
                Napi::Function::New(env, GetPassword));
    exports.Set(Napi::String::New(env, "setPassword"),
                Napi::Function::New(env, SetPassword));
    exports.Set(Napi::String::New(env, "deletePassword"),
                Napi::Function::New(env, DeletePassword));
    exports.Set(Napi::String::New(env, "isKeychainAvailable"),
                Napi::Function::New(env, IsKeychainAvailable));
    exports.Set(Napi::String::New(env, "getPlatform"),
                Napi::Function::New(env, GetPlatform));

    return exports;
}

NODE_API_MODULE(keys_generator, Init)
