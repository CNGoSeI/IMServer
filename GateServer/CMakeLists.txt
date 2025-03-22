cmake_minimum_required (VERSION 3.20)

project (${GateServerProName} LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 20)

message("GateServer项目 PROJECT_NAME: ${PROJECT_NAME} ")

aux_source_directory(./Src GateServerSrc)

add_executable (${PROJECT_NAME} 
	${GateServerSrc}
)

#头文件
target_include_directories(${PROJECT_NAME} PRIVATE ./Include)
#target_include_directories(${PROJECT_NAME} PRIVATE ../Common)

#库文件
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system)
target_link_libraries(${PROJECT_NAME} PRIVATE ${JSONLIB_PATH})

#要拷贝的dll文件
set(DLL_FILES
	${JSONCPP_DLL_PATH}
)


COPY_DLLS(${PROJECT_NAME} DLL_FILES)  #拷贝该项目需要的dll