cmake_minimum_required(VERSION 3.28) # Nâng version tối thiểu để hỗ trợ tốt C++26/Modules nếu cần
project(OopAiAgent VERSION 1.0.0 LANGUAGES CXX)

# Bật chuẩn C++26
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tìm các thư viện bên thứ ba bắt buộc [cite: 45]
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED) # Dùng cho Multi-agent Threading [cite: 218]

# Thu thập source code [cite: 129]
file(GLOB_RECURSE SOURCE_FILES
    "src/agent/*.cpp"
    "src/client/*.cpp"
    "src/tools/*.cpp"
    "src/harness/*.cpp"
)

add_executable(OopAgent benchmark/run_eval.cpp ${SOURCE_FILES})

target_include_directories(OopAgent PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(OopAgent PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Flag nghiêm ngặt + hỗ trợ C++26 hoàn chỉnh
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OopAgent PRIVATE -Wall -Wextra -Wpedantic -std=c++26)
elseif(MSVC)
    target_compile_options(OopAgent PRIVATE /W4 /std:c++latest)
endif()
