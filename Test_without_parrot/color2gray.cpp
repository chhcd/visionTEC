#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/* Developed routines*/
#include "rutinasVision.h"

using namespace std;
using namespace cv;

void flipImageBasic(const Mat &sourceImage, Mat &destinationImage);
void color2gray(const Mat &sourceImage, Mat &grayImage);


int main(int argc, char *argv[])
{
	/* First, open camera device */
	VideoCapture camera = VideoCapture(0);
	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;
	Mat flippedImage;
	Mat grayImage;

	while (true)
	{
		/* Obtain a new frame from camera */
		camera.read(currentImage);
		grayImage = Mat::zeros(currentImage.size(),CV_8UC1);


		/* Call custom flipping routine. From OpenCV, you could call flip(currentImage, flippedImage, 1) */
		flipImageBasic(currentImage, flippedImage);
		color2gray(currentImage,grayImage);

		/* Show images */
		imshow("Original", currentImage);
		imshow("Flipped", flippedImage);
		imshow("Grayed",grayImage);

		
		key = waitKey(3);
		/* If 'x' is pressed, exit program */
		if (waitKey(3) == 'x')
			break;
	}
}


