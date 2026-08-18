#include "Embedding.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace oop_agent
{

float cosine_similarity(
    const EmbeddingVector& lhs,
    const EmbeddingVector& rhs)
{
    if (lhs.empty() || rhs.empty() || lhs.size() != rhs.size())
    {
        return 0.0f;
    }

    double dot = 0.0;
    double norm_l = 0.0;
    double norm_r = 0.0;

    for (std::size_t i = 0; i < lhs.size(); ++i)
    {
        dot += static_cast<double>(lhs[i]) * static_cast<double>(rhs[i]);
        norm_l += static_cast<double>(lhs[i]) * static_cast<double>(lhs[i]);
        norm_r += static_cast<double>(rhs[i]) * static_cast<double>(rhs[i]);
    }

    if (norm_l <= 0.0 || norm_r <= 0.0)
    {
        return 0.0f;
    }

    return static_cast<float>(dot / (std::sqrt(norm_l) * std::sqrt(norm_r)));
}

HashEmbedder::HashEmbedder(std::size_t dim)
    : dim_(dim > 0 ? dim : kEmbeddingDim)
{
}

EmbeddingVector HashEmbedder::embed(const std::string& text) const
{
    EmbeddingVector vec(dim_, 0.0f);

    if (text.empty())
    {
        return vec;
    }

    const std::size_t gram = 3;

    // Character n-gram hashing: mỗi n-gram (lowercase) cộng 1 vào bin hash.
    const std::string normalized = [&text]() {
        std::string out;
        out.reserve(text.size());

        for (unsigned char c : text)
        {
            if (c >= 'A' && c <= 'Z')
            {
                out.push_back(static_cast<char>(c + ('a' - 'A')));
            }
            else if (std::isalnum(c))
            {
                out.push_back(static_cast<char>(c));
            }
            else
            {
                out.push_back(' ');
            }
        }

        return out;
    }();

    std::string window;
    window.reserve(gram);

    for (std::size_t i = 0; i < normalized.size(); ++i)
    {
        window.push_back(normalized[i]);

        if (window.size() > gram)
        {
            window.erase(window.begin());
        }

        if (window.size() == gram)
        {
            std::uint64_t h = 1469598103934665603ULL; // FNV-1a 64-bit
            for (unsigned char c : window)
            {
                h ^= static_cast<std::uint64_t>(c);
                h *= 1099511628211ULL;
            }

            vec[h % dim_] += 1.0f;
        }
    }

    // L2 normalize → cosine similarity = dot product.
    double norm = 0.0;
    for (float v : vec)
    {
        norm += static_cast<double>(v) * static_cast<double>(v);
    }

    if (norm > 0.0)
    {
        const float inv = static_cast<float>(1.0 / std::sqrt(norm));
        std::for_each(vec.begin(), vec.end(),
                      [inv](float& v) { v *= inv; });
    }

    return vec;
}

OllamaEmbedder::OllamaEmbedder(
    std::string host,
    std::string model,
    int timeout_seconds)
    : host_(std::move(host))
    , model_(std::move(model))
    , timeout_seconds_(timeout_seconds > 0 ? timeout_seconds : 10)
{
    if (host_.empty())
    {
        host_ = "http://localhost:11434";
    }
    if (model_.empty())
    {
        model_ = "nomic-embed-text";
    }
}

std::size_t OllamaEmbedder::write_callback(
    void* contents,
    std::size_t size,
    std::size_t nmemb,
    void* userp)
{
    const std::size_t total = size * nmemb;
    static_cast<std::string*>(userp)->append(
        static_cast<const char*>(contents), total);
    return total;
}

EmbeddingVector OllamaEmbedder::embed(const std::string& text) const
{
    if (text.empty())
    {
        throw std::runtime_error("OllamaEmbedder: empty input text");
    }

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("OllamaEmbedder: Failed to initialize cURL");
    }

    std::string endpoint = host_;
    while (!endpoint.empty() && endpoint.back() == '/')
    {
        endpoint.pop_back();
    }

    if (endpoint.ends_with("/api/chat"))
    {
        endpoint = endpoint.substr(0, endpoint.length() - 9) + "/api/embed";
    }
    else if (!endpoint.ends_with("/api/embed") && !endpoint.ends_with("/api/embeddings"))
    {
        endpoint += "/api/embed";
    }

    nlohmann::json req_json;
    req_json["model"] = model_;
    if (endpoint.ends_with("/api/embeddings"))
    {
        req_json["prompt"] = text;
    }
    else
    {
        req_json["input"] = text;
    }

    const std::string req_str = req_json.dump();
    std::string read_buffer;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OllamaEmbedder::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout_seconds_));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, static_cast<long>(std::max(1, timeout_seconds_ / 2)));

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        throw std::runtime_error("OllamaEmbedder network error: " + std::string(curl_easy_strerror(res)));
    }

    if (http_code != 200)
    {
        throw std::runtime_error("OllamaEmbedder HTTP error " + std::to_string(http_code));
    }

    try
    {
        auto res_json = nlohmann::json::parse(read_buffer);
        if (res_json.contains("embeddings") && res_json["embeddings"].is_array() && !res_json["embeddings"].empty())
        {
            auto vec = res_json["embeddings"][0].get<EmbeddingVector>();
            if (vec.empty())
            {
                throw std::runtime_error("OllamaEmbedder: received zero-dimension embedding");
            }
            return vec;
        }
        if (res_json.contains("embedding") && res_json["embedding"].is_array())
        {
            auto vec = res_json["embedding"].get<EmbeddingVector>();
            if (vec.empty())
            {
                throw std::runtime_error("OllamaEmbedder: received zero-dimension embedding");
            }
            return vec;
        }
        throw std::runtime_error("OllamaEmbedder: JSON missing embedding data");
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("OllamaEmbedder JSON parse error: " + std::string(e.what()));
    }
}

} // namespace oop_agent
