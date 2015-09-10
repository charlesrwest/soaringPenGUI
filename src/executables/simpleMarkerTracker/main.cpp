#include<cstdio>
#include "aruco.h"
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<string>
#include "SOMException.hpp"

//Mostly a reimplementation of the aruco_test example from the aruco library.
const double HARRIS_THRESHOLD_PARAMETER_1 = 7.0;
const double HARRIS_THRESHOLD_PARAMETER_2 = 7.0;
const double REPROJECTION_TEST_THRESHOLD = 1.5;
const double DETECTION_PROBABILITY_THRESHOLD = .5;

int main(int argc, char **argv)
{

if(argc < 4)
{
fprintf(stderr, "Error, incorrect number of arguments.  \nUsage: videoDeviceNumber cameraIntrinsics.yml sizeOfMarkerInMetersDiagonal/Side?\n");
return 1;
}

int videoDeviceNumber = 0;
SOM_TRY
videoDeviceNumber = std::stol(std::string(argv[1]));
SOM_CATCH("Error, unable to read video device number\n")

std::string cameraIntrinsicsFilePath = argv[2];
double boardSizeInMeters = atof(argv[3]);
if(fabs(boardSizeInMeters) < .001)
{
fprintf(stderr, "Error, board size is invalid\n");
return 1;
} 

cv::VideoCapture imageSource;
cv::Mat sourceImage;
cv::Mat imageBuffer;
aruco::CameraParameters cameraIntrinsics;
aruco::MarkerDetector markerDetector;

//Open image source
if(imageSource.open(videoDeviceNumber) != true)
{
fprintf(stderr, "Error, unable to open video device\n");
return 1;
}

//Could hardcode/set camera resolution
imageSource.set(CV_CAP_PROP_FRAME_WIDTH,1080);
imageSource.set(CV_CAP_PROP_FRAME_HEIGHT,720);

//Get first image (used for automatic dimension detection)
SOM_TRY
imageSource >> sourceImage;
SOM_CATCH("Error retrieving first image\n")

//Read camera parameters
SOM_TRY
cameraIntrinsics.readFromXMLFile(cameraIntrinsicsFilePath);
SOM_CATCH("Error, unable to read camera intrinsics from " + cameraIntrinsicsFilePath + "\n")

//Automatically resize the camera calibration object to match the current image
SOM_TRY
cameraIntrinsics.resize(sourceImage.size());
SOM_CATCH("Error resizing camera parameters\n") 

//Create display interface using opencv
cv::namedWindow("Display", 1);

SOM_TRY
cv::imshow("Display", sourceImage);
SOM_CATCH("Error showing image\n")


//Configure marker detector
markerDetector.setThresholdParams(HARRIS_THRESHOLD_PARAMETER_1, HARRIS_THRESHOLD_PARAMETER_2);
markerDetector.setThresholdParamRange(2,0); //Not sure what this does


//Process frames until something kills the process or an error occurs
std::vector<aruco::Marker> detectedMarkers;
while(true)
{
if(imageSource.grab() != true)
{
fprintf(stderr, "Error, unable to get image from video source\n");
return 1;
}

//Get image from camera
if(imageSource.retrieve(sourceImage) != true)
{
fprintf(stderr, "Error getting image\n");
return 1;
}

//Copy to buffer
sourceImage.copyTo(imageBuffer);

//See how probable that the given board is in the image
SOM_TRY
markerDetector.detect(sourceImage, detectedMarkers, cameraIntrinsics, boardSizeInMeters);
SOM_CATCH("Error searching for board in image\n")


//Draw indicators on all detected markers in the buffer
for(int i=0; i<detectedMarkers.size(); i++)
{
detectedMarkers[i].draw(imageBuffer, cv::Scalar(0,0,255), 1);
}

//Draw board axis on the image if detection probability is greater than theshold
for(int i=0; i<detectedMarkers.size(); i++)
{
aruco::CvDrawingUtils::draw3dAxis(imageBuffer, detectedMarkers[i], cameraIntrinsics);
}

//Print information of interest
double modelViewMatrix[16];
for(int i=0; i<detectedMarkers.size(); i++)
{ //Get modelview matrix
SOM_TRY
detectedMarkers[i].glGetModelViewMatrix(modelViewMatrix);
SOM_CATCH("Error getting model view matrix\n")

for(int a=0; a<4; a++)
{
for(int aa = 0; aa < 4; aa++)
{
printf("%lf ", modelViewMatrix[a*4+aa]);
}
printf("\n");
}
printf("\n");
}


//Display the image with labeled markers/axis
cv::imshow("Display", imageBuffer);

//Wait, enabling opencv to display the image
auto key = cv::waitKey(10); //Wait 10 milliseconds
}


return 0;
}
