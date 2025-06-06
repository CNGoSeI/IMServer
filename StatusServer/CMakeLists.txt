﻿cmake_minimum_required (VERSION 3.20)
project (StatuServer LANGUAGES CXX )
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
)

#依赖的proto文件
file(GLOB PROTO_FILES
	"${PROTO_OUT_DIR}/StatusService.pb.cc"
	"${PROTO_OUT_DIR}/StatusService.grpc.pb.cc"
)

aux_source_directory(./Src StatuServerSrc)
file(GLOB StatuServerHed "./Include/*.h" )

add_executable (${PROJECT_NAME} 
	${StatuServerHed}
	${StatuServerSrc}
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
	redis++::redis++
	hiredis::hiredis
	mysql::concpp
	mysql::concpp-jdbc

)

file(GLOB CopyTargets
	${JSONCPP_DLL_PATH}
	"config.ini"
	${MySQL_DLLS}
)


COPY_FILES(${PROJECT_NAME} CopyTargets)