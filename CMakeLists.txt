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


#QT stuff
#Access QT5 package
find_package(Qt5Widgets)

#Make moc called as nessisary
set(CMAKE_AUTOMOC ON)

#Get c++11
ADD_DEFINITIONS(-std=c++11)

find_package(Protobuf REQUIRED)

#Generate the C++ for the messages from the libprotobuf markup
add_subdirectory(./messages)

#Tell compiler where to find required libraries
link_directories(/usr/lib/x86_64-linux-gnu/ /usr/local/lib/)

include_directories(src/library/ src/executables/unitTests /usr/local/include/aruco/ src/executables/simpleTagTracker src/executables/simpleMarkerTracker src/executables/testVideoPublisher src/executables/testVideoSubscriber src/executables/userInterface src/executables/controller automoc messages)

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES src/library/*.cpp src/library/*.c automoc/*.cpp automoc/*.c)

FILE(GLOB FORM_FILES src/library/*.ui)

FILE(GLOB LIBRARY_HEADER_FILES src/library/*.hpp src/library/*.h ${QT_USE_FILE} ${QT_HEADERS})

FILE(GLOB UNIT_TESTS_EXECUTABLE_SOURCE src/executables/unitTests/*.cpp src/executables/unitTests/*.c)

FILE(GLOB SIMPLE_TAG_TRACKER_EXECUTABLE_SOURCE src/executables/simpleTagTracker/*.cpp src/executables/simpleTagTracker/*.c)

FILE(GLOB SIMPLE_MARKER_TRACKER_EXECUTABLE_SOURCE src/executables/simpleMarkerTracker/*.cpp src/executables/simpleMarkerTracker/*.c)

FILE(GLOB TEST_VIDEO_PUBLISHER_EXECUTABLE_SOURCE src/executables/testVideoPublisher/*.cpp src/executables/testVideoPublisher/*.c)

FILE(GLOB TEST_VIDEO_SUBSCRIBER_EXECUTABLE_SOURCE src/executables/testVideoSubscriber/*.cpp src/executables/testVideoSubscriber/*.c)

FILE(GLOB USER_INTERFACE_EXECUTABLE_SOURCE src/executables/userInterface/*.cpp src/executables/userInterface/*.c)

FILE(GLOB CONTROLLER_EXECUTABLE_SOURCE src/executables/controller/*.cpp src/controller/userInterface/*.c)

#Add the QT modules we are going to use
#QT_USE_QTNETWORK QT_USE_QTOPENGL QT_USE_QTSQL QT_USE_QTXML QT_USE_QTSVG QT_USE_QTTEST QT_USE_QTDBUS QT_USE_QTSCRIPT QT_USE_QTWEBKIT QT_USE_QTXMLPATTERNS QT_USE_PHONON
SET(QT_USE_QTCORE TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(LIBRARY_HEADERS ${LIBRARY_HEADER_FILES} ${QT_USE_FILE} ${QT_HEADERS})
SET(LIBRARY_FORMS ${FORM_FILES})

#Set the UIs and resources to be processed
QT5_WRAP_UI(LIBRARY_FORMS_HEADERS ${LIBRARY_FORMS})

#Add QT definitions
ADD_DEFINITIONS(${QT_DEFINITIONS})


#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")


#Create a libraries
add_library(soaringPen SHARED  ${SOURCEFILES} ${LIBRARY_HEADERS_MOC} ${LIBRARY_FORMS_HEADERS} ${CMAKE_CURRENT_BINARY_DIR})


#Tell CMake what executables to make
ADD_EXECUTABLE(unitTests ${UNIT_TESTS_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(simpleTagTracker ${SIMPLE_TAG_TRACKER_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(simpleMarkerTracker ${SIMPLE_MARKER_TRACKER_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(testVideoPublisher  ${TEST_VIDEO_PUBLISHER_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(testVideoSubscriber  ${TEST_VIDEO_SUBSCRIBER_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(userInterface  ${USER_INTERFACE_EXECUTABLE_SOURCE}) 

ADD_EXECUTABLE(controller  ${CONTROLLER_EXECUTABLE_SOURCE}) 


target_link_libraries(soaringPen dl ${CMAKE_THREAD_LIBS_INIT} opencv_core opencv_highgui opencv_calib3d aruco Qt5::Widgets soaringPenMessages zmq ${PROTOBUF_LIBRARY})

#link libraries to executable
target_link_libraries(unitTests soaringPen)

target_link_libraries(simpleTagTracker soaringPen)

target_link_libraries(simpleMarkerTracker soaringPen)

target_link_libraries(testVideoPublisher soaringPen)

target_link_libraries(testVideoSubscriber soaringPen)

target_link_libraries(userInterface soaringPen)

target_link_libraries(controller soaringPen)


