#pragma once

#include<QMainWindow>
#include<QShortcut>
#include "ui_userInterfaceWindow.h"
#include<memory>
#include<zmq.hpp>
#include "userInterfaceCommunicationThread.hpp"
#include "SOMException.hpp"
#include<QPixmap>
#include<QPainter>
#include<tuple>
#include<QMouseEvent>
#include "fPoint.hpp"
#include "linearPath.hpp"
#include<cmath>
#include "controller_status_update.pb.h"


namespace soaringPen
{

const double IMAGE_PATH_MARGIN = .07; //The offset from the edges of the camera image that the path is allowed to have in relative coordinates

const int TRAVELLED_PATH_WIDTH = 1000;

class userInterface : public QMainWindow, public Ui::userInterfaceWindow
{
Q_OBJECT

public:
/**
This function initializes the user interface and starts the communication thread.
@param inputContext: The ZMQ context to use
@param inputControllerPairInterfaceURI: The URI "ip:port" of the controller's pair interface to pair with the GUI
@param inputControllerVideoPublishingURI: The interface that the controller publishes video on

@throws: This function can throw exceptions
*/
userInterface(zmq::context_t &inputContext, const std::string &inputControllerPairInterfaceURI, const std::string &inputControllerVideoPublishingURI);

/**
This function tells the communication thread to shutdown
*/
~userInterface();

zmq::context_t *context;

std::unique_ptr<zmq::socket_t> commandInterface; //A ZMQ PAIR socket which sends gui_command messages to the controller and gets controller_status_updates from the controller.  Used by the communication thread.

std::unique_ptr<zmq::socket_t> videoSubscriber; //A ZMQ SUB socket which subscribes to the controller's video publishing interface to get the live stream from the camera.  Used by the communication thread.

std::unique_ptr<userInterfaceCommunicationThread> communicationThread;

public slots:

/**
This function receives a video frame and overlays the current path (and any other required graphics) on it.
@param inputVideoFrame: The video frame to process

@throws: This function can throw exceptions
*/
void overlayVideoFrame(const QPixmap &inputVideoFrame);

/**
When activated, this slot emits the followPathCommandSignal to send the current path to the drone.  If the current path has a length of 1 or less, no signal is emitted.
*/
void emitFollowPathCommandSignal();

/**
Process the status update to get the drone position so that the green field path can be updated.
@param inputStatusUpdate: The status update to process

@throws: This function can throw exceptions
*/
void processStatusUpdateForFieldPath(const controller_status_update &inputStatusUpdate);

signals:
/**
This signal is any received video frame with the current path overlayed on it.
*/
void videoFrameWithOverlay(QPixmap);

/**
This signal is a follow path command for the drone to perform.
*/
void followPathCommandSignal(follow_path_command);


private:
fPoint cameraImageSize;
bool currentlyDrawingPath = false;
linearPath path; //The path to travel/draw
linearPath droneTravelledPath; //The path where the drone has actually gone


/**
This function makes it possible for the main window to handle events that happen in it's widgets.  It is called when an event registered via installEventFilter happens in the registered object.
@param inputTriggeringObject: A pointer to the object the event happened in
@param inputEvent: A pointer to the event that occurred
@return: true if event was handled
*/
bool eventFilter(QObject *inputTriggeringObject, QEvent *inputEvent);

/**
This function converts between the coordinate system associated with the video display label and the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension).
@param inputXCoordinate: The X coordinate in the video display label coordinate system
@param inputYCoordinate: The Y coordinate in the video display label coordinate system
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<double, double> videoDisplayLabelCoordinateToNormalizedImageCoordinate(int inputXCoordinate, int inputYCoordinate);

/**
This function converts between the coordinate system associated with the scaled image used with the label and the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension).
@param inputXCoordinate: The X coordinate in the video display image coordinate system
@param inputYCoordinate: The Y coordinate in the video display image coordinate system
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<double, double> imageCoordinateToNormalizedImageCoordinate(int inputXCoordinate, int inputYCoordinate);

/**
This function converts between the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension) and the coordinate system associated with the scaled image used with the label.
@param inputXCoordinate: The relative X Coordinate
@param inputYCoordinate: The relative X Coordinate
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<int, int> normalizedImageCoordinateToImageCoordinate(double inputXCoordinate, double inputYCoordinate);

/**
This function converts between the coordinate system associated with the video display label and the coordinate system associated with the scaled image used with the label.
@param inputXCoordinate: The relative X Coordinate
@param inputYCoordinate: The relative X Coordinate
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<int, int> videoDisplayLabelCoordinateToImageCoordinate(int inputXCoordinate, int inputYCoordinate);

};















}
