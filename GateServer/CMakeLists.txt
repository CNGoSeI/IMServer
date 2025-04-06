cmake_minimum_required (VERSION 3.20)

project (GateServer LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)
message("GateServer项目 PROJECT_NAME: ${PROJECT_NAME} ")

#依赖common下的cpp
file(GLOB COMMON_FILES
	"${COMMON_PATH}/ConfigMgr.cpp"
	"${COMMON_PATH}/AsioIOServicePool.cpp"
	"${COMMON_PATH}/RedisMgr.cpp"
	"${COMMON_PATH}/MysqlDAO.cpp"
	"${COMMON_PATH}/ThreadWorkerTemplate.h"
	"${COMMON_PATH}/PubFuncLib.cpp"
)

#依赖的proto文件
file(GLOB PROTO_FILES
	"${PROTO_OUT_DIR}/message.pb.cc"
	"${PROTO_OUT_DIR}/message.grpc.pb.cc"
)

aux_source_directory(./Src GateServerSrc)

add_executable (${PROJECT_NAME} 
	${GateServerSrc}
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
	Boost::system
	Boost::filesystem
	redis++::redis++
	hiredis::hiredis
	mysql::concpp
	mysql::concpp-jdbc
	protobuf::libprotobuf
)

file(GLOB CopyTargets
	${JSONCPP_DLL_PATH}
	"config.ini"
	${MySQL_DLLS}
)


COPY_FILES(${PROJECT_NAME} CopyTargets)  #拷贝该项目需要的dll