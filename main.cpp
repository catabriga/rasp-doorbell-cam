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
#include <thread>

#define NUM_IMAGES 200

Magick::Image images[NUM_IMAGES];
std::string imageNames[NUM_IMAGES];
int currentImage = 0;

void imgWriter(void)
{
    int lastImage = 0;
    while(1)
    {
        if(lastImage != currentImage)
        {
            images[lastImage].write(imageNames[lastImage]);
            lastImage = (lastImage+1) % NUM_IMAGES;
            printf("%d\n", lastImage);
        }
        else
        {
            usleep(100000);
        }        
    }
}

int main(int argc, char **argv) 
{
    Magick::InitializeMagick(*argv);
    
    std::thread imgWriterThread(imgWriter);

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

    while(1)
    {
        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	    camera.grab();

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

	    camera.retrieve(data, raspicam::RASPICAM_FORMAT_IGNORE);
        int width = camera.getWidth();
        int height = camera.getHeight();

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        Magick::Image image(width, height, "BGR", MagickCore::StorageType::CharPixel, data);
        
        std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();

        long imgBrightness = 0;
        for(int i=0; i<imgSize; i++)
        {
            imgBrightness += data[i];
        }

        std::chrono::steady_clock::time_point t4 = std::chrono::steady_clock::now();

        if(imgBrightness > 20 * 1228800)
        {
            images[currentImage] = image;

            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
                        (now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d-%T");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            imageNames[currentImage] = "imgs/"+ss.str()+".jpg";
            currentImage = (currentImage+1) % NUM_IMAGES;

            std::chrono::steady_clock::time_point t5 = std::chrono::steady_clock::now();

            int dt1 = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            int dt2 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            int dt3 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
            int dt4 = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
            int dt5 = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count();
            printf("%d -> %d %d %d %d %d\n", currentImage, dt1, dt2, dt3, dt4, dt5);

            int dt = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t0).count();
            if(dt < 500000)
            {
                usleep(500000 - dt);	       
            }

        }
    }

    imgWriterThread.join();

	//free resrources    
	delete data;

	return 0;
}
