#include "ActionTool.h"

#include <sstream>

namespace oop_agent
{

std::string_view ActionTool::get_name() const noexcept
{
    return "gui_action";
}

std::string_view ActionTool::get_description() const noexcept
{
    return "Perform a bounded GUI action. "
           "Args: 'click <x> <y>', 'type_text <text>', or "
           "'key_press <key>' (allowed keys: return, tab, escape, space, "
           "up, down, left, right, backspace, delete). "
           "Coordinates are validated; no arbitrary shell/browser command.";
}

bool ActionTool::is_allowed_action(const std::string& action)
{
    return action == "click" ||
           action == "type_text" ||
           action == "key_press";
}

bool ActionTool::is_allowed_key(const std::string& key)
{
    static constexpr const char* kAllowed[] = {
        "return", "tab", "escape", "space",
        "up", "down", "left", "right",
        "backspace", "delete",
    };

    for (const char* allowed : kAllowed)
    {
        if (key == allowed)
        {
            return true;
        }
    }

    return false;
}

std::expected<std::string, ToolError>
ActionTool::perform_action(
    const std::string& action,
    const std::string& payload)
{
    (void)action;
    (void)payload;

    // Contract-only: action đã validate; thao tác desktop thật do C
    // triển khai trong demo có kiểm soát. Không chạy shell/browser ở đây.
    return std::unexpected(
        ToolError::NotFound);
}

std::expected<std::string, ToolError>
ActionTool::execute(const std::string& arguments)
{
    try
    {
        std::stringstream ss(arguments);

        std::string action;

        ss >> action;

        if (action.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        if (!is_allowed_action(action))
        {
            return std::unexpected(
                ToolError::AccessDenied);
        }

        if (action == "click")
        {
            long x = 0;
            long y = 0;

            ss >> x >> y;

            if (!ss ||
                x < 0 || x > kMaxCoordinate ||
                y < 0 || y > kMaxCoordinate)
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            std::ostringstream payload;
            payload << x << " " << y;

            return perform_action(action, payload.str());
        }

        if (action == "type_text")
        {
            std::string text;

            std::getline(ss, text);

            if (!text.empty() && text.front() == ' ')
            {
                text.erase(0, 1);
            }

            if (text.empty() ||
                text.size() > kMaxTextLength)
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            return perform_action(action, text);
        }

        if (action == "key_press")
        {
            std::string key;

            ss >> key;

            if (key.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            if (!is_allowed_key(key))
            {
                return std::unexpected(
                    ToolError::AccessDenied);
            }

            return perform_action(action, key);
        }

        return std::unexpected(
            ToolError::InvalidArgument);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent
