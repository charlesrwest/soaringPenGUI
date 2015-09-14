#include "userInterface.hpp"
#include<cstdio>
#include<QApplication>
#include "SOMException.hpp"
#include<memory>

using namespace soaringPen;

int main(int argc, char **argv)
{
QApplication app(argc, argv);

std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing context\n");

std::unique_ptr<userInterface> myUserInterface;

SOM_TRY
myUserInterface.reset(new userInterface(*context, "127.0.01:9001", "127.0.01:9002"));
SOM_CATCH("Error, unable to initialize user interface\n")

myUserInterface->show();

//Set to cover screen
//myUserInterface->showFullScreen();



return app.exec();
}
