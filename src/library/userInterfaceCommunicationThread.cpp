#include "userInterfaceCommunicationThread.hpp"

using namespace soaringPen;

/*
This function initializes the processManagerThread.
@param inputCommandSocket: A reference to the ZMQ PAIR socket to use for communications with the controller
@param inputVideoSubscriberSocket: A reference to the ZMQ SUB socket to use for getting the video steam
@param inputParent: This is a pointer to the parent QT object (for cascade delete purposes).

@throws: This function can throw exceptions
*/
userInterfaceCommunicationThread::userInterfaceCommunicationThread(zmq::socket_t &inputCommandSocket, zmq::socket_t &inputVideoSubscriberSocket, QObject *inputParent) : commandSocket(inputCommandSocket), videoSubscriberSocket(inputVideoSubscriberSocket), QThread(inputParent)
{
qRegisterMetaType<controller_status_update>("controller_status_update");

this->moveToThread(this);

//connect(this, SIGNAL(rosCoreStatusChanged(bool)), this, SLOT(handleNodesChanges(bool)));
}

/*
This function cleans up the object and waits for the thread to stop running before returning.
*/
userInterfaceCommunicationThread::~userInterfaceCommunicationThread()
{
quit(); //Tell thread event loop to exit
wait(); //Wait for thread to exit
}

/**
This function sends a follow path command to the ardrone controller.
@param inputFollowPathCommand: The path to follow (0 length paths are ignored)

@throw: This function can throw exceptions
*/
void userInterfaceCommunicationThread::sendFollowPathCommand(follow_path_command inputFollowPathCommand)
{
if(inputFollowPathCommand.path_x_coordinates_size() == 0 || inputFollowPathCommand.path_y_coordinates_size() == 0)
{
return;
}

//Intitialize command
gui_command command;
(*command.mutable_follow_path_command_field()) = inputFollowPathCommand;

SOM_TRY
pylongps::sendProtobufMessage(commandSocket, command);
SOM_CATCH("Error sending follow path command\n");
}

/**
This function sends a return to home command message to the AR drone controller.
@throw: This function can throw exceptions 
*/
void userInterfaceCommunicationThread::sendReturnToHomeCommand()
{
//Intitialize command
gui_command command;
command.set_return_to_home(true);

SOM_TRY
pylongps::sendProtobufMessage(commandSocket, command);
SOM_CATCH("Error sending follow path command\n");
}

/**
This function sends a signal to activate the emergency stop message to the AR drone controller.
@throw: This function can throw exceptions 
*/
void userInterfaceCommunicationThread::sendEmergencyStopCommand()
{
//Intitialize command
emergency_stop_command stopCommand;
stopCommand.set_stop(true);

gui_command command;
(*command.mutable_emergency_stop_command_field()) = stopCommand;

SOM_TRY
pylongps::sendProtobufMessage(commandSocket, command);
SOM_CATCH("Error sending follow path command\n");
}


/*
This function is the code that is run in the seperate thread.  It is responsible for managing the processes and emitting signals via an event loop.
*/
void userInterfaceCommunicationThread::run()
{
try
{
//Setup ZMQ polling
zmq::pollitem_t pollItems[] = { {(void *) (commandSocket), 0, ZMQ_POLLIN, 0}, {(void *) (videoSubscriberSocket), 0, ZMQ_POLLIN, 0} };

//Start event loop, handing slots and timer events
while(true)
{
if(shutdown == true)
{
break; //End loop
}

SOM_TRY
QCoreApplication::processEvents(); //Process Qt events/slots, etc
SOM_CATCH("Error processing events");

//See if any messages have been received, polling for 10 milliseconds
SOM_TRY
if(zmq::poll(pollItems, 2, 1) == 0)
{
continue; //Poll returned without indicating any messages have been received, so check events and go back to polling
}
SOM_CATCH("Error polling\n")

//Check if a status update has been received
if(pollItems[0].revents & ZMQ_POLLIN)
{
//Process status update
SOM_TRY
convertStatusUpdateMessageToSignals();
SOM_CATCH("Error updating and emitting signal\n")
}

//Check if a new video frame has been received
if(pollItems[1].revents & ZMQ_POLLIN)
{
//Convert video frame from jpeg to QPixMap and emit it as a signal
SOM_TRY
convertVideoFrameMessageToSignal();
SOM_CATCH("Error converting/forwarding image\n")
}


}
}
catch(const std::exception &inputException)
{
fprintf(stderr, "%s\n", inputException.what());
return;
}

}

