#pragma once

#include<QThread>
#include<QApplication>
#include<zmq.hpp>
#include<QPixmap>
#include "SOMException.hpp"
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<memory>
#include<QPixmap>
#include<QImage>
#include<cstdio>
#include "utilityFunctions.hpp"
#include "fPoint.hpp"

#include "gui_command.pb.h"
#include "follow_path_command.pb.h"
#include "emergency_stop_command.pb.h"
#include "controller_status_update.pb.h"

namespace soaringPen
{

/**
This class manages communications between the GUI and the demo manager node.  This mostly consists of translating between QT signals/slots and ZMQ/Protobuf messages.
*/
class userInterfaceCommunicationThread : public QThread
{
Q_OBJECT

public:
/*
This function initializes the processManagerThread.
@param inputCommandSocket: A reference to the ZMQ PAIR socket to use for communications with the controller
@param inputVideoSubscriberSocket: A reference to the ZMQ SUB socket to use for getting the video steam
@param inputParent: This is a pointer to the parent QT object (for cascade delete purposes).

@throws: This function can throw exceptions
*/
userInterfaceCommunicationThread(zmq::socket_t &inputCommandSocket, zmq::socket_t &inputVideoSubscriberSocket, QObject *inputParent = nullptr);

/*
This function cleans up the object and waits for the thread to stop running before returning.
*/
~userInterfaceCommunicationThread();

zmq::socket_t &commandSocket;
zmq::socket_t &videoSubscriberSocket;

bool shutdown = false; //Flag to indicate thread should shutdown

public slots:
/**
This function sends a follow path command to the ardrone controller.
@param inputFollowPathCommand: The path to follow (0 length paths are ignored)

@throw: This function can throw exceptions
*/
void sendFollowPathCommand(follow_path_command inputFollowPathCommand);

/**
This function sends a return to home command message to the AR drone controller.
@throw: This function can throw exceptions 
*/
void sendReturnToHomeCommand();

/**
This function sends a signal to activate the emergency stop message to the AR drone controller.
@throw: This function can throw exceptions 
*/
void sendEmergencyStopCommand();

signals:
/**
This signal emits an image to display as part of the video stream from the controller.
*/
void cameraImage(QPixmap);

/**
This signal emits the current battery percentage of the drone's battery.
*/
void droneBatteryStatus(double);

/**
This signal emits the drone's current X velocity when it receives an update.
*/
void droneXVelocity(double);

/**
This signal emits the drone's current Y velocity when it receives an update.
*/
void droneYVelocity(double);

/**
Emits the status updates associated with the controller's updates.
*/
void controllerStatusUpdate(controller_status_update);

protected:
fPoint velocityMovingAverage;

/*
This function is the code that is run in the seperate thread.  It is responsible for managing the processes and emitting signals via an event loop.
*/
void run() Q_DECL_OVERRIDE;

/**
This function receives any messages waiting on videoSubscriberSocket and attempts to emit them as a cameraImage Qt signal for display.

@throws: This function can throw exceptions
*/
void convertVideoFrameMessageToSignal();

/**
This function receives any messages waiting on commandSocket and emits the associated signals for display.

@throw: This function can throw exceptions
*/
void convertStatusUpdateMessageToSignals();

};


/**
This function converts a JPeg to a QPixmap so that it can be displayed.
@param inputJPegData: A pointer to the JPeg data
@param inputJPegDataSize: The size in bytes of the jpeg data

@throws: This function can throw exceptions
*/
QPixmap convertJPegToQPixMap(char *inputJPegData, int JPegDataSize);




}
