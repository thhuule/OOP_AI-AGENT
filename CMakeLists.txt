cmake_minimum_required(VERSION 3.28) # Nâng version tối thiểu để hỗ trợ tốt C++26/Modules nếu cần
project(OopAiAgent VERSION 1.0.0 LANGUAGES CXX)

# Bật chuẩn C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Xuất compile_commands.json cho clangd/IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tìm các thư viện bên thứ ba bắt buộc [cite: 45]
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED) # Dùng cho Multi-agent Threading [cite: 218]
find_package(SQLite3 REQUIRED)

# Thu thập source code [cite: 129]
file(GLOB_RECURSE SOURCE_FILES
    "src/agent/*.cpp"
    "src/client/*.cpp"
    "src/tools/*.cpp"
    "src/harness/*.cpp"
)

add_executable(OopAgent
    src/main.cpp
    src/agent/agent_loop.cpp
    src/agent/SkillLoader.cpp
    src/client/ollama_client.cpp
    src/tools/CalculatorTool.cpp
    src/tools/ExecTool.cpp
    src/tools/FileTool.cpp
    src/tools/WebSearchTool.cpp
    src/tools/ToolRegistry.cpp
)

add_executable(run_eval
    benchmark/run_eval.cpp
    ${SOURCE_FILES}
)

set(common_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
)

target_include_directories(OopAgent PRIVATE ${common_include_dirs})
target_include_directories(run_eval PRIVATE ${common_include_dirs})

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
)

# Flag nghiêm ngặt + hỗ trợ C++23 đầy đủ
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OopAgent PRIVATE -Wall -Wextra -Wpedantic -std=c++23)
    target_compile_options(run_eval PRIVATE -Wall -Wextra -Wpedantic -std=c++23)
elseif(MSVC)
    target_compile_options(OopAgent PRIVATE /W4 /std:c++latest)
    target_compile_options(run_eval PRIVATE /W4 /std:c++latest)
endif()

