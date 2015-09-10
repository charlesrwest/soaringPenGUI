#include<cstdio>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<string>
#include "SOMException.hpp"
#include<zmq.hpp>
#include<memory>


int main(int argc, char **argv)
{

if(argc < 2)
{
fprintf(stderr, "Error, incorrect number of arguments.  \nUsage: IPOfSource:portOfSource\n");
return 1;
}

cv::Mat sourceImage;


//Create display interface using opencv
cv::namedWindow("Display", 1);

//Create ZMQ context
std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing context\n")


//Setup ZMQ pub socket to share video with
std::unique_ptr<zmq::socket_t> videoSubscriber;

SOM_TRY //Initialize
videoSubscriber.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error initializing video subscribing socket\n")

SOM_TRY //Set filter to allow any published messages to be received
videoSubscriber->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription filter\n")

SOM_TRY //Connect
std::string bindingAddress = std::string("tcp://")+ argv[1];
videoSubscriber->connect(bindingAddress.c_str());
SOM_CATCH("Error connecting to video publisher\n")


//Get video from publisher, decode and share it
std::unique_ptr<zmq::message_t> messageBuffer;
while(true)
{
SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Get encoded image
if(videoSubscriber->recv(messageBuffer.get()) != true)
{
continue; //No message received
}
SOM_CATCH("Error receiving message\n")

std::vector<unsigned char> encodedImage((unsigned char *) messageBuffer->data(), ((unsigned char *) messageBuffer->data())+messageBuffer->size()); //Copy to vector

//Decompress/decode image for display
SOM_TRY
sourceImage = cv::imdecode(encodedImage, CV_LOAD_IMAGE_COLOR);
SOM_CATCH("Error decoding image\n")


//Display the image with labeled markers/axis
cv::imshow("Display", sourceImage);

//Wait, enabling opencv to display the image
auto key = cv::waitKey(10); //Wait 10 milliseconds
}


return 0;
}
