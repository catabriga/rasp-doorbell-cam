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
	raspicam::RaspiCam camera; //Camera object

    camera.setFormat(raspicam::RASPICAM_FORMAT_BGR);

	//Open camera 
	printf("Opening Camera...\n");
	if(!camera.open()) 
    {
        printf("Error opening camera\n");
        return -1;
    }

	//wait a while until camera stabilizes
	printf("Sleeping for 3 secs\n");
	usleep(3000000);

	//allocate memory
    int imgSize = camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_BGR);
	unsigned char* data = new unsigned char[imgSize];

    int imgCount = 0;
    while(1)
    {
	    camera.grab();

	    camera.retrieve(data, raspicam::RASPICAM_FORMAT_IGNORE);
        int width = camera.getWidth();
        int height = camera.getHeight();

        Magick::Image image(width, height, "BGR", MagickCore::StorageType::CharPixel, data);

        long imgBrightness = 0;
        for(int i=0; i<imgSize; i++)
        {
            imgBrightness += data[i];
        }

        if(imgBrightness > 20 * 1228800)
        {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
                        (now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d-%T");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            image.write("imgs/"+ss.str()+".jpg");

	        imgCount++;
            usleep(500000);	       
        }
    }

	//free resrources    
	delete data;

	return 0;
}
