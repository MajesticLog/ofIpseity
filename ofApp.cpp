#include "ofApp.h"
#include "opencv2/opencv.hpp"

//optical flow based off of Dr Theo Papatheodorou's opticalFlow exercise for the computational arts MA at Goldsmiths, London 
//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(0);
	ofSetFrameRate(60);
	ofSetBackgroundAuto(false);
	maxBufferSize = 1000;
    video.setDeviceID(0);
    video.setDesiredFrameRate(60);
    video.initGrabber(1280, 720);
    calculatedFlow = false;
	
}

//--------------------------------------------------------------
void ofApp::update(){

	video.update();

	if (video.isFrameNew())
	{
		//add to buffer
		ofImage img;
		img.setFromPixels(video.getPixels());
		img.mirror(false, true); // if you want to mirror input
		imgBuffer.push_front(img);
	}

	//if buffer reched max size
	if (imgBuffer.size() > maxBufferSize) imgBuffer.pop_back();



		//Decode the new frame if needed

	if ( video.isFrameNew() )
        {
		if ( gray1.bAllocated ) {
			gray2 = gray1;
			calculatedFlow = true;
		}

        //Convert to ofxCv images
        ofPixels & pixels = video.getPixels();
        currentColor.setFromPixels( pixels );

        float decimate = 0.5;              //Decimate images to 25% (makes calculations faster + works like a blurr too)
        ofxCvColorImage imageDecimated1;
        imageDecimated1.allocate( currentColor.width * decimate, currentColor.height * decimate );
        imageDecimated1.scaleIntoMe( currentColor, CV_INTER_AREA );             //High-quality resize
        gray1 = imageDecimated1;

		if ( gray2.bAllocated ) {
            Mat img1 = cvarrToMat(gray1.getCvImage());
            Mat img2 = cvarrToMat(gray2.getCvImage());
            Mat flow;                        //Image for flow
            //Computing optical flow (visit https://goo.gl/jm1Vfr for explanation of parameters)
            calcOpticalFlowFarneback( img1, img2, flow, 0.7, 3, 11, 5, 5, 1.1, 0 );
            //Split flow into separate images
            vector<Mat> flowPlanes;
            split( flow, flowPlanes );
            //Copy float planes to ofxCv images flowX and flowY
            //we call this to convert back from native openCV to ofxOpenCV data types
            IplImage iplX( flowPlanes[0] );
            //cvConvert(flowX, iplX);
            flowX = &iplX;
            IplImage iplY( flowPlanes[1] );
            flowY = &iplY;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {


	sumX, sumY, avgX, avgY = 0;
	numOfEntries = 0;
	ofSetRectMode(OF_RECTMODE_CENTER);


	if (calculatedFlow)
	{
		
		int w = gray1.width;
		int h = gray1.height;


		//1. Input images + optical flow
		ofPushMatrix();
		ofScale(4, 4);

		//Optical flow
		float *flowXPixels = flowX.getPixelsAsFloats();
		float *flowYPixels = flowY.getPixelsAsFloats();
		ofSetColor(0, 0, 255);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				float fx = flowXPixels[x + w * y];
				float fy = flowYPixels[x + w * y];
				sumX += fx;
				sumY += fy;
				numOfEntries++;
			}
		}
		ofPopMatrix();
	}

	if (numOfEntries > 0) {
		avgX = sumX / numOfEntries;
		avgY = sumY / numOfEntries;
	}
	
	float angle = atan2(avgX, avgY)*RAD_TO_DEG;
	int rotAngle = ofMap(angle, -360, 360, -20, 20, true); //change those map values to change the steps

	int slitLocX;

	ofScale(0.5);
	ofSetColor(255);


	if (imgBuffer.size() > 0) {
		slitLocX = imgBuffer[0].getHeight();

	}

	ofTranslate(ofGetWidth(), ofGetHeight());
	ofSetColor(255);

	//change to i+=3 etc to play with the image
	for (int i = 0; i < imgBuffer.size(); i++) {

		ofRotate(rotAngle);

		imgBuffer[i].drawSubsection(0, i, ofGetWidth(), 5, 0, i); // 5 corresponds to the width of the pixel line drawn, change that for thicker or thinner lines


	}


	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	//save image
	if (key == 's') {
		glReadBuffer(GL_FRONT);
		ofSaveScreen(ofToString(ofGetFrameNum()) + ".jpg");
	}

	//restart
	if (key == 'r') {
		setup();
	}

}
