cmake_minimum_required (VERSION 3.20)

project (GateServer LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)

message("GateServer项目 PROJECT_NAME: ${PROJECT_NAME} ")
add_executable (${PROJECT_NAME} 
 GateServer.cpp
)

