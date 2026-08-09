#include "ScreenshotTool.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <unistd.h>

namespace oop_agent
{

std::string_view ScreenshotTool::get_name() const noexcept
{
    return "capture_screenshot";
}

std::string_view ScreenshotTool::get_description() const noexcept
{
    return "Capture the current screen and return the image as base64 PNG. "
           "Args: optional output hint (ignored); result feeds the VLM/GUI agent.";
}

std::string ScreenshotTool::base64_encode(const std::string& bytes)
{
    static constexpr char kTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string out;
    out.reserve(((bytes.size() + 2) / 3) * 4);

    std::size_t i = 0;

    while (i + 2 < bytes.size())
    {
        const unsigned int n =
            (static_cast<unsigned char>(bytes[i]) << 16) |
            (static_cast<unsigned char>(bytes[i + 1]) << 8) |
            static_cast<unsigned char>(bytes[i + 2]);

        out.push_back(kTable[(n >> 18) & 0x3F]);
        out.push_back(kTable[(n >> 12) & 0x3F]);
        out.push_back(kTable[(n >> 6) & 0x3F]);
        out.push_back(kTable[n & 0x3F]);

        i += 3;
    }

    const std::size_t remaining = bytes.size() - i;

    if (remaining == 1)
    {
        const unsigned int n =
            static_cast<unsigned char>(bytes[i]) << 16;

        out.push_back(kTable[(n >> 18) & 0x3F]);
        out.push_back(kTable[(n >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    }
    else if (remaining == 2)
    {
        const unsigned int n =
            (static_cast<unsigned char>(bytes[i]) << 16) |
            (static_cast<unsigned char>(bytes[i + 1]) << 8);

        out.push_back(kTable[(n >> 18) & 0x3F]);
        out.push_back(kTable[(n >> 12) & 0x3F]);
        out.push_back(kTable[(n >> 6) & 0x3F]);
        out.push_back('=');
    }

    return out;
}

std::expected<std::string, ToolError>
ScreenshotTool::capture_png()
{
    // Dùng file tạm trong thư mục tmp để tránh ghi vào workspace.
    char tmpl[] = "/tmp/oop_agent_screenshot_XXXXXX";

    const int fd = mkstemp(tmpl);

    if (fd < 0)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    close(fd);

    std::string cmd =
        "screencapture -x -t png \"" +
        std::string(tmpl) + "\" 2>/dev/null";

    const int rc = std::system(cmd.c_str());

    if (rc != 0)
    {
        std::remove(tmpl);

        return std::unexpected(
            ToolError::NotFound);
    }

    FILE* f = std::fopen(tmpl, "rb");

    if (!f)
    {
        std::remove(tmpl);

        return std::unexpected(
            ToolError::NotFound);
    }

    std::string data;

    char buffer[4096];
    std::size_t n = 0;

    while ((n = std::fread(
                buffer, 1, sizeof(buffer), f)) > 0)
    {
        data.append(buffer, n);
    }

    std::fclose(f);
    std::remove(tmpl);

    if (data.empty())
    {
        return std::unexpected(
            ToolError::NotFound);
    }

    return data;
}

std::expected<std::string, ToolError>
ScreenshotTool::execute(const std::string& arguments)
{
    try
    {
        (void)arguments; // không bắt buộc tham số; hint optional

        auto png = capture_png();

        if (!png.has_value())
        {
            return std::unexpected(png.error());
        }

        return "data:image/png;base64," +
               base64_encode(png.value());
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent
