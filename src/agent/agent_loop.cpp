#include "agent/agent_loop.h"
#include "tools/Tool.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <print>
#include <regex>
#include <string_view>
#include <thread>
#include <type_traits>
#include <variant>
#if defined(__has_include)
#if __has_include(<inplace_vector>)
#include <inplace_vector>
#endif
#endif

namespace oop_agent {
namespace {

#if defined(__cpp_lib_inplace_vector)
template <typename T, size_t Capacity>
using FixedCapacityVector = std::inplace_vector<T, Capacity>;
#else
// Fallback: std::inplace_vector (C++26) is not always available on the
// compiler used for this build. We keep the C++26 feature guarded behind
// __cpp_lib_inplace_vector and fall back to a reserve()-backed std::vector
// with the same push_back/erase surface for portability. This is NOT a
// removal of the C++26 requirement — it activates automatically once the
// toolchain ships <inplace_vector>.
template <typename T, size_t Capacity>
class FixedCapacityVector {
public:
    FixedCapacityVector() {
        data_.reserve(Capacity);
    }

    void push_back(const T& value) {
        data_.push_back(value);
    }

    void erase(typename std::vector<T>::iterator it) {
        data_.erase(it);
    }

    [[nodiscard]] size_t size() const noexcept {
        return data_.size();
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return Capacity;
    }

