cmake_minimum_required(VERSION 2.8)

project(OpenHMDDemo)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#install properties
#install (TARGETS openhmd DESTINATION lib)
#install (FILES include/openhmd.h DESTINATION include)

find_package(OGRE REQUIRED)
find_package(OIS REQUIRED)
find_package(TinyXML REQUIRED)
find_package(OpenHMD REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(OpenGL REQUIRED)

include_directories(${OGRE_INCLUDE_DIRS} ${OIS_INCLUDE_DIRS} ${TINYXML_INCLUDE_DIRS} ${OPENHMD_INCLUDE_DIRS})
link_libraries(openhmd m ${OGRE_LIBRARIES} ${OIS_LIBRARY} ${TinyXML_LIBRARIES} ${OPENHMD_LIBRARY})
set(OpenHMDDemo_src main.cpp DotSceneLoader.cpp OpenHMD.cpp)
add_executable(OpenHMDDemo ${OpenHMDDemo_src})