/**
This function receives any messages waiting on videoSubscriberSocket and attempts to emit them as a cameraImage Qt signal for display.

@throws: This function can throw exceptions
*/
void userInterfaceCommunicationThread::convertVideoFrameMessageToSignal()
{
while(true)
{ //Process all queued messages
//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(videoSubscriberSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving video stream message")

SOM_TRY
emit cameraImage(convertJPegToQPixMap((char *) messageBuffer->data(), messageBuffer->size()));
SOM_CATCH("Error converting/emitting video frame\n")
}

}

/**
This function receives any messages waiting on commandSocket and emits the associated signals for display.

@throw: This function can throw exceptions
*/
void userInterfaceCommunicationThread::convertStatusUpdateMessageToSignals()
{
while(true)
{ //Process all queued messages
//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;


SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

/* THIS CAUSED AN ERROR and was not suppose to be here
SOM_TRY //Receive message
if(commandSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving status update message")
*/


bool messageReceived = false;
bool messageDeserialized = false;
controller_status_update statusUpdate;
try
{
SOM_TRY //Receive/deserialize message
std::tie(messageReceived, messageDeserialized) = pylongps::receiveProtobufMessage(commandSocket, statusUpdate, ZMQ_DONTWAIT);
SOM_CATCH("Error receiving status update message")
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Error: %s\n", inputException.what());
}

if(!messageReceived || !messageDeserialized)
{ //Something when wrong with getting the message, so return
return; 
}

//Update moving average
velocityMovingAverage = .8*fPoint(statusUpdate.x_velocity(), statusUpdate.y_velocity())+.2*velocityMovingAverage;


SOM_TRY //Emit defaults if the optional fields are not present
emit controllerStatusUpdate(statusUpdate);
emit droneBatteryStatus(statusUpdate.battery_status());
emit droneXVelocity(((int) velocityMovingAverage.val[0])/100.0);
emit droneYVelocity(((int) velocityMovingAverage.val[1])/100.0);
SOM_CATCH("Error converting/emitting signals\n")
printf("Emitted data\n");
}

}


/**
This function converts a JPeg to a QPixmap so that it can be displayed.
@param inputJPegData: A pointer to the JPeg data
@param inputJPegDataSize: The size in bytes of the jpeg data

@throws: This function can throw exceptions
*/
QPixmap soaringPen::convertJPegToQPixMap(char *inputJPegData, int JPegDataSize)
{

if(inputJPegData == nullptr || JPegDataSize < 0)
{
throw SOMException("Either null data pointer or invalid data size\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

std::vector<unsigned char> encodedImage((unsigned char *) inputJPegData, ((unsigned char *) inputJPegData)+JPegDataSize); //Copy to vector

//Decompress/decode image
cv::Mat convertedOpenCVImage;

{ //Limit scope to preserve RAM
cv::Mat opencvImage;
SOM_TRY
opencvImage = cv::imdecode(encodedImage, CV_LOAD_IMAGE_COLOR);
SOM_CATCH("Error decoding image\n")

//Convert to proper byte order
cv::cvtColor(opencvImage, convertedOpenCVImage, CV_BGR2RGB);
}



//Convert to Qt format
QImage temporaryImage(convertedOpenCVImage.data, convertedOpenCVImage.cols, convertedOpenCVImage.rows, QImage::Format_RGB888);

QImage buffer; //Copy image so that image memory is safe to use
buffer = temporaryImage;


return QPixmap::fromImage(buffer);
}