    auto begin() noexcept { return data_.begin(); }
    auto end() noexcept { return data_.end(); }

private:
    std::vector<T> data_;
};
#endif

constexpr std::string_view trim_sv(std::string_view sv) noexcept {
    auto start = sv.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    auto end = sv.find_last_not_of(" \t\n\r");
    return sv.substr(start, end - start + 1);
}

// Heuristic: does this text look like the model TRIED to call a tool but
// used a format the parser doesn't recognize? Used to decide whether to
// retry-with-instructions instead of silently treating it as a final answer
// (checklist: "nếu sai format nhưng có ý định gọi tool, agent nhắc lại
// protocol thay vì coi là final answer").
bool looks_like_attempted_tool_call(std::string_view text) {
    static const std::array<std::string_view, 9> markers = {
        "\"tool\"", "'tool'", "ACTION:", "call:", "functionCall",
        "I will use", "I will call", "Plan:", "Tool:"
    };
    for (auto marker : markers) {
        if (text.find(marker) != std::string_view::npos) {
            return true;
        }
    }
    return false;
}

std::optional<ToolCallAction> parse_json_tool_call(
    const std::string& candidate) {
    try {
        const auto json = nlohmann::json::parse(candidate);
        const nlohmann::json* call = &json;
        if (json.contains("functionCall") && json["functionCall"].is_object())
            call = &json["functionCall"];

        std::string tool_name;
        if (call->contains("tool") && (*call)["tool"].is_string())
            tool_name = (*call)["tool"].get<std::string>();
        else if (call->contains("name") && (*call)["name"].is_string())
            tool_name = (*call)["name"].get<std::string>();
        if (tool_name.empty())
            return std::nullopt;

        std::string args;
        if (call->contains("args") && !(*call)["args"].is_null()) {
            args = (*call)["args"].is_string()
                ? (*call)["args"].get<std::string>()
                : (*call)["args"].dump();
        } else if (call->contains("arguments") &&
                   !(*call)["arguments"].is_null()) {
            args = (*call)["arguments"].is_string()
                ? (*call)["arguments"].get<std::string>()
                : (*call)["arguments"].dump();
        }
        return ToolCallAction{std::move(tool_name), std::move(args)};
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

std::string tool_error_message(ToolError error) {
    switch (error) {
    case ToolError::InvalidArgument:
        return "ToolError: InvalidArgument. Check the documented args format.";
    case ToolError::ExecutionFailed:
        return "ToolError: ExecutionFailed.";
    case ToolError::AccessDenied:
        return "ToolError: AccessDenied.";
    case ToolError::NotFound:
        return "ToolError: NotFound.";
    case ToolError::UnknownError:
        return "ToolError: UnknownError.";
    }
    return "ToolError: UnknownError.";
}

std::string normalize_for_comparison(std::string_view value) {
    std::string normalized;
    normalized.reserve(value.size());
    bool pending_space = false;
    for (const unsigned char ch : value) {
        if (std::isspace(ch)) {
            pending_space = !normalized.empty();
            continue;
        }
        if (pending_space) {
            normalized.push_back(' ');
            pending_space = false;
        }
        normalized.push_back(static_cast<char>(std::tolower(ch)));
    }
    return normalized;
}

} // namespace

void AgentLoop::truncate_history(std::vector<Message>& history, size_t max_messages) {
    if (history.empty()) return;
    if (history.size() <= max_messages) return;

    constexpr size_t kHistoryCapacity = 12;
    const size_t effective_max_messages = std::min(max_messages, kHistoryCapacity);

    FixedCapacityVector<Message, kHistoryCapacity> truncated;
    truncated.push_back(history.front()); // always keep the system prompt

    size_t start_idx = history.size() - (effective_max_messages - 1);
    for (size_t i = start_idx; i < history.size(); ++i) {
        truncated.push_back(history[i]);
    }
    history.assign(truncated.begin(), truncated.end());
}

Action AgentLoop::parse_llm_response(const std::string& llm_text) {
    std::string clean_text{trim_sv(llm_text)};

    // 1. JSON inside a ```json ... ``` markdown fence.
    std::regex md_json_regex(R"(```(?:json)?\s*(\{[\s\S]*?\})\s*```)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(clean_text, match, md_json_regex)) {
        clean_text = match[1].str();
    }

    // Parse with a real JSON parser so quoted arguments are correctly
    // unescaped and nested object args remain intact.
    if (const auto call = parse_json_tool_call(clean_text))
        return *call;

    // Models occasionally wrap a valid object in a short prose prefix.
    const auto json_start = clean_text.find('{');
    const auto json_end = clean_text.rfind('}');
    if (json_start != std::string::npos && json_end != std::string::npos &&
        json_end > json_start) {
        if (const auto call = parse_json_tool_call(
                clean_text.substr(json_start, json_end - json_start + 1)))
            return *call;
    }

    // 3. Textual protocol: ACTION: tool_name(args)
    size_t action_pos = clean_text.find("ACTION:");
    if (action_pos != std::string::npos) {
        std::string_view action_str = clean_text;
        action_str = action_str.substr(action_pos + 7);
        size_t open = action_str.find("(");
        size_t close = action_str.rfind(")");

        if (open != std::string_view::npos && close != std::string_view::npos && close > open) {
            std::string_view raw_tool_name = action_str.substr(0, open);
            std::string_view clean_tool_name = trim_sv(raw_tool_name);
            std::string_view args_view = action_str.substr(open + 1, close - open - 1);

            return ToolCallAction{std::string(clean_tool_name), std::string(args_view)};
        }
    }

    // 4. call:provider:tool{args} style (occasionally emitted by Gemini-family
    //    models when they attempt native function calling in text form).
    size_t call_pos = clean_text.find("call:");
    if (call_pos != std::string::npos) {
        size_t provider = clean_text.find(':', call_pos + 5);
        if (provider != std::string::npos) {
            size_t brace = clean_text.find('{', provider);
            if (brace != std::string::npos) {
                std::string tool = clean_text.substr(provider + 1, brace - provider - 1);
                size_t end = clean_text.find('}', brace);
                if (end != std::string::npos) {
                std::string args = clean_text.substr(brace, end - brace + 1);
                return ToolCallAction{tool, args};
                }
            }
        }
    }

    return FinalAnswerAction{llm_text};
}

std::string AgentLoop::run(const std::string& user_instruction, int max_steps) {
    loop_detector_.reset();

    FixedCapacityVector<std::string, 10> recent_tool_calls;

    std::vector<Message> conversation_history;
    int successful_tool_calls = 0;
    bool pending_file_verification = false;
    std::string last_read_result;

    // System prompt: names the tools that are ACTUALLY registered (must match
    // benchmark/run_eval.cpp registration list), enforces a single strict
    // JSON output format, and forbids planning-only responses. The previous
    // "copy raw output / include the literal word PASS" instruction has been
    // removed: it coerced the model into echoing evaluator keywords rather
    // than actually completing the task, which produced false-positive PASS
    // results in the benchmark (see FIX_LOI_ROLE_ABC_BENCHMARK.md, item 4).
    std::string system_prompt =
        "You are an AI Agent that solves tasks by calling tools.\n"
        "Registered tools (call ONLY these names):\n"
        "  calculator      - evaluate an arithmetic expression, e.g. args: \"47 * 23\"\n"
        "  execute_shell   - run a shell command, e.g. args: \"ls -la\"\n"
        "  read_file       - read a file, e.g. args: \"notes.txt\"\n"
        "  write_file      - write a file, e.g. args: \"notes.txt,Hello world\" or {\"filename\":\"notes.txt\",\"content\":\"Hello world\"}\n"
        "  append_file     - append without overwriting, e.g. args: \"notes.txt,\\nmore text\"\n"
        "  web_search      - search the web, e.g. args: \"capital of Japan\"\n"
        "  memory          - \"save <text>\" or \"search <keyword>\"\n"
        "  time            - get current date/time, args: \"\"\n"
        "  json            - pretty-print JSON, args: a JSON string\n"
        "  git             - \"status\" | \"branch\" | \"log\" | \"diff\"\n"
        "\n"
        "CRITICAL RULES:\n"
        "1. To call a tool, respond with EXACTLY ONE JSON object and nothing else:\n"
        "   {\"tool\": \"tool_name\", \"args\": \"arguments\"}\n"
        "2. Do not write prose like 'I will call...' or 'Plan:' when you intend to call a tool.\n"
        "   Just emit the JSON object directly.\n"
        "3. Never invent a tool name that is not in the list above.\n"
        "4. Only give your final answer (plain text, no JSON) once the task is fully complete\n"
        "   and you have used tool results to verify it, if the task requires a tool.\n"
        "5. After write_file or append_file, call read_file and verify the exact final content before answering.\n"
        "6. Complete every clause in order. For 'then/after/sau do' tasks, do not stop after the first clause.\n";

    if (skill_loader_) {
        const std::string skills = skill_loader_->getSystemPrompt();
        if (!skills.empty()) {
            system_prompt += "\n---\nRelevant skills:\n" + skills;
        }
    }

    conversation_history.push_back({"system", system_prompt, std::nullopt});
    conversation_history.push_back({"user", user_instruction, std::nullopt});

    for (int step = 0; step < max_steps; ++step) {
        truncate_history(conversation_history, 12);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto response_result = client_->generate_chat(conversation_history);
        if (!response_result.has_value()) {
            std::println(std::cerr, "[AgentLoop] LLM Client Error at step {}!", step + 1);
            // Malformed/empty response: give the model one more chance with
            // an explicit reminder instead of aborting the whole task.
            conversation_history.push_back({"user",
                "System: your last response could not be parsed. Reply with "
                "EXACTLY one JSON object: {\"tool\": \"...\", \"args\": \"...\"}",
                std::nullopt});
            continue;
        }

        std::string llm_text = response_result.value();
        conversation_history.push_back({"assistant", llm_text, std::nullopt});
        Action action = parse_llm_response(llm_text);

        bool finished = false;
        std::string final_answer;

        std::visit([&](auto&& act) {
            using T = std::decay_t<decltype(act)>;

            if constexpr (std::is_same_v<T, ToolCallAction>) {
                if (recent_tool_calls.size() == recent_tool_calls.capacity()) {
                    recent_tool_calls.erase(recent_tool_calls.begin());
                }
                recent_tool_calls.push_back(act.tool_name);

                const std::string action_signature =
                    act.tool_name + "\n" + std::string(trim_sv(act.args));
                auto loop_status = loop_detector_.add_action(action_signature);

                if (loop_status == LoopDetector::Status::Warning) {
                    std::println(std::cerr, "[LoopDetector][WARNING] Possible loop detected for tool: '{}'", act.tool_name);
                } else if (loop_status == LoopDetector::Status::Critical) {
                    std::println(std::cerr, "[LoopDetector][CRITICAL] Infinite loop detected for tool: '{}'! Stopping Agent Loop.", act.tool_name);
                    conversation_history.push_back({"user", "System Error: Infinite loop detected. Stop calling this tool.", std::nullopt});
                    finished = true;
                    final_answer = "Agent stopped due to infinite loop detection.";
                    return;
                }

                if (!tools_.is_allowed(act.tool_name)) {
                    conversation_history.push_back({"user", "Error: Tool is not allowed: " + act.tool_name, std::nullopt});
                    return;
                }

                Tool* tool = tools_.get_tool(act.tool_name);
                if (tool) {
                    auto res = tool->execute(act.args);
                    std::string result = res.has_value()
                        ? res.value()
                        : tool_error_message(res.error());

                    nlohmann::json recorded_action = {
                        {"type", "tool_call"},
                        {"tool", act.tool_name},
                        {"args", act.args}
                    };

                    if (step_hook_) {
                        step_hook_(llm_text, recorded_action.dump(), result);
                    }

                    conversation_history.push_back({"tool", result, std::nullopt});
                    if (res.has_value()) {
                        ++successful_tool_calls;
                        if (act.tool_name == "write_file" ||
                            act.tool_name == "append_file" ||
                            act.tool_name == "create_file") {
                            pending_file_verification = true;
                        } else if (act.tool_name == "read_file" &&
                                   pending_file_verification) {
                            pending_file_verification = false;
                        }
                        if (act.tool_name == "read_file")
                            last_read_result = res.value();
                    }
                } else {
                    conversation_history.push_back({"user",
                        "Error: Tool not found: " + act.tool_name +
                        ". Use one of the registered tool names listed in the system prompt.",
                        std::nullopt});
                }
            } else if constexpr (std::is_same_v<T, FinalAnswerAction>) {
                // Guard against premature "final answers" that are actually
                // failed tool-call attempts (e.g. malformed JSON, or
                // 'I will call write_file...' prose). Nudge the model to
                // retry with the correct format instead of ending the task.
                if (looks_like_attempted_tool_call(act.content) && step + 1 < max_steps) {
                    conversation_history.push_back({"user",
                        "System: that looked like an attempted tool call but "
                        "wasn't valid JSON. Reply with EXACTLY one JSON object: "
                        "{\"tool\": \"tool_name\", \"args\": \"arguments\"}",
                        std::nullopt});
                    return;
                }
                if (successful_tool_calls == 0 && step + 1 < max_steps) {
                    conversation_history.push_back({"user",
                        "System: this task must be completed with a real tool call. "
                        "Call the appropriate tool now using the JSON protocol.",
                        std::nullopt});
                    return;
                }
                if (pending_file_verification && step + 1 < max_steps) {
                    conversation_history.push_back({"user",
                        "System: a file was changed but not verified. Call read_file "
                        "for the changed file, check every requested line/content, "
                        "and only then answer.",
                        std::nullopt});
                    return;
                }
                if (!last_read_result.empty() && last_read_result.size() <= 512 &&
                    normalize_for_comparison(act.content).find(
                        normalize_for_comparison(last_read_result)) ==
                        std::string::npos && step + 1 < max_steps) {
                    conversation_history.push_back({"user",
                        "System: your final answer did not faithfully report the "
                        "latest read_file observation. Answer the original task and "
                        "include the relevant file content/result exactly: " +
                        last_read_result,
                        std::nullopt});
                    return;
                }
                finished = true;
                final_answer = act.content;
            }
        }, action);

        if (finished) {
            return final_answer;
        }
    }

    return "Agent reached maximum step limit or stopped due to loop.";
}

} // namespace oop_agent
