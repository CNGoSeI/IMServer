cmake_minimum_required (VERSION 3.20)

project (GateServer LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)

message("GateServer项目 PROJECT_NAME: ${PROJECT_NAME} ")

aux_source_directory(./Src GateServerSrc)

add_executable (${PROJECT_NAME} 
	${GateServerSrc}
	"${CMAKE_SOURCE_DIR}/Common/ConfigMgr.cpp"
	"${CMAKE_SOURCE_DIR}/Common/AsioIOServicePool.cpp"
	"${PROTO_OUT_DIR}/message.pb.cc"
	"${PROTO_OUT_DIR}/message.grpc.pb.cc"
)

#头文件
target_include_directories(${PROJECT_NAME} PRIVATE ./Include)
#target_include_directories(${PROJECT_NAME} PRIVATE ../Common)

#库文件
target_link_libraries(${PROJECT_NAME} PRIVATE 
	${JSONLIB_PATH}
	gRPC::grpc++
	gRPC::grpc++_reflection
	protobuf::libprotobuf
	Boost::system
	Boost::filesystem
)

file(GLOB CopyTargets
	${JSONCPP_DLL_PATH}
	"config.ini"
)


COPY_FILES(${PROJECT_NAME} CopyTargets)  #拷贝该项目需要的dll