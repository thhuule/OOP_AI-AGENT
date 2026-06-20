#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Tool;

class ToolRegistry {
public:
    void registerTool(std::shared_ptr<Tool> tool);

    std::shared_ptr<Tool> getTool(const std::string& name);

private:
    std::unordered_map<std::string,
                       std::shared_ptr<Tool>> tools_;
};
