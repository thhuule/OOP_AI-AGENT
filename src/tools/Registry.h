#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
class Registry {
public:
    bool register_item(
        const std::string& name,
        std::shared_ptr<T> item)
    {
        if (contains(name))
            return false;

        items_[name] = std::move(item);
        return true;
    }

    T* get(const std::string& name) const
    {
        auto it = items_.find(name);
        return it != items_.end() ? it->second.get() : nullptr;
    }

    bool contains(const std::string& name) const
    {
        return items_.find(name) != items_.end();
    }

    std::vector<std::string> list() const
    {
        std::vector<std::string> names;
        for (const auto& [k, v] : items_)
            names.push_back(k);
        return names;
    }

private:
    std::unordered_map<
        std::string,
        std::shared_ptr<T>
    > items_;
};