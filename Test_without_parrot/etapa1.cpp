/* Developed routines*/
#include "rutinasVision.h"

Mat bgrImage;
Mat bgrHistogram;
Mat hsvImage;
Mat hsvHistogram;
Mat yiqImage;
Mat yiqHistogram;
Mat grayImage;
Mat binImage;

int Px=0, Py=0;
int vR=0, vG=0, vB=0;
int vH=0, vS=0, vV=0;
int vY=0, vI=0, vQ=0;

int sliderBinValue;
int const SLIDER_MAX = 255;

void mCoordinatesComponentVal(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            Px=x;
            Py=y;
            
            vB = bgrImage.at<Vec3b>(y, x)[0];
            vG = bgrImage.at<Vec3b>(y, x)[1];
            vR = bgrImage.at<Vec3b>(y, x)[2];

            vH = hsvImage.at<Vec3b>(y, x)[0];
            vS = hsvImage.at<Vec3b>(y, x)[1];
            vV = hsvImage.at<Vec3b>(y, x)[2];

            vY = yiqImage.at<Vec3b>(y, x)[0];
            vI = yiqImage.at<Vec3b>(y, x)[1];
            vQ = yiqImage.at<Vec3b>(y, x)[2];

            // Print Coordinates and Values
            cout <<"Pos X: "<<Px<<" Pos Y: "<<Py<<endl;
            cout <<"Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
            cout <<"Valor HSV: ("<<vH<<","<<vS<<","<<vV<<")"<<endl;
            cout <<"Valor YIQ: ("<<vY<<","<<vI<<","<<vQ<<")"<<endl<<endl;

            break;
        case CV_EVENT_MOUSEMOVE:
            break;
        case CV_EVENT_LBUTTONUP:
            break;
        case CV_EVENT_RBUTTONDOWN:
            break;
    }
	
}

int main(int argc, char *argv[])
{
	/* Open camera device */
	VideoCapture camera = VideoCapture(0);

	/* Show interface menu to user*/
	Mat menu;
	menu.create(600,600,CV_8UC3);
	menu.setTo(Scalar(200,200,200));
	putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
	putText(menu,"p. Cambiar modo continuo / modo congelado", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	putText(menu,"r. Imagen RGB", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	putText(menu,"h. Imagen HSV", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	putText(menu,"y. Imagen YIQ", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	putText(menu,"b. Imagen gris / binarizacion", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	putText(menu,"x. Terminar", Point(15,290) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
	imshow("Menu", menu);

	bool freezeImage = false;
	bool bContinue = true;
	bool bEnRGB = false;
	bool bEnYIQ = false;
	bool bEnHSV = false;
	bool bEnBinarization = false;
	while (bContinue)
	{

		if(!freezeImage){
			/* Obtain a new frame from camera */
			camera.read(bgrImage);

	    	/* Calling routines to convert color spaces*/
			color2gray(bgrImage,grayImage);
			color2yiq(bgrImage,yiqImage);
			cvtColor(bgrImage,hsvImage,CV_BGR2HSV);
		}

		/* Show images */
		if (bEnRGB)
		{
			namedWindow("RGB");
			setMouseCallback("RGB", mCoordinatesComponentVal);
			imshow("RGB", bgrImage);
			imageHistogram(bgrImage, bgrHistogram);

			Mat overlay;
			bgrHistogram.copyTo(overlay);
			rectangle(overlay, Rect(2*vB,0,2,400), Scalar(255,0,0), -1);
			rectangle(overlay, Rect(2*vB,420,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vG,0,2,400), Scalar(0,255,0), -1);
			rectangle(overlay, Rect(2*vG,490,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vR,0,2,400), Scalar(0,0,255), -1);
			rectangle(overlay, Rect(2*vR,560,2,50), Scalar(255,255,255), -1);
			addWeighted(overlay, 0.7, bgrHistogram, 0.3, 0, bgrHistogram);

			namedWindow("RGB Histogram", CV_WINDOW_AUTOSIZE);
			imshow("RGB Histogram", bgrHistogram);
		}

		if (bEnHSV)
		{
			namedWindow("HSV");
			setMouseCallback("HSV", mCoordinatesComponentVal);
			imshow("HSV", hsvImage);
			imageHistogram(hsvImage, hsvHistogram);

			Mat overlay;
			hsvHistogram.copyTo(overlay);
			rectangle(overlay, Rect(2*vH,0,2,400), Scalar(255,0,0), -1);
			rectangle(overlay, Rect(2*vH,420,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vS,0,2,400), Scalar(0,255,0), -1);
			rectangle(overlay, Rect(2*vS,490,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vV,0,2,400), Scalar(0,0,255), -1);
			rectangle(overlay, Rect(2*vV,560,2,50), Scalar(255,255,255), -1);
			addWeighted(overlay, 0.7, hsvHistogram, 0.3, 0, hsvHistogram);

			namedWindow("HSV Histogram", CV_WINDOW_AUTOSIZE);
			imshow("HSV Histogram", hsvHistogram);
		}

		if (bEnYIQ)
		{
			namedWindow("YIQ");
			setMouseCallback("YIQ", mCoordinatesComponentVal);
			imshow("YIQ", yiqImage);
			imageHistogram(yiqImage, yiqHistogram);

			Mat overlay;
			yiqHistogram.copyTo(overlay);
			rectangle(overlay, Rect(2*vY,0,2,400), Scalar(255,0,0), -1);
			rectangle(overlay, Rect(2*vY,420,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vI,0,2,400), Scalar(0,255,0), -1);
			rectangle(overlay, Rect(2*vI,490,2,50), Scalar(255,255,255), -1);
			rectangle(overlay, Rect(2*vQ,0,2,400), Scalar(0,0,255), -1);
			rectangle(overlay, Rect(2*vQ,560,2,50), Scalar(255,255,255), -1);
			addWeighted(overlay, 0.7, yiqHistogram, 0.3, 0, yiqHistogram);

			namedWindow("YIQ Histogram", CV_WINDOW_AUTOSIZE);
			imshow("YIQ Histogram", yiqHistogram);
		}

		if (bEnBinarization)
		{
			/* Create Trackbar */
			gray2threshold(grayImage,binImage,sliderBinValue);
			namedWindow("Gray Binarization",1);
			Mat trackImage = Mat::zeros(Size(500,30),CV_8UC1);
			imshow("Gray Binarization",trackImage);
			createTrackbar( "Valor de binarizacion", "Gray Binarization", &sliderBinValue, SLIDER_MAX, NULL );
			imshow("Gray", grayImage);
			imshow("Gray Binarization", binImage);
		}

		uint8_t key = waitKey(3);
		switch (key)
		{
			/* Exit -> 'ESC' key */
			case 'x':
			case 27:
				cout << "Terminating program" << endl;
				bContinue = false;
				break;
			case 'p':
				freezeImage = !freezeImage;
				break;
			case 'r':
				if(bEnRGB){ destroyWindow("RGB"); destroyWindow("RGB Histogram"); }
				bEnRGB = !bEnRGB;
				break;
			case 'h':
				if(bEnHSV){ destroyWindow("HSV"); destroyWindow("HSV Histogram"); }
				bEnHSV = !bEnHSV;
				break;
			case 'y':
				if(bEnYIQ){ destroyWindow("YIQ"); destroyWindow("YIQ Histogram"); }
				bEnYIQ = !bEnYIQ;
				break;
			case 'b':
				if(bEnBinarization){ destroyWindow("Gray"); destroyWindow("Gray Binarization"); }
				bEnBinarization = !bEnBinarization;
				break;
		}
	}
}