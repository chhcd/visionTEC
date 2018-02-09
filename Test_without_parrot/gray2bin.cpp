#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

void color2gray(const Mat &sourceImage, Mat &grayImage);
void gray2bin(const Mat &sourceImage, Mat &binImage, uint8_t threshold);


int main(int argc, char *argv[])
{
	/* First, open camera device */
	VideoCapture camera = VideoCapture(0);
	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;
	Mat grayImage;
	Mat binImage;

	while (true)
	{
		/* Obtain a new frame from camera */
		camera.read(currentImage);
		grayImage = Mat::zeros(currentImage.size(),CV_8UC1);
		binImage = Mat::zeros(currentImage.size(),CV_8UC1);


		color2gray(currentImage,grayImage);
		gray2bin(grayImage,binImage,100);

		/* Show images */
		imshow("Original", currentImage);
		imshow("Grayed",grayImage);
		imshow("Binarized",binImage);

		/* If 'x' is pressed, exit program */
		if (waitKey(3) == 'x')
			break;
	}
}


void color2gray(const Mat &sourceImage, Mat &grayImage)
{
	int grayValue = 0;
	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols ; ++x){
			grayValue = 0;
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				grayValue += sourceImage.at<Vec3b>(y, x)[i] ;
			}
			grayImage.at<uint8_t>(y,x) = grayValue/3;
		}
		
	}
}

void gray2bin(const Mat &sourceImage, Mat &binImage, uint8_t threshold)
{
	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols ; ++x){
			binImage.at<uint8_t>(y,x) = (sourceImage.at<uint8_t>(y,x) < threshold) ? 255:0;
			
		}
		
	}	
}