#include "MemoryTool.h"

#include <sstream>

namespace oop_agent
{

MemoryTool::MemoryTool()
    : db_(nullptr)
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
           "Commands:\n"
           "save <text>\n"
           "search <keyword>";
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
        "text TEXT NOT NULL);";

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

        return std::unexpected(
            ToolError::InvalidArgument);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

}