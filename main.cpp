/**
*/
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <fstream>
#include <raspicam/raspicam.h>
#include <cstdio>
#include <stdlib.h>
#include <Magick++.h>
#include <thread>
#include <queue>

#define BRIGHTNESS_THRESHOLD 0.2

std::queue<std::string> ppmImages;

void imgConverter(void)
{
    while(1)
    {
        if(!ppmImages.empty())
        {
            std::string imgName = ppmImages.front();
            ppmImages.pop();
            
            Magick::Image image(imgName+".ppm");
         
            image.rotate(90);
            image.font("Helvetica");
            image.fillColor(Magick::Color("white"));
            image.strokeColor(Magick::Color("black"));
            image.draw(Magick::DrawableText(0, 0, imgName));

            image.write(imgName+".jpg");
            remove((imgName+".ppm").c_str());
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
    
    std::thread imgConverterThread(imgConverter);

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
    int width = camera.getWidth();
    int height = camera.getHeight();            
 	        

    while(1)
    {
        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	    camera.grab();

	    camera.retrieve(data, raspicam::RASPICAM_FORMAT_IGNORE);
        
        long imgBrightness = 0;
        for(int i=0; i<imgSize; i++)
        {
            imgBrightness += data[i];
        }

        double brightRatio = ((double)imgBrightness) / (width * height * 255);
        printf("%f\n", brightRatio);
        if(brightRatio > BRIGHTNESS_THRESHOLD)
        {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
                        (now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d-%T");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            std::string imageName = "imgs/"+ss.str();       
            std::ofstream outFile(imageName+".ppm", std::ios::binary);

            outFile << "P6\n" << width << " " << height << " 255\n";
 	        outFile.write(  (char*)data, 
                            camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_BGR));            
            ppmImages.push(imageName);

            std::chrono::steady_clock::time_point t5 = std::chrono::steady_clock::now();

            int dt = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t0).count();
            if(dt < 500000)
            {
                usleep(500000 - dt);	       
            }

        }
    }

    imgConverterThread.join();

	//free resrources    
	delete data;

	return 0;
}
