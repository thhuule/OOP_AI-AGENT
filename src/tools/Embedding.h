#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace oop_agent
{

/// Độ dài vector embedding cố định cho persistent memory.
inline constexpr std::size_t kEmbeddingDim = 64;

using EmbeddingVector = std::vector<float>;

/**
 * @brief Tính cosine similarity giữa hai vector (dùng cho vector search).
 *
 * Deterministic, không phụ thuộc mạng. Hai vector rỗng → 0.0f.
 * Input không cần L2-normalize trước; hàm tự chuẩn hoá.
 */
[[nodiscard]]
float cosine_similarity(
    const EmbeddingVector& lhs,
    const EmbeddingVector& rhs);

/**
 * @brief Strategy Pattern — interface embedder để MemoryTool không phụ thuộc
 *        vào một model embedding cụ thể (dễ inject/mock trong test).
 */
class Embedder
{
public:
    virtual ~Embedder() = default;

    /// Chuyển text thành vector embedding cố định chiều.
    [[nodiscard]]
    virtual EmbeddingVector embed(const std::string& text) const = 0;
};

/**
 * @brief Embedder deterministic dựa trên character n-gram hashing.
 *
 * - Không cần mạng/model ngoài → chạy offline, test lặp lại ổn định.
 * - Cùng text → cùng vector (deterministic).
 * - Vector được L2-normalize nên cosine similarity = dot product.
 * Dùng làm provider tham chiếu; có thể thay bằng model thật (ví dụ
 * nomic-embed-text) qua interface Embedder mà không đổi MemoryTool.
 */
class HashEmbedder final : public Embedder
{
public:
    explicit HashEmbedder(std::size_t dim = kEmbeddingDim);

    [[nodiscard]]
    EmbeddingVector embed(const std::string& text) const override;

private:
    std::size_t dim_;
};

} // namespace oop_agent
