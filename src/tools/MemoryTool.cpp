#include "MemoryTool.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace oop_agent
{

MemoryTool::MemoryTool()
    : db_(nullptr)
    , embedder_(std::make_unique<HashEmbedder>())
{
    init_database();
}

MemoryTool::~MemoryTool()
{
    if (db_)
    {
        sqlite3_close(db_);
    }
}

std::string_view
MemoryTool::get_name() const noexcept
{
    return "memory";
}

std::string_view
MemoryTool::get_description() const noexcept
{
    return "Save and search memories using SQLite. "
           "Args: 'save <text>', 'search <keyword>', "
           "'vsave <text>' (lưu kèm vector embedding), "
           "'vsearch <text>' (tìm theo cosine similarity).\n"
           "Examples: save Tokyo, search Tokyo, vsearch weather in Tokyo";
}

bool MemoryTool::init_database()
{
    if (sqlite3_open("memory.db", &db_) != SQLITE_OK)
    {
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS memories("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "text TEXT NOT NULL,"
        "embedding BLOB);";

    char* err = nullptr;

    int rc = sqlite3_exec(
        db_,
        sql,
        nullptr,
        nullptr,
        &err);

    if (rc != SQLITE_OK)
    {
        sqlite3_free(err);
        return false;
    }

    // BNS-V-01: migration — thêm cột embedding nếu DB cũ chưa có.
    sqlite3_stmt* check = nullptr;
    const char* pragma = "PRAGMA table_info(memories);";

    bool has_embedding = false;

    if (sqlite3_prepare_v2(db_, pragma, -1, &check, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(check) == SQLITE_ROW)
        {
            const unsigned char* col =
                sqlite3_column_text(check, 1);

            if (col && std::strcmp(
                    reinterpret_cast<const char*>(col),
                    "embedding") == 0)
            {
                has_embedding = true;
                break;
            }
        }
    }

    sqlite3_finalize(check);

    if (!has_embedding)
    {
        const char* alter =
            "ALTER TABLE memories "
            "ADD COLUMN embedding BLOB;";

        sqlite3_exec(db_, alter, nullptr, nullptr, nullptr);
    }

    return true;
}

std::expected<std::string, ToolError>
MemoryTool::save_memory(const std::string& text)
{
    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "INSERT INTO memories(text) VALUES(?);";

    if (sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &stmt,
            nullptr)
        != SQLITE_OK)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    sqlite3_bind_text(
        stmt,
        1,
        text.c_str(),
        -1,
        SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);

        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    sqlite3_finalize(stmt);

    return "Memory saved.";
}

std::expected<std::string, ToolError>
MemoryTool::search_memory(
    const std::string& keyword)
{
    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "SELECT text FROM memories "
        "WHERE text LIKE ?;";

    if (sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &stmt,
            nullptr)
        != SQLITE_OK)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    std::string pattern =
        "%" + keyword + "%";

    sqlite3_bind_text(
        stmt,
        1,
        pattern.c_str(),
        -1,
        SQLITE_TRANSIENT);

    std::stringstream result;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* text =
            sqlite3_column_text(stmt, 0);

        if (text)
        {
            result
                << reinterpret_cast<
                       const char*>(text)
                << '\n';
        }
    }

    sqlite3_finalize(stmt);

    auto output = result.str();

    if (output.empty())
    {
        return "No memory found.";
    }

    return output;
}

std::expected<std::string, ToolError>
MemoryTool::vsave_memory(const std::string& text)
{
    const EmbeddingVector vec =
        embedder_->embed(text);

    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "INSERT INTO memories(text, embedding) "
        "VALUES(?, ?);";

    if (sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &stmt,
            nullptr)
        != SQLITE_OK)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    sqlite3_bind_text(
        stmt,
        1,
        text.c_str(),
        -1,
        SQLITE_TRANSIENT);

    const float* data = vec.data();
    const int bytes = static_cast<int>(
        vec.size() * sizeof(float));

    sqlite3_bind_blob(
        stmt,
        2,
        static_cast<const void*>(data),
        bytes,
        SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);

        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    sqlite3_finalize(stmt);

    return "Memory saved with vector embedding.";
}

std::expected<std::string, ToolError>
MemoryTool::vsearch_memory(
    const std::string& query,
    int top_k)
{
    const EmbeddingVector query_vec =
        embedder_->embed(query);

    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "SELECT text, embedding FROM memories "
        "WHERE embedding IS NOT NULL;";

    if (sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &stmt,
            nullptr)
        != SQLITE_OK)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    struct Scored {
        std::string text;
        float score;
    };

    std::vector<Scored> ranked;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* text =
            sqlite3_column_text(stmt, 0);

        const void* blob =
            sqlite3_column_blob(stmt, 1);

        const int bytes =
            sqlite3_column_bytes(stmt, 1);

        if (!text || !blob ||
            bytes <= 0 ||
            bytes % static_cast<int>(sizeof(float)) != 0)
        {
            continue;
        }

        const auto count =
            static_cast<std::size_t>(bytes) /
            sizeof(float);

        EmbeddingVector stored(
            static_cast<const float*>(blob),
            static_cast<const float*>(blob) + count);

        const float score =
            cosine_similarity(query_vec, stored);

        ranked.push_back({
            reinterpret_cast<const char*>(text),
            score});
    }

    sqlite3_finalize(stmt);

    std::sort(
        ranked.begin(),
        ranked.end(),
        [](const Scored& a, const Scored& b) {
            return a.score > b.score;
        });

    if (ranked.empty())
    {
        return "No vector memory found.";
    }

    std::stringstream result;

    const std::size_t limit =
        static_cast<std::size_t>(
            std::max(top_k, 1));

    const std::size_t count =
        std::min(limit, ranked.size());

    for (std::size_t i = 0; i < count; ++i)
    {
        result << i + 1 << ". ["
               << ranked[i].score << "] "
               << ranked[i].text << '\n';
    }

    return result.str();
}

std::expected<std::string, ToolError>
MemoryTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::stringstream ss(arguments);

        std::string command;

        ss >> command;

        if (command == "save")
        {
            std::string text;

            std::getline(ss, text);

            if (!text.empty() &&
                text.front() == ' ')
            {
                text.erase(0, 1);
            }

            if (text.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            return save_memory(text);
        }

        if (command == "search")
        {
            std::string keyword;

            ss >> keyword;

            if (keyword.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            return search_memory(keyword);
        }

        if (command == "vsave")
        {
            std::string text;

            std::getline(ss, text);

            if (!text.empty() &&
                text.front() == ' ')
            {
                text.erase(0, 1);
            }

            if (text.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            return vsave_memory(text);
        }

        if (command == "vsearch")
        {
            std::string query;

            std::getline(ss, query);

            if (!query.empty() &&
                query.front() == ' ')
            {
                query.erase(0, 1);
            }

            if (query.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            return vsearch_memory(query, 3);
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
