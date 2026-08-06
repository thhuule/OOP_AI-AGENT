cmake_minimum_required(VERSION 3.28)
project(OopAiAgent VERSION 1.0.0 LANGUAGES CXX)

include(CTest)

# Bật chuẩn C++23 ở level CMake, nhưng ép dùng -std=c++26 cho compiler hỗ trợ inplace_vector
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Xuất compile_commands.json cho clangd/IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tìm các thư viện bên thứ ba bắt buộc
find_package(CURL REQUIRED)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # The repository vendors nlohmann/json.hpp under include/.  Use an
    # interface target when the system CMake package is unavailable.
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
endif()
find_package(Threads REQUIRED) # Dùng cho Multi-agent Threading
find_package(SQLite3 REQUIRED)

# Thu thập source code tự động cho các executable phụ/eval
file(GLOB_RECURSE SOURCE_FILES
    "src/agent/*.cpp"
    "src/client/*.cpp"
    "src/tools/*.cpp"
    "src/harness/*.cpp"
    "src/environment/*.cpp"
    "src/multiagent/*.cpp"
)

# 1. Executable OopAgent (Liệt kê tường minh theo yêu cầu Tuần 6)
add_executable(OopAgent
    src/main.cpp
    src/agent/agent_loop.cpp
    src/agent/SkillLoader.cpp
    src/agent/LoopDetector.cpp          # MỚI: Bổ sung LoopDetector
    src/client/ollama_client.cpp        # MỚI: Giữ lại OllamaClient cho Polymorphism
    src/client/gemini_client.cpp        # MỚI: Bổ sung GeminiClient
    src/tools/CalculatorTool.cpp
    src/tools/ExecTool.cpp
    src/tools/FileTool.cpp
    src/tools/WebSearchTool.cpp
    src/tools/ToolRegistry.cpp
    src/tools/MemoryTool.cpp
    src/tools/TimeTool.cpp
    src/tools/JsonTool.cpp
    src/tools/GitTool.cpp
    src/multiagent/MultiAgentRunner.cpp
)

# 2. Executable run_eval (Sử dụng toàn bộ SOURCE_FILES bao gồm cả Ollama và Gemini)
add_executable(run_eval
    benchmark/run_eval.cpp
    ${SOURCE_FILES}
)

add_executable(test_multi_agent
    benchmark/test_multi_agent.cpp
    ${SOURCE_FILES}
)

add_executable(test_harness
    benchmark/test_harness.cpp
    ${SOURCE_FILES}
)

add_executable(test_tools
    benchmark/test_tools.cpp
    ${SOURCE_FILES}
)

add_executable(demo_multi_agent
    benchmark/demo_multi_agent.cpp
    ${SOURCE_FILES}
)

if(BUILD_TESTING)
    add_test(NAME harness COMMAND test_harness)
    add_test(NAME multi_agent COMMAND test_multi_agent)
    add_test(NAME tools COMMAND test_tools)
endif()

# Cấu hình Include Directories
set(common_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
)

target_include_directories(OopAgent PRIVATE ${common_include_dirs})
target_include_directories(run_eval PRIVATE ${common_include_dirs})
target_include_directories(test_multi_agent PRIVATE ${common_include_dirs})
target_include_directories(test_harness PRIVATE ${common_include_dirs})
target_include_directories(test_tools PRIVATE ${common_include_dirs})
target_include_directories(demo_multi_agent PRIVATE ${common_include_dirs})

# Link Libraries
target_link_libraries(OopAgent PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

target_link_libraries(run_eval PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

target_link_libraries(test_multi_agent PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

target_link_libraries(test_harness PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

target_link_libraries(test_tools PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

target_link_libraries(demo_multi_agent PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)

# Flag biên dịch nghiêm ngặt
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OopAgent PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
    target_compile_options(run_eval PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
    target_compile_options(test_multi_agent PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
    target_compile_options(test_harness PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
    target_compile_options(test_tools PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
    target_compile_options(demo_multi_agent PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
elseif(MSVC)
    target_compile_options(OopAgent PRIVATE /W4 /std:c++latest)
    target_compile_options(run_eval PRIVATE /W4 /std:c++latest)
    target_compile_options(test_harness PRIVATE /W4 /std:c++latest)
endif()
