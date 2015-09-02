cmake_minimum_required (VERSION 2.8.3)
PROJECT(soaringPen)

#Enable doxygen documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)



#Get c++11
ADD_DEFINITIONS(-std=c++11)

#Tell compiler where to find required libraries
link_directories(/usr/lib/x86_64-linux-gnu/ /usr/local/lib/)

include_directories(./src/library/ ./src/executables/unitTests /usr/local/include/aruco/)

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c)

FILE(GLOB UNIT_TESTS_EXECUTABLE_SOURCE ./src/executables/unitTests/*.cpp ./src/executables/unitTests/*.c)


#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")


#Create a libraries
add_library(soaringPen SHARED  ${SOURCEFILES})


#Tell CMake what executables to make
ADD_EXECUTABLE(unitTests ${UNIT_TESTS_EXECUTABLE_SOURCE}) 

target_link_libraries(soaringPen dl ${CMAKE_THREAD_LIBS_INIT} opencv_core opencv_highgui opencv_calib3d aruco)

#link libraries to executable
target_link_libraries(unitTests soaringPen)


