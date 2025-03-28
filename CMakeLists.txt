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

if(WIN32)
set(VCPKG_TOOL_PATH "${VCPKG_INSTALLED_DIR}/x64-windows/tools")
endif()
message("VCPKG 工具路径 ${VCPKG_TOOL_PATH}")

find_package(Boost COMPONENTS system CONFIG REQUIRED)  # 显式指定组件
find_package(Protobuf CONFIG REQUIRED)#protobuf 引入
find_package(gRpc CONFIG REQUIRED)
message(STATUS "Protoc 路径: ${PROTOBUF_PROTOC_EXECUTABLE}")

# proto文件处理 #
set(PROTO_PATH "Protos")
file(GLOB PROTO_MESSAGES ${PROTO_PATH}/*.proto)
message("项目proto文件：${PROTO_MESSAGES}")

set(PROTO_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Protos/out")
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# 根据平台判断插件扩展名[1,5](@ref)
if(WIN32)
    set(GRPC_PLUGIN_SUFFIX ".exe")
else()
    set(GRPC_PLUGIN_SUFFIX "")
endif()


FOREACH(cur_proto ${PROTO_MESSAGES})

 get_filename_component(proto_name ${cur_proto} NAME_WE)
    
    # 定义生成文件路径
    #set(pb_src "${PROTO_OUT_DIR}/${proto_name}.pb.cc")
    #set(pb_hdr "${PROTO_OUT_DIR}/${proto_name}.pb.h")
    #set(grpc_src "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.cc")
    #set(grpc_hdr "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.h")
    message("执行编译文件 ${cur_proto} 目标 ${pb_src}")
    message("编译proto: ${PROTOBUF_PROTOC_EXECUTABLE} -I=${CMAKE_CURRENT_SOURCE_DIR}/Protos --cpp_out=${PROTO_OUT_DIR} ${proto_name}.proto")
    # 生成Protobuf基础代码
    EXECUTE_PROCESS(
       # OUTPUT ${pb_src} ${pb_hdr}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} -I=${CMAKE_CURRENT_SOURCE_DIR}/Protos --cpp_out=${PROTO_OUT_DIR} ${proto_name}.proto
        #DEPENDS ${cur_proto}
        #COMMENT "Generating Protobuf code for ${proto_name}"
    )

    # 生成gRPC代码
    EXECUTE_PROCESS(
       # OUTPUT ${grpc_src} ${grpc_hdr}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --proto_path=${PROTO_PATH}
            --grpc_out=${PROTO_OUT_DIR}
            --plugin=protoc-gen-grpc="${VCPKG_TOOL_PATH}/grpc/grpc_cpp_plugin${GRPC_PLUGIN_SUFFIX}"
            ${cur_proto}
        DEPENDS ${cur_proto} #DEPENDS 部分的作用是 ​定义触发该自定义命令重新执行的依赖项
            "${VCPKG_TOOL_PATH}/grpc/grpc_cpp_plugin${GRPC_PLUGIN_SUFFIX}"
        COMMENT "Generating gRPC code for ${proto_name}"
    )

    # 收集生成文件
    #list(APPEND GENERATED_SOURCES ${pb_src} ${grpc_src})
    #list(APPEND GENERATED_HEADERS ${pb_hdr} ${grpc_hdr})
ENDFOREACH()

#jsoncpp
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
    ./Protos/out
)

#填写子项目名
set(GateServerProName "GateServer")

# 包含子项目。
add_subdirectory (./GateServer)