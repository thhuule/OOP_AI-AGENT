#include "ExecTool.h"

#include <chrono>
#include <csignal>
#include <expected>
#include <poll.h>
#include <string>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace oop_agent
{

std::string_view ExecTool::get_name() const noexcept
{
    return "execute_shell";
}

std::string_view ExecTool::get_description() const noexcept
{
    return "Execute an allowed shell command. Example: pwd";
}

std::expected<std::string, ToolError>
ExecTool::execute(const std::string& arguments)
{
    try {
    if (arguments.empty())
        return std::unexpected(ToolError::InvalidArgument);

    int pipefd[2];

    if (pipe(pipefd) != 0)
        return std::unexpected(ToolError::ExecutionFailed);

    //---------------------------------------
    // non-blocking
    //---------------------------------------

    int flags = fcntl(pipefd[0], F_GETFL);

    fcntl(
        pipefd[0],
        F_SETFL,
        flags | O_NONBLOCK);

    //---------------------------------------
    // fork
    //---------------------------------------

    pid_t pid = fork();

    if (pid < 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);

        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    //---------------------------------------
    // Child
    //---------------------------------------

    if (pid == 0)
    {
        setpgid(0,0);

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);

        execl(
            "/bin/sh",
            "sh",
            "-c",
            arguments.c_str(),
            (char*)nullptr);

        _exit(127);
    }

    //---------------------------------------
    // Parent
    //---------------------------------------

    close(pipefd[1]);

    std::string output;

    pollfd pfd;

    pfd.fd = pipefd[0];
    pfd.events = POLLIN;

    auto start =
        std::chrono::steady_clock::now();

    bool running = true;

    while (running)
    {
        //-----------------------------------
        // timeout?
        //-----------------------------------

        auto elapsed =
            std::chrono::steady_clock::now()
            - start;

        if (elapsed >
            std::chrono::seconds(10))
        {
            killpg(pid, SIGKILL);

            waitpid(pid,nullptr,0);

            close(pipefd[0]);

            return std::unexpected(
                ToolError::ExecutionFailed);
        }

        //-----------------------------------
        // read output
        //-----------------------------------

        int ready =
            poll(&pfd,1,100);

        if (ready > 0 &&
            (pfd.revents & POLLIN))
        {
            char buffer[4096];

            ssize_t n;

            while ((n = read(
                pipefd[0],
                buffer,
                sizeof(buffer))) > 0)
            {
                output.append(buffer,n);
            }
        }

        //-----------------------------------
        // finished?
        //-----------------------------------

        int status;

        pid_t r =
            waitpid(
                pid,
                &status,
                WNOHANG);

        if (r == pid)
            running = false;
    }

    //---------------------------------------
    // read remaining bytes
    //---------------------------------------

    while (true)
    {
        char buffer[4096];

        ssize_t n =
            read(pipefd[0],
                 buffer,
                 sizeof(buffer));

        if (n <= 0)
            break;

        output.append(buffer,n);
    }

        close(pipefd[0]);

    return output;
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

}
