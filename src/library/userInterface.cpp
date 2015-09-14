#include "userInterface.hpp"


using namespace soaringPen;

/**
This function initializes the user interface and starts the communication thread.
@param inputContext: The ZMQ context to use
@param inputControllerPairInterfaceURI: The URI "ip:port" of the controller's pair interface to pair with the GUI
@param inputControllerVideoPublishingURI: The interface that the controller publishes video on

@throws: This function can throw exceptions
*/
userInterface::userInterface(zmq::context_t &inputContext, const std::string &inputControllerPairInterfaceURI, const std::string &inputControllerVideoPublishingURI)
{

//Make window
setupUi(this);
//Add way to close in full screen mode without adding a button (CTRL + Q)
new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));

context = &inputContext;

//Initialize/connect the sockets before passing them to the communication thread 
SOM_TRY //Init
commandInterface.reset(new zmq::socket_t(*(context), ZMQ_PAIR));
SOM_CATCH("Error intializing commandInterface\n")

SOM_TRY //Connect
std::string connectionString = "tcp://" + inputControllerPairInterfaceURI;
commandInterface->connect(connectionString.c_str());
SOM_CATCH("Error connecting commandInterface socket")

SOM_TRY //Init
videoSubscriber.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error intializing videoSubscriber\n")

SOM_TRY //Set filter to allow any published messages to be received
videoSubscriber->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for videoSubscriber\n")

SOM_TRY //Connect
std::string connectionString = "tcp://" + inputControllerVideoPublishingURI;
videoSubscriber->connect(connectionString.c_str());
SOM_CATCH("Error connecting videoSubscriber socket")

//Setup communication thread
SOM_TRY
communicationThread.reset(new userInterfaceCommunicationThread(*commandInterface, *videoSubscriber));
SOM_CATCH("Error starting communication thread\n")

connect(communicationThread.get(), SIGNAL(cameraImage(QPixmap)), this, SLOT(overlayVideoFrame(const QPixmap &)));

connect(this, SIGNAL(videoFrameWithOverlay(QPixmap)), videoDisplayLabel, SLOT(setPixmap(const QPixmap &)));

//Register to receive events that occur in the video display label
videoDisplayLabel->installEventFilter(this);


communicationThread->start();
}

/**
This function tells the communication thread to shutdown
*/
userInterface::~userInterface()
{
communicationThread->shutdown = true;
}


/**
This function receives a video frame and overlays the current path (and any other required graphics) on it.
@param inputVideoFrame: The video frame to process

@throws: This function can throw exceptions
*/
void userInterface::overlayVideoFrame(const QPixmap &inputVideoFrame)
{
//Copy image to make version to emit
QPixmap buffer = inputVideoFrame;

//Scale to fit label while maintaining aspect ratio
buffer = buffer.scaled(videoDisplayLabel->size(),Qt::KeepAspectRatio);

cameraImageSize = fPoint(buffer.width(), buffer.height());

//Draw stuff
QPainter painter(&buffer);

QPen penSettings = painter.pen();
penSettings.setWidth(3);

painter.setPen(penSettings);

//Convert path to scaled picture coordinates
std::vector<std::pair<int, int> > convertedPath;
for(auto iter = path.points.begin(); iter != path.points.end(); iter++)
{
convertedPath.push_back(normalizedImageCoordinateToImageCoordinate((*iter).val[0], (*iter).val[1]));
}

//Draw path with linear interpolation
for(int i=1; i<convertedPath.size(); i++)
{
painter.drawLine(convertedPath[i-1].first+.5, convertedPath[i-1].second+.5, convertedPath[i].first+.5, convertedPath[i].second+.5);
}

//TODO: Remove
static double pathLocation = 0.01;
if(currentlyDrawingPath || pathLocation > path.pathLength)
{
pathLocation = 0.01;
}
else
{
fPoint pointToDraw = path.interpolate(pathLocation);
auto convertedPointToDraw = normalizedImageCoordinateToImageCoordinate(pointToDraw.val[0], pointToDraw.val[1]);

painter.drawEllipse(convertedPointToDraw.first-5, convertedPointToDraw.second-5, 10,10);
pathLocation += .01;
}

//printf("Path size: %ld\n", convertedPath.size());

emit videoFrameWithOverlay(buffer);
}

