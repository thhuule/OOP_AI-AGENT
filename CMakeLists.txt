cmake_minimum_required(VERSION 3.15)
project(AI_AGENT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(ai_agent
    src/main.cpp
    src/SkillLoader.cpp
    # thêm file .cpp khác vào đây dần
)