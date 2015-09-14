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

signals:
/**
This signal emits an image to display as part of the video stream from the controller.
*/
void cameraImage(QPixmap);

protected:
/*
This function is the code that is run in the seperate thread.  It is responsible for managing the processes and emitting signals via an event loop.
*/
void run() Q_DECL_OVERRIDE;

/**
This function receives any messages waiting on videoSubscriberSocket and attempts to emit them as a cameraImage Qt signal for display.

@throws: This function can throw exceptions
*/
void convertVideoFrameMessageToSignal();
};


/**
This function converts a JPeg to a QPixmap so that it can be displayed.
@param inputJPegData: A pointer to the JPeg data
@param inputJPegDataSize: The size in bytes of the jpeg data

@throws: This function can throw exceptions
*/
QPixmap convertJPegToQPixMap(char *inputJPegData, int JPegDataSize);




}
