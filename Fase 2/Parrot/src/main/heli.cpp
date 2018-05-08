#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SDL/SDL.h"

#include <stdlib.h>
#include "CHeli.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

/* Developed routines*/
#include "rutinasVision.h"

using namespace std;
using namespace cv;

#define PI 3.14159265

/* Declaring all the image matrices */
Mat bgrImage = Mat(240, 320, CV_8UC3);
Mat bHistogram;
Mat gHistogram;
Mat rHistogram;
Mat bgrFilter;
Mat hsvImage;
Mat hHistogram;
Mat sHistogram;
Mat vHistogram;
Mat hsvFilter;
Mat yiqImage;
Mat yHistogram;
Mat iHistogram;
Mat qHistogram;
Mat yiqFilter;
Mat grayImage;
Mat binImage;
Mat snapedMat;
Mat graph;

bool bSnap = false;
bool firstClick = true;
int rObjPoints[] = {0,0,0,0};

/* Setting the pixel coordinates and values in the different color models */
int Px=0, Py=0;
int vB=0, vG=0, vR=0;
int vH=0, vS=0, vV=0;
int vY=0, vI=0, vQ=0;

/* Setting the color mode range for the filter
   It's divided by channel 0 min, channel 0 max, channel 1 min, channel 1 max, channel 2 min, channel 2 max */
int rBGR[] = {0, 255, 0, 255, 0, 255};
int rHSV[] = {0, 255, 0, 255, 0, 255};
int rYIQ[] = {0, 255, 0, 255, 0, 255};

// needed outside
bool bEnRGB = false;
bool bEnYIQ = false;
bool bEnHSV = false;
bool freezeImage = false;
bool bEnBinarization = false;
bool bContinue = true;
bool bCalibrationMode = false;
bool bObjectDetectionMode = false;
bool bBasicBin = false;
bool bSegment = false;
bool bManualMovement = true;

/* Slider values for binarization */
int sliderBinValue=0;
int const SLIDER_MAX = 255;


CRawImage *image;
CHeli *heli;
float pitch, roll, yaw, height;
int hover=0;
// Joystick related
SDL_Joystick* m_joystick;
bool useJoystick;
int joypadRoll, joypadPitch, joypadVerticalSpeed, joypadYaw;
bool navigatedWithJoystick, joypadTakeOff, joypadLand, joypadHover, joypadAutonomous;
string ultimo = "init";


Mat imagenClick;

// Convert CRawImage to Mat
void rawToMat( Mat &destImage, CRawImage* sourceImage)
{   
    uchar *pointerImage = destImage.ptr(0);
    
    for (int i = 0; i < 240*320; i++)
    {
        pointerImage[3*i] = sourceImage->data[3*i+2];
        pointerImage[3*i+1] = sourceImage->data[3*i+1];
        pointerImage[3*i+2] = sourceImage->data[3*i];
    }
}


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

