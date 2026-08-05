#pragma once
#include "Environment.h"
#include <filesystem>

class NativeEnvironment : public Environment {
public:
    std::expected<std::string, EnvError> readFile(const std::string& path) override;
    std::expected<void, EnvError> writeFile(const std::string& path, const std::string& content) override;
    std::expected<void, EnvError> removeFile(const std::string& path) override;
    bool exists(const std::string& path) const override;
    std::expected<void, EnvError> cleanArtifacts(const std::vector<std::string>& paths) override;
};