#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SDL/SDL.h"
/*
 * A simple 'getting started' interface to the ARDrone, v0.2 
 * author: Tom Krajnik
 * The code is straightforward,
 * check out the CHeli class and main() to see 
 */
#include <stdlib.h>
#include "CHeli.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>


/* Developed routines*/
#include "rutinasVision.h"

using namespace std;
using namespace cv;

/* Declaring all the image matrices */
Mat bgrImage = Mat(240, 320, CV_8UC3);
Mat bgrHistogram;
Mat bgrFilter;
Mat hsvImage;
Mat hsvHistogram;
Mat hsvFilter;
Mat yiqImage;
Mat yiqHistogram;
Mat yiqFilter;
Mat grayImage;
Mat binImage;

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
bool navigatedWithJoystick, joypadTakeOff, joypadLand, joypadHover;
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


int main(int argc,char* argv[])
{
    //establishing connection with the quadcopter
    heli = new CHeli();
    
    //this class holds the image from the drone 
    image = new CRawImage(320,240);
    
    // Initial values for control   
    pitch = roll = yaw = height = 0.0;
    joypadPitch = joypadRoll = joypadYaw = joypadVerticalSpeed = 0.0;

    // Destination OpenCV Mat   
    Mat currentImage = Mat(240, 320, CV_8UC3);
    // Show it  
    imshow("ParrotCam", currentImage);

    // Initialize joystick
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    useJoystick = SDL_NumJoysticks() > 0;
    if (useJoystick)
    {
        SDL_JoystickClose(m_joystick);
        m_joystick = SDL_JoystickOpen(0);
    }




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
    putText(menu,"q. Terminar", Point(15,290) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
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
            //image is captured
            heli->renewImage(image);

            // Copy to OpenCV Mat
            rawToMat(bgrImage, image);
            imshow("ParrotCam", bgrImage);


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



        uint8_t key = waitKey(3);
        switch (key)
        {
            /* Exit -> 'ESC' key or q key*/
            case 'q':
            case 27:
                cout << "Terminating program" << endl;
                bContinue = false;
                break;
            case 'p':
                freezeImage = !freezeImage;
                break;
            case 'r':
                if(bEnRGB){ destroyWindow("RGB"); destroyWindow("RGB Histogram"); destroyWindow("RGB Filter"); }
                bEnRGB = !bEnRGB;
                break;
            case 'h':
                if(bEnHSV){ destroyWindow("HSV"); destroyWindow("HSV Histogram"); destroyWindow("HSV Filter"); }
                bEnHSV = !bEnHSV;
                break;
            case 'y':
                if(bEnYIQ){ destroyWindow("YIQ"); destroyWindow("YIQ Histogram"); destroyWindow("YIQ Filter"); }
                bEnYIQ = !bEnYIQ;
                break;
            case 'b':
                if(bEnBinarization){ destroyWindow("Gray"); destroyWindow("Gray Binarization"); }
                bEnBinarization = !bEnBinarization;
                break;

            // Parrot commands
            case 'a': yaw = -20000.0; break;
            case 'd': yaw = 20000.0; break;
            case 'w': height = -20000.0; break;
            case 's': height = 20000.0; break;
            case 't': heli->takeoff(); break;
            case 'e': heli->land(); break;
            case 'z': heli->switchCamera(0); break;
            case 'x': heli->switchCamera(1); break;
            case 'c': heli->switchCamera(2); break;
            case 'v': heli->switchCamera(3); break;
            case 'j': roll = -20000.0; break;
            case 'l': roll = 20000.0; break;
            case 'i': pitch = -20000.0; break;
            case 'k': pitch = 20000.0; break;
            case 'o': hover = (hover + 1) % 2; break;
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
        }

        // prints the drone telemetric data, helidata struct contains drone angles, speeds and battery status
        // printf("===================== Parrot Basic Example =====================\n\n");
        // fprintf(stdout, "Angles  : %.2lf %.2lf %.2lf \n", helidata.phi, helidata.psi, helidata.theta);
        // fprintf(stdout, "Speeds  : %.2lf %.2lf %.2lf \n", helidata.vx, helidata.vy, helidata.vz);
        // fprintf(stdout, "Battery : %.0lf \n", helidata.battery);
        // fprintf(stdout, "Hover   : %d \n", hover);
        // fprintf(stdout, "Joypad  : %d \n", useJoystick ? 1 : 0);
        // fprintf(stdout, "  Roll    : %d \n", joypadRoll);
        // fprintf(stdout, "  Pitch   : %d \n", joypadPitch);
        // fprintf(stdout, "  Yaw     : %d \n", joypadYaw);
        // fprintf(stdout, "  V.S.    : %d \n", joypadVerticalSpeed);
        // fprintf(stdout, "  TakeOff : %d \n", joypadTakeOff);
        // fprintf(stdout, "  Land    : %d \n", joypadLand);
        // fprintf(stdout, "Navigating with Joystick: %d \n", navigatedWithJoystick ? 1 : 0);
        // cout<<"Pos X: "<<Px<<" Pos Y: "<<Py<<" Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
        

        if (joypadTakeOff) {
            heli->takeoff();
        }
        if (joypadLand) {
            heli->land();
        }
        //hover = joypadHover ? 1 : 0;

        //setting the drone angles
        if (joypadRoll != 0 || joypadPitch != 0 || joypadVerticalSpeed != 0 || joypadYaw != 0)
        {
            heli->setAngles(joypadPitch, joypadRoll, joypadYaw, joypadVerticalSpeed, hover);
            navigatedWithJoystick = true;
        }
        else
        {
            heli->setAngles(pitch, roll, yaw, height, hover);
            navigatedWithJoystick = false;
        }

        usleep(15000);
    }
    
    heli->land();
    SDL_JoystickClose(m_joystick);
    delete heli;
    delete image;
    return 0;
}
