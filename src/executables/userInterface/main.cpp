#include "userInterface.hpp"
#include<cstdio>
#include<QApplication>
#include "SOMException.hpp"
#include<memory>

using namespace soaringPen;

int main(int argc, char **argv)
{
QApplication app(argc, argv);

if(argc < 3)
{
fprintf(stderr, "Error, missing arguments\nUsage: %s 'ipOfController:portNumberOfController' 'ipOfVideoSource:portNumberOfVideoSource'", argv[0]);
}

std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing context\n");

std::unique_ptr<userInterface> myUserInterface;

SOM_TRY
myUserInterface.reset(new userInterface(*context, std::string(argv[1]), std::string(argv[2]) ));
SOM_CATCH("Error, unable to initialize user interface\n")

myUserInterface->show();

//Set to cover screen
myUserInterface->showFullScreen();



return app.exec();
}
