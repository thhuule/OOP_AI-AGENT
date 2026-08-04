#pragma once

#include <string>
#include <vector>
#include <expected>
#include <system_error>

enum class EnvError {
    FileNotFound,
    AccessDenied,
    IOError,
    AlreadyExists
};

class Environment {
public:
    virtual ~Environment() = default;

    // Các thao tác file cơ bản cốt lõi cho Agent và Harness
    virtual std::expected<std::string, EnvError> readFile(const std::string& path) = 0;
    virtual std::expected<void, EnvError> writeFile(const std::string& path, const std::string& content) = 0;
    virtual std::expected<void, EnvError> removeFile(const std::string& path) = 0;
    virtual bool exists(const std::string& path) const = 0;
    
    // Quản lý artifact chung cho batch evaluation
    virtual std::expected<void, EnvError> cleanArtifacts(const std::vector<std::string>& paths) = 0;
};