void mSnapObj(int event, int x, int y, int flags, void* param)
{
    int iDelta = 40;
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            if(firstClick)
            {
                
                line(snapedMat, Point(x,y), Point(x,y+20), CV_RGB(255,0,255),1,8,0);
                line(snapedMat, Point(x,y), Point(x+20,y), CV_RGB(0,255,255),1,8,0);
                imshow("Snaped",snapedMat);
                firstClick = false;
                rObjPoints[0] = x;
                rObjPoints[1] = y;
                // Print Coordinates and Values
                waitKey(0);
            }
            else
            {
                line(snapedMat, Point(x,y), Point(x,y-20), CV_RGB(255,0,255),1,8,0);
                line(snapedMat, Point(x,y), Point(x-20,y), CV_RGB(0,255,255),1,8,0);
                imshow("Snaped",snapedMat);
                rObjPoints[2] = x;
                rObjPoints[3] = y;

                int av0,av1,av2;
                regionAvg(bgrImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                cout <<"Valor PROMEDIO RGB: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                rBGR[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                rBGR[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                rBGR[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                rBGR[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                rBGR[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                rBGR[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;

                if(bEnHSV){
                    regionAvg(hsvImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                    cout <<"Valor PROMEDIO HSV: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                    rHSV[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                    rHSV[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                    rHSV[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                    rHSV[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                    rHSV[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                    rHSV[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;
                }
                if(bEnYIQ){
                    regionAvg(yiqImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                    cout <<"Valor PROMEDIO YIQ: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                    rYIQ[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                    rYIQ[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                    rYIQ[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                    rYIQ[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                    rYIQ[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                    rYIQ[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;
                }

                firstClick = true;
                waitKey(0);
            }
            break;
    }

}

void calibrationMode(uint8_t key)
{
    if(!freezeImage){
        //image is captured
        // heli->renewImage(image);

        // Copy to OpenCV Mat
        // rawToMat(bgrImage, image);


        //// TODO CHANGE
        bgrImage = imread("../fotosVision/muestra1.jpg", CV_LOAD_IMAGE_COLOR);

        /* Obtain a new frame from camera */
        //camera.read(bgrImage);

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

        imageHistogram(bgrImage, bHistogram, "RGB", 0);
        Mat overlayB;
        bHistogram.copyTo(overlayB);
        rectangle(overlayB, Rect(2*vB,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayB, Rect(2*vB,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayB, 0.7, bHistogram, 0.3, 0, bHistogram);
        namedWindow("Blue Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Blue Histogram", bHistogram);

        imageHistogram(bgrImage, gHistogram, "RGB", 1);
        Mat overlayG;
        gHistogram.copyTo(overlayG);
        rectangle(overlayG, Rect(2*vG,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayG, Rect(2*vG,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayG, 0.7, gHistogram, 0.3, 0, gHistogram);
        namedWindow("Green Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Green Histogram", gHistogram);

        imageHistogram(bgrImage, rHistogram, "RGB", 2);
        Mat overlayR;
        rHistogram.copyTo(overlayR);
        rectangle(overlayR, Rect(2*vR,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayR, Rect(2*vR,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayR, 0.7, rHistogram, 0.3, 0, rHistogram);
        namedWindow("Red Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Red Histogram", rHistogram);

        namedWindow("RGB Filter");
        colorFilter(bgrImage, bgrFilter, rBGR);
        createTrackbar( "B Min Value", "RGB Filter", &rBGR[0], SLIDER_MAX, NULL);
        createTrackbar( "B Max Value", "RGB Filter", &rBGR[1], SLIDER_MAX, NULL);
        createTrackbar( "G Min Value", "RGB Filter", &rBGR[2], SLIDER_MAX, NULL);
        createTrackbar( "G Max Value", "RGB Filter", &rBGR[3], SLIDER_MAX, NULL);
        createTrackbar( "R Min Value", "RGB Filter", &rBGR[4], SLIDER_MAX, NULL);
        createTrackbar( "R Max Value", "RGB Filter", &rBGR[5], SLIDER_MAX, NULL);
        imshow("RGB Filter", bgrFilter);
    }

    if (bEnHSV)
    {
        namedWindow("HSV");
        setMouseCallback("HSV", mCoordinatesComponentVal);
        imshow("HSV", hsvImage);

        imageHistogram(hsvImage, hHistogram, "HSV", 0);
        Mat overlayH;
        hHistogram.copyTo(overlayH);
        rectangle(overlayH, Rect(2*vH,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayH, Rect(2*vH,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayH, 0.7, hHistogram, 0.3, 0, hHistogram);
        namedWindow("Hue Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Hue Histogram", hHistogram);

        imageHistogram(hsvImage, sHistogram, "HSV", 1);
        Mat overlayS;
        sHistogram.copyTo(overlayS);
        rectangle(overlayS, Rect(2*vS,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayS, Rect(2*vS,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayS, 0.7, sHistogram, 0.3, 0, sHistogram);
        namedWindow("Saturation Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Saturation Histogram", sHistogram);

        imageHistogram(hsvImage, vHistogram, "HSV", 2);
        Mat overlayV;
        vHistogram.copyTo(overlayV);
        rectangle(overlayV, Rect(2*vV,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayV, Rect(2*vV,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayV, 0.7, vHistogram, 0.3, 0, vHistogram);
        namedWindow("Value Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Value Histogram", vHistogram);

        namedWindow("HSV Filter");
        colorFilter(hsvImage, hsvFilter, rHSV);
        createTrackbar( "H Min Value", "HSV Filter", &rHSV[0], SLIDER_MAX, NULL);
        createTrackbar( "H Max Value", "HSV Filter", &rHSV[1], SLIDER_MAX, NULL);
        createTrackbar( "S Min Value", "HSV Filter", &rHSV[2], SLIDER_MAX, NULL);
        createTrackbar( "S Max Value", "HSV Filter", &rHSV[3], SLIDER_MAX, NULL);
        createTrackbar( "V Min Value", "HSV Filter", &rHSV[4], SLIDER_MAX, NULL);
        createTrackbar( "V Max Value", "HSV Filter", &rHSV[5], SLIDER_MAX, NULL);
        imshow("HSV Filter", hsvFilter);
    }

    if (bEnYIQ)
    {
        namedWindow("YIQ");
        setMouseCallback("YIQ", mCoordinatesComponentVal);
        imshow("YIQ", yiqImage);

        imageHistogram(yiqImage, yHistogram, "YIQ", 0);
        Mat overlayY;
        yHistogram.copyTo(overlayY);
        rectangle(overlayY, Rect(2*vY,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayY, Rect(2*vY,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayY, 0.7, yHistogram, 0.3, 0, yHistogram);
        namedWindow("Luma Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Luma Histogram", yHistogram);

        imageHistogram(yiqImage, iHistogram, "YIQ", 1);
        Mat overlayI;
        iHistogram.copyTo(overlayI);
        rectangle(overlayI, Rect(2*vI,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayI, Rect(2*vI,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayI, 0.7, iHistogram, 0.3, 0, iHistogram);
        namedWindow("In-phase Histogram", CV_WINDOW_AUTOSIZE);
        imshow("In-phase Histogram", iHistogram);

        imageHistogram(yiqImage, qHistogram, "YIQ", 2);
        Mat overlayQ;
        qHistogram.copyTo(overlayQ);
        rectangle(overlayQ, Rect(2*vQ,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayQ, Rect(2*vQ,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayQ, 0.7, qHistogram, 0.3, 0, qHistogram);
        namedWindow("Quadrature Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Quadrature Histogram", qHistogram);

        namedWindow("YIQ Filter");
        colorFilter(yiqImage, yiqFilter, rYIQ);
        createTrackbar( "Y Min Value", "YIQ Filter", &rYIQ[0], SLIDER_MAX, NULL);
        createTrackbar( "Y Max Value", "YIQ Filter", &rYIQ[1], SLIDER_MAX, NULL);
        createTrackbar( "I Min Value", "YIQ Filter", &rYIQ[2], SLIDER_MAX, NULL);
        createTrackbar( "I Max Value", "YIQ Filter", &rYIQ[3], SLIDER_MAX, NULL);
        createTrackbar( "Q Min Value", "YIQ Filter", &rYIQ[4], SLIDER_MAX, NULL);
        createTrackbar( "Q Max Value", "YIQ Filter", &rYIQ[5], SLIDER_MAX, NULL);
        imshow("YIQ Filter", yiqFilter);
    }

    if (bEnBinarization)
    {
        /* Create Trackbar */
        gray2threshold(grayImage,binImage,sliderBinValue);
        namedWindow("Gray Binarization",1);
        createTrackbar( "Binatization threshold", "Gray Binarization", &sliderBinValue, SLIDER_MAX, NULL);
        imshow("Gray", grayImage);
        imshow("Gray Binarization", binImage);
    }

    if(bSnap)
    {   

        colorFilter(bgrImage,snapedMat,rBGR);
        namedWindow("Snaped");
        setMouseCallback("Snaped",mSnapObj);
        imshow("Snaped",snapedMat);
    }

    switch (key)
    {
        case 'p':
            freezeImage = !freezeImage;
            break;
        case 'r':
            if(bEnRGB){ destroyWindow("RGB"); destroyWindow("Blue Histogram"); destroyWindow("Green Histogram"); destroyWindow("Red Histogram"); destroyWindow("RGB Filter"); }
            bEnRGB = !bEnRGB;
            break;
        case 'h':
            if(bEnHSV){ destroyWindow("HSV"); destroyWindow("Hue Histogram"); destroyWindow("Saturation Histogram"); destroyWindow("Value Histogram"); destroyWindow("HSV Filter"); }
            bEnHSV = !bEnHSV;
            break;
        case 'y':
            if(bEnYIQ){ destroyWindow("YIQ"); destroyWindow("Luma Histogram"); destroyWindow("In-phase Histogram"); destroyWindow("Quadrature Histogram"); destroyWindow("YIQ Filter"); }
            bEnYIQ = !bEnYIQ;
            break;
        case 'b':
            if(bEnBinarization){ destroyWindow("Gray"); destroyWindow("Gray Binarization"); }
            bEnBinarization = !bEnBinarization;
            break;

        case 'f':
            if(bSnap){
                destroyWindow("Snaped"); 
                // rBGR[0]=0 ; rBGR[1]=255; rBGR[2] = 0; rBGR[3] = 255; rBGR[4] = 0; rBGR[5] = 255;
                // rHSV[0]=0 ; rHSV[1]=255; rHSV[2] = 0; rHSV[3] = 255; rHSV[4] = 0; rHSV[5] = 255;
                // rYIQ[0]=0     ; rYIQ[1]=255; rYIQ[2] = 0; rYIQ[3] = 255; rYIQ[4] = 0; rYIQ[5] = 255;
            }
            bSnap = !bSnap;
            break;
    }
}
int mapC(double x, double in_min, double in_max, double out_min, double out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void showRegionGraph(double phi1, double phi2){
    
    int delta = 10;

   

    // create axis lines
    // Y
    line(graph, Point(20,20), Point(20,685), CV_RGB(0,255,255),1,8,0);
    // X
    line(graph, Point(20,685), Point(985,685), CV_RGB(0,255,255),1,8,0);

    // Phi 1
    putText(graph,"PHI 2", Point(25,25) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    putText(graph,"PHI 1", Point(925,675) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );


    // X MAX  = 1.5 = 1000
    // Y MAX  = 2.0 = 700
    double x_max = 1.5;
    double y_max = 2.0;
    double x_min = 0.0;
    double y_min = 0.0;
    double mat_x_max = 984;
    double mat_y_max = 30;

    double mat_x_min = 30;
    double mat_y_min = 675;


    double fig1X = 0.38;
    double fig1Y = 0.07;

    int fig1X_coord = mapC(fig1X,x_min,x_max,mat_x_min,mat_x_max);
    int fig1Y_coord = mapC(fig1Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Golfista", Point(fig1X_coord - delta,fig1Y_coord - delta - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig1X_coord - delta, fig1Y_coord - delta),Point(fig1X_coord + delta, fig1Y_coord + delta),Scalar(255,0,0),2);

    double fig2X = 0.16;
    double fig2Y = 0.001;

    int fig2X_coord = mapC(fig2X,x_min,x_max,mat_x_min,mat_x_max);
    int fig2Y_coord = mapC(fig2Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Pelota", Point(fig2X_coord - delta,fig2Y_coord - delta - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig2X_coord - delta, fig2Y_coord - delta),Point(fig2X_coord + delta, fig2Y_coord + delta),Scalar(255,0,0),2);

    double fig3X = 0.2;
    double fig3Y = 0.005;

    int fig3X_coord = mapC(fig3X,x_min,x_max,mat_x_min,mat_x_max);
    int fig3Y_coord = mapC(fig3Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Palo ancho", Point(fig3X_coord - delta,fig3Y_coord - delta - 20) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig3X_coord - delta, fig3Y_coord - delta),Point(fig3X_coord + delta, fig3Y_coord + delta),Scalar(255,0,0),2);



    double fig4X = 1.36;
    double fig4Y = 1.80;

    int fig4X_coord = mapC(fig4X,x_min,x_max,mat_x_min,mat_x_max);
    int fig4Y_coord = mapC(fig4Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Baston Basic", Point(fig4X_coord - delta*2,fig4Y_coord - delta*2 - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig4X_coord - delta*6, fig4Y_coord - delta*6),Point(fig4X_coord + delta*6, fig4Y_coord + delta*6),Scalar(255,0,0),2);



    // circle(graph, Point(fig1X_coord,fig1Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig2X_coord,fig2Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig3X_coord,fig3Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig4X_coord,fig4Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
  
    // plot new point

    int newFigX = mapC(phi1,x_min,x_max,mat_x_min,mat_x_max);
    int newFigY = mapC(phi2,y_min,y_max,mat_y_min,mat_y_max);

    circle(graph, Point(newFigX,newFigY), 2 ,Scalar(0,0,255),CV_FILLED,8,0);

    // imshow("Grafica de regiones", graph);
}


void objectDetectionMode(uint8_t key)
{
    if(bManualMovement)
    {
        //// TODO TODO
        //image is captured
        // heli->renewImage(image);

        // Copy to OpenCV Mat
        // rawToMat(bgrImage, image);

        imshow("Current image", bgrImage);

        switch (key)
        {
            // Parrot commands
            case 'a': yaw = -2000.0; break;
            case 'd': yaw = 2000.0; break;
            case 'w': height = -2000.0; break;
            case 's': height = 2000.0; break;
            case 't': heli->takeoff(); break;
            case 'e': heli->land(); break;
            case 'z': heli->switchCamera(0); break;
            case 'x': heli->switchCamera(1); break;
            case 'c': heli->switchCamera(2); break;
            case 'v': heli->switchCamera(3); break;
            case 'j': roll = -2000.0; break;
            case 'l': roll = 2000.0; break;
            case 'i': pitch = -2000.0; break;
            case 'k': pitch = 2000.0; break;
            case 'o': hover = (hover + 1) % 2; break;
            case 'm': bManualMovement = false; break;
            case 'b':
                if(bBasicBin){ destroyWindow("Filtro"); destroyWindow("Binarizado"); }
                    bBasicBin = !bBasicBin;
                break;
            case 'f':
                if(bSegment){ destroyWindow("Segmentacion"); }
                    bSegment = !bSegment;
                break;
        }

        if(bBasicBin)
        {
            Mat bgrFilter;
            Mat binImage;

            colorFilter(bgrImage,bgrFilter,rBGR);
            cvtColor(bgrFilter, bgrFilter, CV_BGR2GRAY);

            imshow("Filtro", bgrFilter);
            
            gray2threshold(bgrFilter,binImage,80);
            imshow("Binarizado",binImage);
        }

        if(bSegment)
        {
            Mat bgrFilter;
            Mat binImage;
            Mat colormat;

            colorFilter(bgrImage,bgrFilter,rBGR);
            cvtColor(bgrFilter, bgrFilter, CV_BGR2GRAY);
            gray2threshold(bgrFilter,binImage,80);
        
            vector<vector<Point> > vp;
            // each region is a vector of Point
            vp = mycontours(binImage,300,colormat);

            // vector to store moments
            vector<rMoments> vMoments;

            printf("Number of regions: %lu \n\n", vp.size()); 

            int recognizedObjects = 0;

            graph.create(700,1000, CV_8UC3);
            graph.setTo(Scalar(30,30,30));

            for (int i = 0; i< vp.size(); i++){
                vMoments.push_back(computeMoments(vp[i]));

                printf("m00: %Lf  m10:  %Lf  m01: %Lf \n", vMoments[i].m00,vMoments[i].m10,vMoments[i].m01);
                printf("u10: %Lf  u01:  %Lf  u11: %Lf  u20: %Lf  u02: %Lf\n",vMoments[i].u10,vMoments[i].u01,vMoments[i].u11,vMoments[i].u20,vMoments[i].u02 );
                printf("phi1: %Lf  phi2: %Lf  \n", vMoments[i].phi1, vMoments[i].phi2);

                int x2 = vMoments[i].m10/vMoments[i].m00 + 100* cos(vMoments[i].theta);
                int y2 = vMoments[i].m01/vMoments[i].m00 + 100* sin(vMoments[i].theta); 
                int x3 = vMoments[i].m10/vMoments[i].m00 - 100* cos(vMoments[i].theta);
                int y3 = vMoments[i].m01/vMoments[i].m00 - 100* sin(vMoments[i].theta); 
                
                line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x2,y2), Scalar(0,255,0),2 );
                line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x3,y3), Scalar(0,255,0),2 );

                printf("Theta: %Lf \n\n",vMoments[i].theta); 
                circle(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), 5 ,Scalar(0,0,255),CV_FILLED,8,0);


                // Draw in graph
                showRegionGraph(vMoments[i].phi1, vMoments[i].phi2);

                // Pelota de golf (Adelante)
                if(vMoments[i].phi1 >= 0.15 && vMoments[i].phi1 <= 0.17 && vMoments[i].phi2 >= 0 && vMoments[i].phi2 <= 0.0005)
                {
                    printf("Pelota de golf (Adelante)\n");
                    recognizedObjects++;
                }
                // Palo de golf ancho (Atras)
                else if(vMoments[i].phi1 >= 0.19 && vMoments[i].phi1 <= 0.21 && vMoments[i].phi2 >= 0.003 && vMoments[i].phi2 <= 0.01)
                {
                    printf("Palo de golf ancho (Atras)\n");
                    recognizedObjects++;
                }
                // Golfista (derecha)
                else if(vMoments[i].phi1 >= 0.36 && vMoments[i].phi1 <= 0.45 && vMoments[i].phi2 >= 0.04 && vMoments[i].phi2 <= 0.1)
                {
                    printf("Golfista (derecha) con theta %d\n", vMoments[i].theta);
                    recognizedObjects++;
                }
                // Palo de golf delgado (izquierda)
                else if(vMoments[i].phi1 >= 0.9 && vMoments[i].phi2 >= 0.9)
                {
                    printf("Palo de golf delgado (izquierda) con theta %d\n", vMoments[i].theta);
                    recognizedObjects++;
                }
            }
            printf("Fuera del for\n");
            imshow("Segmentacion",colormat);
            imshow("Grafica de regiones", graph);
            waitKey(100);
            printf("Recognized objects: %d\n", recognizedObjects);
        }

        if (useJoystick)
        {
            SDL_Event event;
            SDL_PollEvent(&event);

            joypadRoll = SDL_JoystickGetAxis(m_joystick, 2);
            joypadPitch = SDL_JoystickGetAxis(m_joystick, 3);
            joypadVerticalSpeed = SDL_JoystickGetAxis(m_joystick, 1);
            joypadYaw = SDL_JoystickGetAxis(m_joystick, 0);
            joypadTakeOff = SDL_JoystickGetButton(m_joystick, 1);
            joypadLand = SDL_JoystickGetButton(m_joystick, 2);
            joypadHover = SDL_JoystickGetButton(m_joystick, 0);
            joypadAutonomous = SDL_JoystickGetButton(m_joystick,3);
        }

        // prints the drone telemetric data, helidata struct contains drone angles, speeds and battery status
        printf("===================== Parrot Basic Example =====================\n\n");
        fprintf(stdout, "Angles  : %.2lf %.2lf %.2lf \n", helidata.phi, helidata.psi, helidata.theta);
        fprintf(stdout, "Speeds  : %.2lf %.2lf %.2lf \n", helidata.vx, helidata.vy, helidata.vz);
        fprintf(stdout, "Battery : %.0lf \n", helidata.battery);
        fprintf(stdout, "Hover   : %d \n", hover);
        fprintf(stdout, "Joypad  : %d \n", useJoystick ? 1 : 0);
        fprintf(stdout, "  Roll    : %d \n", joypadRoll);
        fprintf(stdout, "  Pitch   : %d \n", joypadPitch);
        fprintf(stdout, "  Yaw     : %d \n", joypadYaw);
        fprintf(stdout, "  V.S.    : %d \n", joypadVerticalSpeed);
        fprintf(stdout, "  TakeOff : %d \n", joypadTakeOff);
        fprintf(stdout, "  Land    : %d \n", joypadLand);
        fprintf(stdout, "Navigating with Joystick: %d \n", navigatedWithJoystick ? 1 : 0);
        cout<<"Pos X: "<<Px<<" Pos Y: "<<Py<<" Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
        

        if (joypadTakeOff) {
            heli->takeoff();
        }
        if (joypadLand) {
            heli->land();
        }

        bManualMovement = joypadAutonomous ? false : bManualMovement;
        
        hover = joypadHover ? !hover : hover;

        //setting the drone angles
        if (joypadRoll != 0 || joypadPitch != 0 || joypadVerticalSpeed != 0 || joypadYaw != 0)
        {
            heli->setAngles(joypadPitch/10, joypadRoll/5, joypadYaw/3, joypadVerticalSpeed/2, hover);
            navigatedWithJoystick = true;
        }
        else
        {
            heli->setAngles(pitch, roll, yaw, height, hover);
            navigatedWithJoystick = false;
        } 
    }
    else
    {
        Mat frame;
        Mat img;
        Mat binImage;
        Mat colormat;
        Mat yiq;
        Mat yiqFilter;

        //// TODO CHANGE
        frame = imread("../fotosVision/muestra1.jpg", CV_LOAD_IMAGE_COLOR);

        colorFilter(bgrImage,yiqFilter,rBGR);
        cvtColor(yiqFilter, yiqFilter, CV_BGR2GRAY);

        // imshow("img", yiqFilter);
        
        gray2threshold(yiqFilter,binImage,80);
        // imshow("bin",binImage);
        
        vector<vector<Point> > vp;
        // each region is a vector of Point
        vp = mycontours(binImage,300,colormat);

        // vector to store moments
        vector<rMoments> vMoments;


        printf("Number of regions: %lu \n\n", vp.size()); 

        int recognizedObjects = 0;
        int horAxisFig = 0;
        int verAxisFig = 0;
        int movAction = 0;

        graph.create(700,1000, CV_8UC3);
        graph.setTo(Scalar(30,30,30));

        for (int i = 0; i< vp.size(); i++){
            vMoments.push_back(computeMoments(vp[i]));

            // printf("m00: %Lf  m10:  %Lf  m01: %Lf \n", vMoments[i].m00,vMoments[i].m10,vMoments[i].m01);
            // printf("u10: %Lf  u01:  %Lf  u11: %Lf  u20: %Lf  u02: %Lf\n",vMoments[i].u10,vMoments[i].u01,vMoments[i].u11,vMoments[i].u20,vMoments[i].u02 );
            printf("phi1: %Lf  phi2: %Lf  \n", vMoments[i].phi1, vMoments[i].phi2);

            int x2 = vMoments[i].m10/vMoments[i].m00 + 100* cos(vMoments[i].theta);
            int y2 = vMoments[i].m01/vMoments[i].m00 + 100* sin(vMoments[i].theta); 
            int x3 = vMoments[i].m10/vMoments[i].m00 - 100* cos(vMoments[i].theta);
            int y3 = vMoments[i].m01/vMoments[i].m00 - 100* sin(vMoments[i].theta); 
            
            line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x2,y2), Scalar(0,255,0),2 );
            line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x3,y3), Scalar(0,255,0),2 );

            printf("Theta: %Lf \n\n",vMoments[i].theta); 
            circle(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), 5 ,Scalar(0,0,255),CV_FILLED,8,0);

            imshow("Color",colormat);

            showRegionGraph(vMoments[i].phi1, vMoments[i].phi2);

            // Pelota de golf (Adelante)
            if(vMoments[i].phi1 >= 0.15 && vMoments[i].phi1 <= 0.17 && vMoments[i].phi2 >= 0 && vMoments[i].phi2 <= 0.0005)
            {
                printf("Pelota de golf (Adelante)\n");
                verAxisFig = 1;
                recognizedObjects++;
            }
            // Palo de golf ancho (Atras)
            else if(vMoments[i].phi1 >= 0.19 && vMoments[i].phi1 <= 0.21 && vMoments[i].phi2 >= 0.003 && vMoments[i].phi2 <= 0.01)
            {
                printf("Palo de golf ancho (Atras)\n");
                verAxisFig = 2;
                recognizedObjects++;
            }

            // Golfista (derecha)
            else if(vMoments[i].phi1 >= 0.36 && vMoments[i].phi1 <= 0.45 && vMoments[i].phi2 >= 0.04 && vMoments[i].phi2 <= 0.1)
            {
                printf("Golfista (derecha)\n");
                horAxisFig = 1;
                if(vMoments[i].theta >= 0)
                {
                    printf("Ir hacia abajo\n");
                    movAction = 2;
                }
                else
                {
                    printf("Ir hacia arriba\n");
                    movAction = 1;
                }

                recognizedObjects++;
            }

            // Palo de golf delgado (izquierda)
            else if(vMoments[i].phi1 >= 0.9 && vMoments[i].phi2 >= 0.9)
            {
                printf("Palo de golf delgado (izquierda)\n");
                horAxisFig = 2;
                if(vMoments[i].theta >= 0)
                {
                    printf("Ir hacia abajo\n");
                    movAction = 2;
                }
                else
                {
                    printf("Ir hacia arriba\n");
                    movAction = 1;
                }

                recognizedObjects++;
            }

        }
        imshow("Grafica de regiones", graph);
        waitKey(100);

        printf("Recognized objects: %d\n", recognizedObjects);

        if(recognizedObjects != 2)
        {
            printf("Command not recognized, try again\n");
            bManualMovement = true;
        }
        else
        {
            printf("Modo automatico activado\n");
        }

        if(!bManualMovement)
        {
            // Movimientos del parrot

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            //Pitch Atras
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"pitch"<<endl;
            heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
            usleep(2500000);

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);


            if (horAxisFig==1)
            {
                //Derecha
                //listo
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout <<" Roll right" << endl;
                heli->setAngles(0.0, 3000.0, 0.0, 0.0, 0.0);
                usleep(2500000);

            }

            else if (horAxisFig==2)
            {      
                //Izquierda
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Return" << endl;
                heli->setAngles(0.0, -3000.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }


            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            if (movAction==1)
            {
                //Elevar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Lift" << endl;
                heli->setAngles(0.0, 0.0, 0.0, -17000.0, 0.0);
                usleep(3000000);
            }

            else if (movAction==2)
            {
                //Bajar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Down" << endl;
                heli->setAngles(0.0, 0.0, 0.0, 9000.0, 0.0);
                usleep(3000000);
            }

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            if (verAxisFig==1)
            {
                //Pitch Adelante
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout<<"pitch"<<endl;
                heli->setAngles(-3000, 0.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }
            else if (verAxisFig==2)
            {
                //Pitch Atras
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout<<"pitch"<<endl;
                heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }

            
            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            cout << "land" << endl;
            heli->land();
            usleep(10000000);

            bManualMovement = true;

        }
    }
}

void showCalibrationMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"1. Regresar al menu de funciones", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"p. Cambiar modo continuo / modo congelado", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"r. Imagen RGB", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"h. Imagen HSV", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"y. Imagen YIQ", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"b. Imagen gris / binarizacion", Point(15,290) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"f. Snap Object", Point(15,330) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,370) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de calibracion", menu);
}

void showFunctionMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"1. Calibracion de camara y thresholds", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"2. Rutina deteccion de objetos y movimiento", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de funciones", menu);
}

void showObjectDetectionMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"2. Regresar al menu de funciones", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"b. Binarizacion", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"f. Segmentacion de imagen", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"m. Iniciar rutina", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de deteccion de objetos", menu);
}




// ------------------------------------------ //
//                  MAIN
// ------------------------------------------ //

int main(int argc,char* argv[])
{
    //establishing connection with the quadcopter
    heli = new CHeli();
    
    //this class holds the image from the drone 
    image = new CRawImage(320,240);
    
    // Initial values for control   
    pitch = roll = yaw = height = 0.0;
    joypadPitch = joypadRoll = joypadYaw = joypadVerticalSpeed = 0.0;

    // Initialize joystick
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    useJoystick = SDL_NumJoysticks() > 0;
    if (useJoystick)
    {
        SDL_JoystickClose(m_joystick);
        m_joystick = SDL_JoystickOpen(0);
    }

    /* Show interface menu to user*/
    showFunctionMenu();

    // showRegionGraph(0.38,0.07);

    while (bContinue)
    {
        uint8_t key = waitKey(3);
        switch (key)
        {
            /* Exit -> 'ESC' key or q key*/
            case 'q':
            case 27:
                cout << "Terminating program" << endl;
                bContinue = false;
                break;
            case '1':
                if(bCalibrationMode)
                {
                    bCalibrationMode = 0;
                    destroyWindow("Menu de calibracion");
                    showFunctionMenu();
                }
                else
                {
                    bCalibrationMode = 1;
                    destroyWindow("Menu de funciones");
                    showCalibrationMenu();
                }
                break;
            case '2':
                if(bObjectDetectionMode)
                {
                    bObjectDetectionMode = 0;
                    destroyWindow("Menu de deteccion de objetos");
                    showFunctionMenu();
                }
                else
                {
                    bObjectDetectionMode = 1;
                    destroyWindow("Menu de funciones");
                    showObjectDetectionMenu();
                }
                break;
        }

        if(bCalibrationMode) calibrationMode(key);
        if(bObjectDetectionMode) objectDetectionMode(key);

        usleep(15000);
    }
    
    heli->land();
    SDL_JoystickClose(m_joystick);
    delete heli;
    delete image;
    return 0;
}
