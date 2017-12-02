/**
*/
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
 
int main ( int argc,char **argv ) 
{
	raspicam::RaspiCam Camera; //Camera object

	//Open camera 
	cout<<"Opening Camera..."<<endl;
	if ( !Camera.open()) 
    {
        cerr<<"Error opening camera"<<endl;return -1;
    }

	//wait a while until camera stabilizes
	cout<<"Sleeping for 3 secs"<<endl;
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
	    Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_RGB );//get camera image

        long imgBrightness = 0;
        for(int i=0; i<imgSize; i++)
        {
            imgBrightness += data[i];
        }

        cout << "Brightness: " << imgBrightness << endl;

        if(imgBrightness > 4000)
        {
	        //save
	        std::ofstream outFile("imgs/img"+imgCount+".ppm",std::ios::binary);
	        outFile << "P6\n" << Camera.getWidth() << " " << Camera.getHeight() << " 255\n";
	        outFile.write( ( char* ) data, Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB ) );
            imgCount++;
	        //cout<<"Image saved at raspicam_image.ppm"<<endl;
        }
    }

	//free resrources    
	delete data;

	return 0;
}
