
#include "WebSearchTool.h"
#include <iostream>
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
Args: the search query string.
Example: C++ programming
Output: a short summary related to the query.)";
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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) OOP_AI_Agent/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Giới hạn thời gian chờ để tránh treo vô hạn; classify timeout riêng
    // (không throw, trả ToolError ổn định).
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10'000L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5'000L);

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cerr << "[ERROR] libcurl: "
                << curl_easy_strerror(result)
                << " (code: " << result << ")\n";
    }

    curl_easy_cleanup(curl);

    if (result == CURLE_OPERATION_TIMEDOUT ||
        result == CURLE_COULDNT_CONNECT ||
        result == CURLE_COULDNT_RESOLVE_HOST)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

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

        if (j.contains("query") && j["query"].contains("search") &&
            !j["query"]["search"].empty())
        {
            std::string result_text;
            for (const auto& item : j["query"]["search"])
            {
                if (item.contains("snippet"))
                {
                    std::string snippet = item["snippet"].get<std::string>();
                    std::string clean_snippet;
                    bool in_tag = false;
                    for (char c : snippet)
                    {
                        if (c == '<') in_tag = true;
                        else if (c == '>') in_tag = false;
                        else if (!in_tag) clean_snippet += c;
                    }
                    if (!clean_snippet.empty())
                    {
                        if (!result_text.empty()) result_text += "\n---\n";
                        if (item.contains("title"))
                        {
                            result_text += item["title"].get<std::string>() + ": ";
                        }
                        result_text += clean_snippet;
                    }
                }
            }
            if (!result_text.empty())
            {
                return result_text;
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

        std::string encoded_str(encoded);
        curl_free(encoded);
        curl_easy_cleanup(curl);

        std::ostringstream url;
        url << "https://api.duckduckgo.com/?q="
            << encoded_str
            << "&format=json"
            << "&no_html=1";

        std::cerr << "[DEBUG] URL = " << url.str() << '\n';
        auto response = http_get(url.str());

        if (response.has_value())
        {
            auto parsed = parse_response(response.value());
            if (parsed.has_value())
            {
                return parsed;
            }
        }
        else if (response.error() != ToolError::NotFound)
        {
            return std::unexpected(response.error());
        }

        // Fallback 1: Wikipedia API tiếng Việt
        std::ostringstream wiki_url;
        wiki_url << "https://vi.wikipedia.org/w/api.php?action=query&list=search&srsearch="
                 << encoded_str
                 << "&format=json";
        std::cerr << "[DEBUG] Fallback Wiki URL = " << wiki_url.str() << '\n';
        auto wiki_resp = http_get(wiki_url.str());
        if (wiki_resp.has_value())
        {
            auto parsed_wiki = parse_response(wiki_resp.value());
            if (parsed_wiki.has_value())
            {
                return parsed_wiki;
            }
        }

        // Fallback 2: Wikipedia API tiếng Anh
        std::ostringstream wiki_en_url;
        wiki_en_url << "https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch="
                    << encoded_str
                    << "&format=json";
        std::cerr << "[DEBUG] Fallback Wiki EN URL = " << wiki_en_url.str() << '\n';
        auto wiki_en_resp = http_get(wiki_en_url.str());
        if (wiki_en_resp.has_value())
        {
            auto parsed_wiki_en = parse_response(wiki_en_resp.value());
            if (parsed_wiki_en.has_value())
            {
                return parsed_wiki_en;
            }
        }

        return std::unexpected(ToolError::NotFound);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent