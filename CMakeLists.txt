# 项目IMServers 顶层 CMake 项目文件，在此处执行全局配置

cmake_minimum_required (VERSION 3.20)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (IMServers LANGUAGES CXX )
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS system CONFIG REQUIRED)  # 显式指定组件

#手动指定其他路径
set(JSONCPP_ROOT "${JSONCPP_HEADER}/..")

if(WIN32)
set(JSONCPP_DLL_PATH "${JSONCPP_ROOT}/bin/jsoncpp.dll")
set(JSONLIB_PATH "${JSONCPP_ROOT}/lib/jsoncpp.lib")
endif()

# 将各dll拷贝至构建目录的函数
function(COPY_DLLS target_name dll_list)
    foreach(dll IN LISTS ${dll_list})
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}"
                $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying DLL: ${dll}"
        )
    endforeach()
endfunction()

# 全局包含路径（所有子项目继承）
include_directories(
    ${JSONCPP_HEADER}       # JSONCPP 头文件目录
    ./Common
)

#填写子项目名
set(GateServerProName "GateServer")

# 包含子项目。
add_subdirectory (./GateServer)