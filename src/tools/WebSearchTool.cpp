
#include "WebSearchTool.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <sstream>

namespace oop_agent
{

using json = nlohmann::json;

std::string_view WebSearchTool::get_name() const noexcept
{
    return "web_search";
}

std::string_view WebSearchTool::get_description() const noexcept
{
    return R"(Search the web using DuckDuckGo Instant Answer API.

Input:
A search query.

Example:
C++ programming

Output:
A short summary related to the query.)";
}

size_t WebSearchTool::write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp)
{
    size_t total_size = size * nmemb;

    static_cast<std::string*>(userp)->append(
        static_cast<char*>(contents),
        total_size);

    return total_size;
}

std::expected<std::string, ToolError>
WebSearchTool::http_get(const std::string& url)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    return response;
}

std::expected<std::string, ToolError>
WebSearchTool::parse_response(
    const std::string& response)
{
    try
    {
        json j = json::parse(response);

        if (j.contains("AbstractText") &&
            !j["AbstractText"].get<std::string>().empty())
        {
            return j["AbstractText"].get<std::string>();
        }

        if (j.contains("Answer") &&
            !j["Answer"].get<std::string>().empty())
        {
            return j["Answer"].get<std::string>();
        }

        if (j.contains("RelatedTopics") &&
            !j["RelatedTopics"].empty())
        {
            auto& first = j["RelatedTopics"][0];

            if (first.contains("Text"))
            {
                return first["Text"].get<std::string>();
            }

            if (first.contains("Topics") &&
                !first["Topics"].empty() &&
                first["Topics"][0].contains("Text"))
            {
                return first["Topics"][0]["Text"]
                    .get<std::string>();
            }
        }

        return std::unexpected(
            ToolError::NotFound);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

std::expected<std::string, ToolError>
WebSearchTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        CURL* curl = curl_easy_init();

        if (!curl)
        {
            return std::unexpected(
                ToolError::ExecutionFailed);
        }

        char* encoded =
            curl_easy_escape(
                curl,
                arguments.c_str(),
                static_cast<int>(arguments.size()));

        if (!encoded)
        {
            curl_easy_cleanup(curl);

            return std::unexpected(
                ToolError::ExecutionFailed);
        }

        std::ostringstream url;

        url
            << "https://api.duckduckgo.com/?q="
            << encoded
            << "&format=json"
            << "&no_html=1";

        curl_free(encoded);

        curl_easy_cleanup(curl);

        auto response = http_get(url.str());

        if (!response.has_value())
        {
            return std::unexpected(
                response.error());
        }

        return parse_response(response.value());
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent