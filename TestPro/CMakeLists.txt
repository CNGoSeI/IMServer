cmake_minimum_required (VERSION 3.20)



project (TestPro LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)

#依赖common下的cpp
file(GLOB COMMON_FILES
	"${COMMON_PATH}/RedisMgr.cpp"
)
message("通用类文件夹： ${COMMON_PATH}")

#依赖的proto文件
file(GLOB PROTO_FILES
)

file(GLOB ALL_CPP
 "./*.cpp"
)

add_executable (${PROJECT_NAME} 
	${ALL_CPP}
	#${GateServerSrc}
	${COMMON_FILES}
	#${PROTO_FILES}
)

#头文件
target_include_directories(${PROJECT_NAME} PRIVATE ./Include)

#库文件
target_link_libraries(${PROJECT_NAME} PRIVATE 
	#${JSONLIB_PATH}
	#gRPC::grpc++
	#gRPC::grpc++_reflection
	#protobuf::libprotobuf
	#Boost::system
	#Boost::filesystem
	redis++::redis++
	hiredis::hiredis
)

file(GLOB CopyTargets
	#${JSONCPP_DLL_PATH}
	#"config.ini"
)


#COPY_FILES(${PROJECT_NAME} CopyTargets)  #拷贝该项目需要的dll