/**
This function makes it possible for the main window to handle events that happen in it's widgets.  It is called when an event registered via installEventFilter happens in the registered object.
@param inputTriggeringObject: A pointer to the object the event happened in
@param inputEvent: A pointer to the event that occurred
@return: true if event was handled
*/
bool userInterface::eventFilter(QObject *inputTriggeringObject, QEvent *inputEvent)
{
if(inputTriggeringObject == videoDisplayLabel)
{
if(inputEvent->type() == QEvent::MouseButtonPress)
{
currentlyDrawingPath = true;
path.clear(); //Erase old path, start new one
}

if(inputEvent->type() == QEvent::MouseButtonRelease)
{
currentlyDrawingPath = false; //Path completed
}

if(inputEvent->type() == QEvent::MouseMove)
{
QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(inputEvent);
std::pair<double, double> normalizedMousePosition = videoDisplayLabelCoordinateToNormalizedImageCoordinate(mouseMoveEvent->x(), mouseMoveEvent->y());
path.addPoint(fPoint(normalizedMousePosition.first, normalizedMousePosition.second));

//Smooth line by removing short segments
//path.regularize(.03);
path.regularize(.1);


//Limit path length
path.truncate(4.0);
}

}
}


/**
This function converts between the coordinate system associated with the video display label and the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension).
@param inputXCoordinate: The X coordinate in the video display label coordinate system
@param inputYCoordinate: The Y coordinate in the video display label coordinate system
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<double, double> userInterface::videoDisplayLabelCoordinateToNormalizedImageCoordinate(int inputXCoordinate, int inputYCoordinate)
{
if(cameraImageSize.val[0] == 0.0 || cameraImageSize.val[0] == 0.0)
{ //Can't divide by zero
return std::pair<double, double>(0.0,0.0);
}

//Centers should still be aligned, despite different aspect ratios
fPoint inputPoint(inputXCoordinate, inputYCoordinate);
fPoint labelSize(videoDisplayLabel->size().width(), videoDisplayLabel->size().height());


fPoint differenceFromCenter = inputPoint - .5*labelSize;

//Normalize
differenceFromCenter.val[0] = differenceFromCenter.val[0]/cameraImageSize.mag();
differenceFromCenter.val[1] = differenceFromCenter.val[1]/cameraImageSize.mag();

return std::pair<double, double>(differenceFromCenter.val[0], differenceFromCenter.val[1]);
}

/**
This function converts between the coordinate system associated with the scaled image used with the label and the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension).
@param inputXCoordinate: The X coordinate in the video display image coordinate system
@param inputYCoordinate: The Y coordinate in the video display image coordinate system
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<double, double> userInterface::imageCoordinateToNormalizedImageCoordinate(int inputXCoordinate, int inputYCoordinate)
{
if(cameraImageSize.val[0] == 0.0 || cameraImageSize.val[0] == 0.0)
{ //Can't divide by zero
return std::pair<double, double>(0.0,0.0);
}

fPoint pointBuffer(inputXCoordinate, inputYCoordinate);

pointBuffer.val[0] = pointBuffer.val[0]/cameraImageSize.mag();
pointBuffer.val[1] = pointBuffer.val[1]/cameraImageSize.mag();

return std::pair<double, double>(pointBuffer.val[0],pointBuffer.val[1]);
}

/**
This function converts between the relative coordinate system used with the camera image (centered at the middle of the image and ranging from -1 to +1 on each dimension) and the coordinate system associated with the scaled image used with the label.
@param inputXCoordinate: The relative X Coordinate
@param inputYCoordinate: The relative X Coordinate
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<int, int> userInterface::normalizedImageCoordinateToImageCoordinate(double inputXCoordinate, double inputYCoordinate)
{
if(cameraImageSize.val[0] == 0.0 || cameraImageSize.val[0] == 0.0)
{ //Can't divide by zero
return std::pair<int, int>(0,0);
}

//Centers should still be aligned, despite different aspect ratios
fPoint pointBuffer(inputXCoordinate*cameraImageSize.mag(), inputYCoordinate*cameraImageSize.mag());

pointBuffer = pointBuffer + .5*cameraImageSize;


return std::pair<int, int>(pointBuffer.val[0]+.5, pointBuffer.val[1]+.5);
}

/**
This function converts between the coordinate system associated with the video display label and the coordinate system associated with the scaled image used with the label.
@param inputXCoordinate: The relative X Coordinate
@param inputYCoordinate: The relative X Coordinate
@return: <relativeXCoordinate, relativeYCoordinate>
*/
std::pair<int, int> userInterface::videoDisplayLabelCoordinateToImageCoordinate(int inputXCoordinate, int inputYCoordinate)
{

std::pair<double, double> normalizedCoordinates = videoDisplayLabelCoordinateToNormalizedImageCoordinate(inputXCoordinate, inputYCoordinate);

return normalizedImageCoordinateToImageCoordinate(normalizedCoordinates.first, normalizedCoordinates.second);
}
