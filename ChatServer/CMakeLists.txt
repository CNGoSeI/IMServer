﻿cmake_minimum_required (VERSION 3.20)
project (ChatServer LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)


#依赖common下的cpp
file(GLOB COMMON_FILES
	"${COMMON_PATH}/ConfigMgr.cpp"
	"${COMMON_PATH}/AsioIOServicePool.cpp"
	"${COMMON_PATH}/RedisMgr.cpp"
	"${COMMON_PATH}/MysqlDAO.cpp"
	"${COMMON_PATH}/ThreadWorkerTemplate.h"
	"${COMMON_PATH}/PubFuncLib.cpp"
	"${COMMON_PATH}/const.h"
	"${COMMON_PATH}/StatusGrpcClient.cpp"
	"${COMMON_PATH}/UserMgr.h"
	"${COMMON_PATH}/UserMgr.cpp"
)

#依赖的proto文件
file(GLOB PROTO_FILES
	"${PROTO_OUT_DIR}/StatusService.pb.cc"
	"${PROTO_OUT_DIR}/StatusService.grpc.pb.cc"
	"${PROTO_OUT_DIR}/message.pb.cc"
	"${PROTO_OUT_DIR}/message.grpc.pb.cc"
)

aux_source_directory(./Src ChatServerSrc)
file(GLOB ChatServerHed "./Include/*.h" )

add_executable (${PROJECT_NAME} 
	${ChatServerHed}
	${ChatServerSrc}
	${COMMON_FILES}
	${PROTO_FILES}
)

#头文件
target_include_directories(${PROJECT_NAME} PRIVATE ./Include)

#库文件
target_link_libraries(${PROJECT_NAME} PRIVATE 
	${JSONLIB_PATH}
	gRPC::grpc++
	gRPC::grpc++_reflection
	protobuf::libprotobuf
	Boost::system
	Boost::filesystem
	mysql::concpp
	mysql::concpp-jdbc
	redis++::redis++
	hiredis::hiredis

)

file(GLOB CopyTargets
	${JSONCPP_DLL_PATH}
	"config.ini"
	${MySQL_DLLS}
)


COPY_FILES(${PROJECT_NAME} CopyTargets)