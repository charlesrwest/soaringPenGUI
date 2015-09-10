#include<cstdio>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<string>
#include "SOMException.hpp"
#include<zmq.hpp>
#include<memory>


int main(int argc, char **argv)
{

if(argc < 3)
{
fprintf(stderr, "Error, incorrect number of arguments.  \nUsage: videoDeviceNumber portNumberToBind\n");
return 1;
}

int videoDeviceNumber = 0;
SOM_TRY
videoDeviceNumber = std::stol(std::string(argv[1]));
SOM_CATCH("Error, unable to read video device number\n")

int portNumberToBind = 0;
SOM_TRY
portNumberToBind = std::stol(std::string(argv[2]));
SOM_CATCH("Error, unable to read portNumberToBind\n")

cv::VideoCapture imageSource;
cv::Mat sourceImage;

//Open image source
if(imageSource.open(videoDeviceNumber) != true)
{
fprintf(stderr, "Error, unable to open video device\n");
return 1;
}

//Could hardcode/set camera resolution
//imageSource.set(CV_CAP_PROP_FRAME_WIDTH,1080);
//imageSource.set(CV_CAP_PROP_FRAME_HEIGHT,720);

imageSource.set(CV_CAP_PROP_FRAME_WIDTH,1280);
imageSource.set(CV_CAP_PROP_FRAME_HEIGHT,720);


//Get first image (used for automatic dimension detection)
SOM_TRY
imageSource >> sourceImage;
SOM_CATCH("Error retrieving first image\n")

//Create display interface using opencv
cv::namedWindow("Display", 1);

SOM_TRY
cv::imshow("Display", sourceImage);
SOM_CATCH("Error showing image\n")

//Create ZMQ context
std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing context\n")


//Setup ZMQ pub socket to share video with
std::unique_ptr<zmq::socket_t> videoPublisher;

SOM_TRY //Initialize
videoPublisher.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error initializing video sharing socket\n")

SOM_TRY //Bind
std::string bindingAddress = "tcp://*:"+ std::to_string(portNumberToBind);
videoPublisher->bind(bindingAddress.c_str());
SOM_CATCH("Error binding video publisher\n")


//Display video and share it
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

//Compress/encode it for transmission
std::vector<unsigned char> encodedImage;
std::vector<int> options; //Pairs of format type:value

//Set jpg quality
options.push_back(CV_IMWRITE_JPEG_QUALITY);
options.push_back(95);

if(cv::imencode(".jpg", sourceImage, encodedImage, options) != true)
{
fprintf(stderr, "Error encoding image\n");
return 1;
}

//Publish image
SOM_TRY
videoPublisher->send(encodedImage.data(), encodedImage.size());
SOM_CATCH("Error publishing image\n");

//Display the image with labeled markers/axis
cv::imshow("Display", sourceImage);

//Wait, enabling opencv to display the image
auto key = cv::waitKey(10); //Wait 10 milliseconds
}


return 0;
}
