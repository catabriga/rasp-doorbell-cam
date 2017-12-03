/**
*/
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <raspicam/raspicam.h>
#include <cstdio>
#include <stdlib.h>
#include <Magick++.h>
 
int main(int argc, char **argv) 
{
    Magick::InitializeMagick(*argv);
	raspicam::RaspiCam Camera; //Camera object

	//Open camera 
	printf("Opening Camera...\n");
	if(!Camera.open()) 
    {
        printf("Error opening camera\n");
        return -1;
    }

	//wait a while until camera stabilizes
	printf("Sleeping for 3 secs\n");
	usleep(3000000);

	//allocate memory
    int imgSize = Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB);
	unsigned char* data = new unsigned char[imgSize];

    int imgCount = 0;
    while(1)
    {
	    //capture
	    Camera.grab();

	    //extract the image in rgb format
	    Camera.retrieve(data, raspicam::RASPICAM_FORMAT_RGB );//get camera image
        int width = Camera.getWidth();
        int height = Camera.getHeight();

        Magick::Image image(width, height, "RGB", MagickCore::StorageType::CharPixel, data);

        long imgBrightness = 0;
        for(int i=0; i<imgSize; i++)
        {
            imgBrightness += data[i];
        }

        if(imgBrightness > 20 * 1228800)
        {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %X");
            image.write("imgs/"+ss.str()+".jpg");

	        imgCount++;
            usleep(500000);	       
        }
    }

	//free resrources    
	delete data;

	return 0;
}
