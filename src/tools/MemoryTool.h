#pragma once

#include "Tool.h"
#include "Embedding.h"

#include <memory>
#include <sqlite3.h>
#include <string>

namespace oop_agent
{

class MemoryTool final : public Tool
{
public:
    MemoryTool();
    explicit MemoryTool(std::unique_ptr<Embedder> embedder);
    explicit MemoryTool(std::string db_path, std::unique_ptr<Embedder> embedder = nullptr);
    ~MemoryTool() override;

    [[nodiscard]] const std::string& get_db_path() const noexcept { return db_path_; }

    /// W10.5-B-02: kiểm tra DB đã mở thành công và sẵn sàng nhận lệnh.
    [[nodiscard]] bool is_ready() const noexcept { return db_ != nullptr; }

    [[nodiscard]]
    std::string_view
    get_name() const noexcept override;

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

private:
    std::string db_path_;
    sqlite3* db_;
    std::unique_ptr<Embedder> embedder_;

    bool init_database();

    std::expected<std::string, ToolError>
    save_memory(const std::string& text);

    std::expected<std::string, ToolError>
    search_memory(const std::string& keyword);

    /// BNS-V-01: lưu text kèm vector embedding.
    std::expected<std::string, ToolError>
    vsave_memory(const std::string& text);

    /// BNS-V-01: tìm top-N memory theo cosine similarity.
    std::expected<std::string, ToolError>
    vsearch_memory(const std::string& query, int top_k);
};

} // namespace oop_agent
