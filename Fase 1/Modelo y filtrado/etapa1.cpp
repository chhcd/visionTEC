#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	/* Open camera device */
	Mat originalImage = imread("Foto.jpg", CV_LOAD_IMAGE_GRAYSCALE);

    imshow("Original Image", originalImage);

    float data[25] = { 1, 2, 2, 2, 1,
                       2, 4, 4, 4, 2,
                       2, 4, 8, 4, 2,
                       2, 4, 4, 4, 2,
                       1, 2, 2, 2, 1};

    float normalize_factor = 0;
    for(int i=0; i<25; i++)
        normalize_factor += data[i];

    Mat kernel = Mat(5, 5, CV_32F, data);

    Mat filterImage;

    filter2D(originalImage, filterImage, -1 , kernel / normalize_factor, Point(-1,-1), 0, BORDER_DEFAULT);

    imshow("Gaussian Image", filterImage);
    imwrite( "gaussian_image.jpg", filterImage);
    waitKey(0);
}