# 项目IMServers 顶层 CMake 项目文件，在此处执行全局配置

cmake_minimum_required (VERSION 3.20)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (IMServers LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)

# 下面进行配置 #
include("${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")#vcpkg必选
set(COMMON_PATH "${CMAKE_SOURCE_DIR}/Common")
message("common文件夹 ${COMMON_PATH}")

if(WIN32)
set(VCPKG_TOOL_PATH "${VCPKG_INSTALLED_DIR}/x64-windows/tools")
endif()
message("VCPKG 工具路径 ${VCPKG_TOOL_PATH}")

# proto文件处理 #
set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Protos")
file(GLOB PROTO_MESSAGES ${PROTO_PATH}/*.proto)
message("项目proto文件：${PROTO_MESSAGES}")
set(PROTO_OUT_DIR "${PROTO_PATH}/out")

#jsoncpp
set(JSONCPP_ROOT "${JSONCPP_HEADER}/..")

find_package(Boost COMPONENTS system filesystem CONFIG REQUIRED)  # 显式指定组件
find_package(Protobuf CONFIG REQUIRED)#protobuf 引入
find_package(gRpc CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(mysql-concpp REQUIRED)

if(WIN32)
set(JSONCPP_DLL_PATH "${JSONCPP_ROOT}/bin/jsoncpp.dll")
set(JSONLIB_PATH "${JSONCPP_ROOT}/lib/jsoncpp.lib")

set(MySQL_DLLS 
	${mysql-concpp_ROOT}/lib64/mysqlcppconn-10-vs14.dll
	${mysql-concpp_ROOT}/lib64/mysqlcppconnx-2-vs14.dll
)
endif()

# 将各dll拷贝至构建目录的函数
function(COPY_FILES target_name Files_list)
    foreach(file IN LISTS ${Files_list})
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${file}"
                $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying DLL: ${file}"
        )
        message("项目：${target_name}拷贝文件：${file} 拷贝至 $<TARGET_FILE_DIR:${target_name}")
    endforeach()
endfunction()

# 全局包含路径（所有子项目继承）
include_directories(
    ${JSONCPP_HEADER}       # JSONCPP 头文件目录
    ./Common
    ./Protos/out
)

# 包含子项目。
add_subdirectory (./Protos)
add_subdirectory (./GateServer)
add_subdirectory (./TestPro)