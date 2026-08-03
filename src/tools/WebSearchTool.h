
#pragma once

#include "Tool.h"

#include <expected>
#include <string>
#include <string_view>

namespace oop_agent
{

class WebSearchTool : public Tool
{
public:
    WebSearchTool() = default;
    ~WebSearchTool() override = default;

    [[nodiscard]]
    std::string_view get_name() const noexcept override;

    [[nodiscard]]
    std::string_view get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

private:

    /**
     * @brief Callback của libcurl để ghi dữ liệu nhận được.
     */
    static size_t write_callback(
        void* contents,
        size_t size,
        size_t nmemb,
        void* userp);

    /**
     * @brief Thực hiện HTTP GET.
     */
    std::expected<std::string, ToolError>
    http_get(const std::string& url);

    /**
     * @brief Parse JSON trả về từ DuckDuckGo.
     */
    std::expected<std::string, ToolError>
    parse_response(const std::string& response);
};

} // namespace oop_agent