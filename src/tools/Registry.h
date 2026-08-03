#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
class Registry {
public:
    void register_item(const std::string& name,
                       std::unique_ptr<T> item)
    {
        items_[name] = std::move(item);
    }

    T* get(const std::string& name) const
    {
        auto it = items_.find(name);
        return it != items_.end() ? it->second.get() : nullptr;
    }

    std::vector<std::string> list() const
    {
        std::vector<std::string> names;
        for (const auto& [k, v] : items_)
            names.push_back(k);
        return names;
    }

private:
    std::unordered_map<std::string,
        std::unique_ptr<T>> items_;
};