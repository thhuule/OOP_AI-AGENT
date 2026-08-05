#pragma once
#include "Environment.h"
#include <unordered_map>

class SandboxEnvironment : public Environment {
private:
    std::unordered_map<std::string, std::string> virtual_files_;

public:
    std::expected<std::string, EnvError> readFile(const std::string& path) override {
        auto it = virtual_files_.find(path);
        if (it == virtual_files_.end()) {
            return std::unexpected(EnvError::FileNotFound);
        }
        return it->second;
    }

    std::expected<void, EnvError> writeFile(const std::string& path, const std::string& content) override {
        virtual_files_[path] = content;
        return {};
    }

    std::expected<void, EnvError> removeFile(const std::string& path) override {
        if (virtual_files_.erase(path) == 0) {
            return std::unexpected(EnvError::FileNotFound);
        }
        return {};
    }

    bool exists(const std::string& path) const override {
        return virtual_files_.find(path) != virtual_files_.end();
    }

    std::expected<void, EnvError> cleanArtifacts(const std::vector<std::string>& paths) override {
        for (const auto& p : paths) {
            virtual_files_.erase(p);
        }
        return {};
    }
};