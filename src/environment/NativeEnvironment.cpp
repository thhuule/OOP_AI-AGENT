#include "NativeEnvironment.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

std::expected<std::string, EnvError> NativeEnvironment::readFile(const std::string& path) {
    if (!fs::exists(path)) {
        return std::unexpected(EnvError::FileNotFound);
    }
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected(EnvError::AccessDenied);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::expected<void, EnvError> NativeEnvironment::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected(EnvError::AccessDenied);
    }
    file << content;
    if (!file) {
        return std::unexpected(EnvError::IOError);
    }
    return {};
}

std::expected<void, EnvError> NativeEnvironment::removeFile(const std::string& path) {
    std::error_code ec;
    const bool exists = fs::exists(path, ec);
    if (ec)
        return std::unexpected(EnvError::IOError);
    if (!exists)
        return {};

    fs::remove(path, ec);
    if (ec)
        return std::unexpected(EnvError::IOError);
    return {};
}

bool NativeEnvironment::exists(const std::string& path) const {
    return fs::exists(path);
}

std::expected<void, EnvError> NativeEnvironment::cleanArtifacts(const std::vector<std::string>& paths) {
    for (const auto& p : paths) {
        const auto removed = removeFile(p);
        if (!removed)
            return std::unexpected(removed.error());
    }
    return {};
}
