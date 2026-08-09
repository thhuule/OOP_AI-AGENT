#include "Embedding.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
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

} // namespace oop_agent
