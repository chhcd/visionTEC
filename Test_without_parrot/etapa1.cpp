#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/* Developed routines*/
#include "rutinasVision.h"

using namespace std; 
using namespace cv;

Mat currentImage;
Mat flippedImage;
Mat grayImage;
Mat hsvImage;
Mat yiqImage;
Mat binImage;


int sliderBinValue;
int const SLIDER_MAX = 255;

void mCoordinatesComponentVal(int event, int x, int y, int flags, void* param)
{
	int Px,Py;
	int vR,vG,vB;
	int vH,vS,vV;
	int vY,vI,vQ;

    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            Px=x;
            Py=y;
            
            vR = currentImage.at<Vec3b>(y, x)[0];
            vG = currentImage.at<Vec3b>(y, x)[1];
            vB = currentImage.at<Vec3b>(y, x)[2];

            vH = hsvImage.at<Vec3b>(y, x)[0];
            vS = hsvImage.at<Vec3b>(y, x)[1];
            vV = hsvImage.at<Vec3b>(y, x)[2];

            // vY = yiqImage.at<Vec3b>(y, x)[0];
            // vI = yiqImage.at<Vec3b>(y, x)[1];
            // vQ = yiqImage.at<Vec3b>(y, x)[2];


            // Print Coordinates and Values
            cout <<"Pos X: "<<Px<<" Pos Y: "<<Py<<endl;
            cout <<"Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
            cout <<"Valor HSV: ("<<vH<<","<<vS<<","<<vV<<")"<<endl;
            //cout <<"Valor YIQ: ("<<vH<<","<<vS<<","<<vV<<")"<<endl;


            break;
        case CV_EVENT_MOUSEMOVE:
            break;
        case CV_EVENT_LBUTTONUP:
            break;
        case CV_EVENT_RBUTTONDOWN:
        //flag=!flag;
            break;
        
    }
	
}

void trackbarCallBack(int,void*)
{
	cout << sliderBinValue << endl;
}

int main(int argc, char *argv[])
{
	/* Open camera device */
	VideoCapture camera = VideoCapture(0);



	/* Show interface menu to user*/
	showMenu();

	/* Create Trackbar */
	namedWindow("Bin Value",1);
	Mat trackImage = Mat::zeros(Size(500,30),CV_8UC1);
	imshow("Bin Value",trackImage);
	createTrackbar( "Valor de binarizacion", "Bin Value", &sliderBinValue, SLIDER_MAX, trackbarCallBack );

	/* Set Mouse Call Back*/
	namedWindow("Click");
	setMouseCallback("Click", mCoordinatesComponentVal);


	bool freezeImage = false;
	bool bContinue = true;
	while (bContinue)
	{

		if(!freezeImage){
			/* Obtain a new frame from camera */
			camera.read(currentImage);
		}

  		/* Imit gray image based on image from camera*/
		grayImage = Mat::zeros(currentImage.size(),CV_8UC1);

    	/* Calling routines to convert color spaces*/
		color2gray(currentImage,grayImage);
		cvtColor(currentImage,hsvImage,CV_RGB2HSV);
		gray2threshold(grayImage,binImage,sliderBinValue);

		/* Show images */
		imshow("Click", currentImage);
		//imshow("Grayed",grayImage);
		imshow("HSV",hsvImage);
		//imshow("YIQ",yiqImage);
		imshow("Gray Binarizacion", binImage);


		uint8_t key = waitKey(3);
		switch (key)
		{
			/* Exit -> 'ESC' key */
			case 'x':
				bContinue = false;
				break;
			case 'p':
				freezeImage = !freezeImage;

		}

	}
}

void flipImageBasic(const Mat &sourceImage, Mat &destinationImage);
void color2gray(const Mat &sourceImage, Mat &grayImage);
void showMenu();
