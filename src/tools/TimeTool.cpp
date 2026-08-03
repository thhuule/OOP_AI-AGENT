#include "TimeTool.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace oop_agent
{

std::string_view
TimeTool::get_name() const noexcept
{
    return "time";
}

std::string_view
TimeTool::get_description() const noexcept
{
    return "Get the current local date and time.";
}

std::expected<std::string, ToolError>
TimeTool::execute(const std::string& arguments)
{
    

    try
    {
        if (!arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }
        auto now =
            std::chrono::system_clock::now();

        std::time_t current_time =
            std::chrono::system_clock::to_time_t(now);

        std::tm local_time{};

#ifdef _WIN32
        localtime_s(&local_time, &current_time);
#else
        localtime_r(&current_time, &local_time);
#endif

        std::ostringstream out;

        out << std::put_time(
            &local_time,
            "%Y-%m-%d %H:%M:%S");

        return out.str();
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent