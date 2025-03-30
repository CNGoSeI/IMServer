####################################
#   运行工具，编译cppProt和grpcProt #
#         文件监测、增量更新        #
####################################
project (GenerateProtos)

message(STATUS "Protoc 路径: ${PROTOBUF_PROTOC_EXECUTABLE}")


#file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# 根据平台判断插件扩展名[1,5](@ref)
if(WIN32)
    set(GRPC_PLUGIN_SUFFIX ".exe")
else()
    set(GRPC_PLUGIN_SUFFIX "")
endif()

FOREACH(PROTO_FILE ${PROTO_MESSAGES})
  # 提取文件名和路径
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)

    # 定义生成的 .pb.cc 和 .pb.h 路径
    set(PB_CC "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc")
    set(PB_H "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_CC "${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_H "${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.h")

    # 自定义命令：仅当 proto 文件修改时执行
    add_custom_command(
        OUTPUT ${PB_CC} ${PB_H} ${GRPC_CC} ${GRPC_H} #这里的OUT 和下面的DEPENDS进行了对应，构建监测关系
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
            -I=${PROTO_PATH}
            --cpp_out=${PROTO_OUT_DIR}
            --grpc_out=${PROTO_OUT_DIR} #grpc式的文件
            --plugin=protoc-gen-grpc=${VCPKG_TOOL_PATH}/grpc/grpc_cpp_plugin${GRPC_PLUGIN_SUFFIX} #grpc插件位置
            ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
    )
        # 将生成的文件加入源码列表
    list(APPEND GENERATED_SOURCES ${PB_CC} ${GRPC_CC})
ENDFOREACH()

# 创建自定义目标，聚合所有生成任务 这会将变化的proto文件进行编译
# DEPENDS 参数声明 任务 依赖于这些文件(.cc)的存在性和时效性
add_custom_target(${PROJECT_NAME} ALL DEPENDS ${GENERATED_SOURCES})#ALL 参数会让目标会被默认包含在构建流程中，跟随项目构建

message("编译Proto项目的名称：${PROJECT_NAME}")