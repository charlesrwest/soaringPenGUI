#include<cstdio>
#include "aruco.h"
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<string>
#include "SOMException.hpp"

//Mostly a reimplementation of the aruco_test_board example from the aruco library.
const double HARRIS_THRESHOLD_PARAMETER_1 = 7.0;
const double HARRIS_THRESHOLD_PARAMETER_2 = 7.0;
const double REPROJECTION_TEST_THRESHOLD = 1.5;
const double DETECTION_PROBABILITY_THRESHOLD = .5;

int main(int argc, char **argv)
{

if(argc < 5)
{
fprintf(stderr, "Error, incorrect number of arguments.  \nUsage: videoDeviceNumber boardToDetectConfig.yml cameraIntrinsics.yml sizeOfBoardInMeters (format: X:Y)\n");
return 1;
}

int videoDeviceNumber = 0;
SOM_TRY
videoDeviceNumber = std::stol(std::string(argv[1]));
SOM_CATCH("Error, unable to read video device number\n")

std::string boardToDetectConfigFile = argv[2];
std::string cameraIntrinsicsFilePath = argv[3];
double boardSizeInMeters = atof(argv[4]);
if(fabs(boardSizeInMeters) < .001)
{
fprintf(stderr, "Error, board size is invalid\n");
return 1;
} 

cv::VideoCapture imageSource;
cv::Mat sourceImage;
cv::Mat imageBuffer;
aruco::CameraParameters cameraIntrinsics;
aruco::BoardConfiguration boardToDetect;
aruco::BoardDetector boardDetector;

//Open image source
if(imageSource.open(videoDeviceNumber) != true)
{
fprintf(stderr, "Error, unable to open video device\n");
return 1;
}

//Could hardcode/set camera resolution
imageSource.set(CV_CAP_PROP_FRAME_WIDTH,1080);
imageSource.set(CV_CAP_PROP_FRAME_HEIGHT,720);

//Initialize profile of board to detect from given file
SOM_TRY
boardToDetect.readFromFile(boardToDetectConfigFile);
SOM_CATCH("Error reading board configuration from " + boardToDetectConfigFile + "\n")

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

//Configure board detector
boardDetector.setParams(boardToDetect, cameraIntrinsics, boardSizeInMeters);

//Set marker detector type and threshold values
boardDetector.getMarkerDetector().setThresholdParams(HARRIS_THRESHOLD_PARAMETER_1, HARRIS_THRESHOLD_PARAMETER_2);
boardDetector.getMarkerDetector().setCornerRefinementMethod(aruco::MarkerDetector::HARRIS);

//Set threshold for reprojection test
boardDetector.set_repj_err_thres(REPROJECTION_TEST_THRESHOLD); 

//Process frames until something kills the process or an error occurs
double detectionProbability = 0.0;
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
detectionProbability = boardDetector.detect(sourceImage);
SOM_CATCH("Error searching for board in image\n")

printf("Probability of detection: %lf\n", detectionProbability);

//Draw indicators on all detected markers in the buffer
for(int i=0; i<boardDetector.getDetectedMarkers().size(); i++)
{
boardDetector.getDetectedMarkers()[i].draw(imageBuffer, cv::Scalar(0,0,255), 1);
}

//Draw board axis on the image if detection probability is greater than theshold
if(detectionProbability > DETECTION_PROBABILITY_THRESHOLD)
{
aruco::CvDrawingUtils::draw3dAxis(imageBuffer, boardDetector.getDetectedBoard(), cameraIntrinsics);
}

//Display the image with labeled markers/axis
cv::imshow("Display", imageBuffer);

//Wait, enabling opencv to display the image
auto key = cv::waitKey(10); //Wait 10 milliseconds
}


return 0;
}
