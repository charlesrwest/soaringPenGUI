/**
\mainpage
Introduction
---------------------
_______________

Hello and welcome.

This code framework is meant to assist students in getting started with C++ programming using an experimental approach.  This is done by encouraging the student to test their assumptions and code using unit testing as they learn the language.

As it is currently set up, new source can be added to the library (new functionality to test) by simply placing the source/header files in ./src/library/

New tests can be added by modifying the main.cpp file in ./src/executables/unitTests (according to the format defined in https://github.com/philsquared/Catch ).

More executables which use the dynamic link library can be added by adding 3 lines for each executable to the cmake file (./CMakeLists.txt).  Stick them next to the similar lines already there for the unit test executable:

FILE(GLOB NAME_OF_NEW_EXECUTABLE_EXECUTABLE_SOURCE ./src/executables/nameOfNewExecutableSourceFolder/*.cpp ./src/executables/nameOfNewExecutableSourceFolder/*.c)

ADD_EXECUTABLE(nameOfNewExecutable ${NAME_OF_NEW_EXECUTABLE_EXECUTABLE_SOURCE}) 

target_link_libraries(nameOfNewExecutable learnCppByExperiment)

Adding new files to documentation
---------------------
_______________

Files in the library folder will automatically be added to documentation but source folders for new executables has to be manually added.

To add a new executable folder to the documentation, add append the following token to ./Doxyfile.in at the line starting with "INPUT = " (search).

@CMAKE_CURRENT_SOURCE_DIR@/src/nameOfNewExecutableSourceFolder/name/

Compilation
---------------------
_______________

The source can be downloaded via: 
https://github.com/charlesrwest/learnCpp/archive/master.zip

Alternatively, it can be cloned from GitHub using the following git command:

git clone https://github.com/charlesrwest/learnCpp.git

Required libraries/utilities:
G++ with support for C++11 (most systems support this)
CMake (http://www.cmake.org/)
Doxygen (http://www.stack.nl/~dimitri/doxygen/ if documentation is desired)

Embedded Libraries:

Catch, a header only unit test framework (https://github.com/philsquared/Catch)

On Ubuntu Linux, it should be possible to install all required dependencies with the following command:

sudo apt-get install build-essential cmake doxygen

Compilation can be carried out via the following commands performed in the downloaded project directory:

cmake ./

make

make doc

The "make doc" command is only nessesary if access to nice documentation is desired.  This step generates html files which allow browsing the code documentation in a web browser via the /learnCppByExperiment/doc/html/index.html file (just double click on the file and it should open a browser).  The linkToGeneratedDocumentation.html file has been added to the top level directory to make it easier to get the the generated index file.

The integrity of the code can be checked via running the ./bin/unitTests executable.  If some of the unit tests fail on your platform, please email crwest@ncsu.edu with a screenshot of your error and a description of your operating enviroment.

Using This software
---------------------
_______________

This code currently makes one executable and one shared library.  The executable is meant to allow the student to unit test their code.

./lib/liblearnCppByExperiment.so:

This shared library includes most of the functionality that the student will write and is linked to by all of the executables made in the project.

./bin/unitTests:

This binary can be run to run unit tests defined in ./src/executables/unitTests

These tests can be used to identify and correct logic and other runtime errors.


*/ 
