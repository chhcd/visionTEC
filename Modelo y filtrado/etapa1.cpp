#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	/* Open camera device */
	Mat originalImage = imread("puto.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat binImage;
    Mat filterImage;

    threshold( originalImage, binImage, 128, 255,0 );

    imshow("Original Image", binImage);

    Mat kernel = cv::getStructuringElement(MORPH_CROSS, Size(5, 5));

    cv::erode(binImage, filterImage, kernel);

    imshow("Gaussian Image", filterImage);
    imwrite("bin_puto.jpg", binImage);
    imwrite( "Erosion_puto.jpg", filterImage);
    waitKey(0);
}