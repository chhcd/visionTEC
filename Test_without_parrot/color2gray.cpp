#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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

		/* If 'x' is pressed, exit program */
		if (waitKey(3) == 'x')
			break;
	}
}


/*
 * This method flips horizontally the sourceImage into destinationImage. Because it uses 
 * "Mat::at" method, its performance is low (redundant memory access searching for pixels).
 */
void flipImageBasic(const Mat &sourceImage, Mat &destinationImage)
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

	for (int y = 0; y < sourceImage.rows; ++y)
		for (int x = 0; x < sourceImage.cols / 2; ++x)
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				destinationImage.at<Vec3b>(y, x)[i] = sourceImage.at<Vec3b>(y, sourceImage.cols - 1 - x)[i];
				destinationImage.at<Vec3b>(y, sourceImage.cols - 1 - x)[i] = sourceImage.at<Vec3b>(y, x)[i];
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