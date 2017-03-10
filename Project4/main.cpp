/*
* Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
* This file is part of MindControl.
*
* MindControl is free software: you can redistribute it and/or modify
* it under the terms of the GNU  General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* MindControl s distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with MindControl. If not, see <http://www.gnu.org/licenses/>.
*
* For the most up to date version of this software, see:
* https://github.com/samuellab/mindcontrol
*
*
*
* NOTE: If you use any portion of this code in your research, kindly cite:
* Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
* 	"Optogenetic manipulation of neural activity with high spatial resolution in
*	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
*/

/*
* ~/workspace/OpticalMindControl/main.cpp
* main.cpp
*
*  Created on: Jul 20, 2009
*      Author: Andy
*/
/*we've changed the style of mindcontrol code, that made it to oo style
*	Modified on:Mar ,2017
	Author: Wenkai
*/


/*
* This is the main file for the MindControl software.
*
* This file starts two parallel threads. One thread is responsible for displaying
* images, interacting with the user and manipulating the microscope stage.
* The other thread reads in images of a moving Fish and generates illumination patterns
* corresponding to targets on that Fish which are then transmitted to a digital
* micromirror device.
* then we made it three parallel threads. One thread is responsible for displaying images,
* interacting with the user and manipulating some parameters.
* the other two is about high resolution and low resolution images of the fish.
*

*
*/

//Standard C headers
#include <unistd.h>//unisted.h is used in the linux/unix, I find a substitute code realized the same funtion in the stackflow
#include <stdio.h>
#include <ctime>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <sys/time.h>//sys/time.h is also a unix header

//Windows Header
#include <windows.h>

//C++ header
#include <iostream>
#include <limits>

using namespace std;

//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//DAQ card Header
/*#include <NIDAQmx.h> */

//Andy's Personal Headers
#include "MyLibs/AndysOpenCVLib.h"
//#include "MyLibs/Talk2FrameGrabber.h"
#include "MyLibs/Talk2Camera.h"
#include "MyLibs/AndysComputations.h"
#include "MyLibs/FishAnalysis.h"
#include "MyLibs/WriteOutFish.h"
#include "API/mc_api_dll.h"
#include "MyLibs/experiment.h"


//3rd Party Libraries
#include "3rdPartyLibs/tictoc.h"

/** Global Variables (for multithreading) **/
UINT Thread1(LPVOID lpdwParam);

using namespace cv;

//IplImage* CurrentImg;
bool DispThreadHasStarted;
bool MainThreadHasStopped;
bool DispThreadHasStopped;
bool UserWantsToStop;

int main(int argc, char** argv) {

	int DEBUG = 0;
	int ret = 0;


	if (DEBUG) {
		namedWindow("Debug");
		//	cvNamedWindow("Debug2");
	}

	/** Display output about the OpenCV setup currently installed **/
	DisplayOpenCVInstall();

	/** Create a new experiment object **/
	Experiment exp ;
	//create object can be finished in the class using the constructor;

	/** Create memory and objects **/
	exp.InitializeExperiment();//to init the whole class,using the direct constructor



	exp->e = 0; //set errors to zero.,exp.e can be a public value

				/** Deal with CommandLineArguments **/
	exp.LoadCommandLineArguments( argc, argv);

	//handle command line can be checked in the load function
	if (HandleCommandLineArguments(exp) == -1)
		return -1;

	/** Start Camera Input **/
	exp.RollCameraInput();


	/** SetUp Data Recording **/
	//may be we should not use the exp.e too often,just check it in the class
	exp.e = SetupRecording();



	/*Start the frame rate timer */
	exp.StartFrameRateTimer();


	/** Quit now if we have some errors **/

	//check can be set at the step 
	if (exp->e != 0)
		return -1;


	/** Setup Segmentation Gui **/
	// 
	exp.AssignWindowNames();


	/**if (exp->stageIsPresent) {

	printf("TrackingThread: invoking stage...\n ");
	InvokeStage(exp);

	} **/



	/** Start New Thread1 **/
	//the gui thread may do well
	DWORD dwThreadId1;
	HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread1,
		(void*)exp, 0, &dwThreadId1);
	if (hThread1 == NULL) {
		printf("Cannot create display thread.\n");
		return -1;
	}





	// wait for thread
	DispThreadHasStarted = FALSE;
	DispThreadHasStopped = FALSE;
	MainThreadHasStopped = FALSE;

	while (!DispThreadHasStarted)
		Sleep(50);



	/** Giant While Loop Where Everything Happens **/
	
	
	TICTOC::timer().tic("WholeLoop");
	int VideoRanOut = 0;
	UserWantsToStop = 0;




	while (UserWantsToStop != 1) {
		TICTOC::timer().tic("OneLoop");
		//may be we should consider that the public and the private value,or just a struct may do well?
		
		if (isFrameReady(exp)) {

			/** Set error to zero **/
			exp->e = 0;
			TICTOC::timer().tic("GrabFrame()");
			/** Grab a frame **/


			ret = exp.GrabFrame();
			TICTOC::timer().toc("GrabFrame()");

		}

		if (ret == EXP_VIDEO_RAN_OUT) {
			VideoRanOut = 1;
			printf("Video ran out!\n");
			break;
		}

		if (ret == EXP_ERROR) {
			/** Loop again to try to get another frame **/
			printf("Trying again to grab a frame...\n");
			if (UserWantsToStop) break;
			continue;
		}

		/** Calculate the frame rate and every second print the result **/
		exp.CalculateAndPrintFrameRate();


		//exp.param is a subclass
		/** Do we even bother doing analysis?**/
		if (exp.Params.OnOff == 0) {
			/**Don't perform any analysis**/;
			continue;
		}

		/** Load Image into Our Fish Objects **/

		if (exp.e == 0)
		{
			exp.e = exp.LoadFishImg();
			//printf("no Segmentation\n");
		}


		TICTOC::timer().tic("EntireSegmentation");
		/** Do Segmentation **/
		//DoSegmentation(exp);
		//LoadFishColorOriginal(exp->Fish,exp->Fish->ImgOrig);






		if (exp.FindFishBoundary()) {
			//cvThin(FishPtr,15);
			//cvDrawContours(exp->Fish->ImgOrig,exp->Fish->Boundary,cvScalarAll(255),cvScalarAll(0),5);
			//start=clock();


			exp.find_fish_head();

			exp.cvThin(10);
		//10 can be changed in the fishanalysis.h ,define a const,or a ...
			//opencv 2/3 may offer us thin algorithm



			//10 can be changed
			//cvCircle(exp->ImgOrig,FishPtr->Head,3,cvScalarAll(255));
			// cvCircle(FishPtr->ImgOrig,FishPtr->Tail,5,cvScalarAll(255));




		}
		else {
			cout<<"can not find a proper fish, please change the params to make it"
		}


		/*** DIsplay Some Monitoring Output ***/
		//if (exp->e == 0) CreateWormHUDS(exp->HUDS,exp->Fish,exp->Params,exp->IlluminationFrame);

		if (exp->e == 0) exp.CreateFishHUDS();//exp->HUDS, exp->Fish, exp->Params


		if (exp->e == 0) {
			TICTOC::timer().tic("DoWriteToDisk()");
			exp.DoWriteToDisk();
			TICTOC::timer().toc("DoWriteToDisk()");
		}

		//if (exp->e != 0) {
		//	printf("\nError in main loop. :(\n");
		//	if (exp->stageIsPresent) {
		//		printf("\tAuto-safety STAGE SHUTOFF!\n");
		//		ShutOffStage(exp);
		//	}

		//	}

		//	TICTOC::timer().tic("HandleStageTracker()");
		//		HandleStageTracker(exp);
		//	TICTOC::timer().toc("HandleStageTracker()");

		if (UserWantsToStop)
			break;

		TICTOC::timer().toc("OneLoop");

		//if (exp->e == 0
		//		&& EverySoOften(exp->Fish->frameNum, exp->Params->DispRate)) {
		//	TICTOC::timer().tic("DisplayOnScreen");
		/** Setup Display but don't actually send to screen **/
		//	PrepareSelectedDisplay(exp);
		//	TICTOC::timer().toc("DisplayOnScreen");
		//}

	}
	/** Shut down the main thread **/

	TICTOC::timer().toc("WholeLoop");
	/** Tell the display thread that the main thread is shutting down**/
	MainThreadHasStopped = TRUE;

	TICTOC::timer().tic("FinishRecording()");
	exp.FinishRecording();
	TICTOC::timer().toc("FinishRecording()");

	/***** Turn off Camera & DLP ****/
	if (!(exp->VidFromFile) && !(exp->UseFrameGrabber)) {
		/***** Turn off Camera & DLP ****/
		T2Cam_Close(exp->MyCamera);
	}

	if (!(exp->VidFromFile) && (exp->UseFrameGrabber)) {
		//	CloseFrameGrabber(exp->fg);
	}

	printf("%s", TICTOC::timer().generateReportCstr());
	if (!DispThreadHasStopped) {
		printf("Waiting for DisplayThread to Stop...");

	}
	while (!DispThreadHasStopped) {
		printf(".");
		Sleep(500);
		cvWaitKey(10);
	}


	//if (exp->stageIsPresent)
	//	ShutOffStage(exp);
	//this part need a deconstrutor
	ReleaseExperiment(exp);
	DestroyExperiment(&exp);

	printf("\nMain Thread: Good bye.\n");





	return 0;
}

/**
* Thread to display image.
*/
UINT Thread1(LPVOID lpdwParam) {

	Experiment* exp = (Experiment*)lpdwParam;
	printf("DisplayThread: Hello!\n");
	MSG Msg;

	SetupGUI(exp);
	cvWaitKey(30);
	//	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	DispThreadHasStarted = TRUE;
	printf("DisplayThread has started! \n");
	cvWaitKey(30);

	int key;

	while (!MainThreadHasStopped) {

		//needed for display window
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&Msg);

		TICTOC::timer().tic("DisplayThreadGuts");
		TICTOC::timer().tic("cvShowImage");

		if (exp->Params->OnOff) {
			TICTOC::timer().tic("DisplayOnScreen");

			if (exp->e == 0) PrepareSelectedDisplay(exp);

			TICTOC::timer().toc("DisplayOnScreen");

		}
		else {

			cvShowImage(exp->WinDisp, exp->fromCCD->iplimg);
		}
		TICTOC::timer().toc("cvShowImage");

		if (MainThreadHasStopped == 1)
			continue;

		TICTOC::timer().toc("DisplayThreadGuts");
		UpdateGUI(exp);

		//key = cvWaitKey(100);   

		if (MainThreadHasStopped == 1)
			continue;

		if (HandleKeyStroke(key, exp)) {
			printf("\n\nEscape key pressed!\n\n");

			/** Let the Other thread know that the user wants to stop **/
			UserWantsToStop = 1;

			/** Emergency Shut off the Stage **/
			//	printf("Emergency stage shut off.");

			//	if (exp->stageIsPresent)
			//		ShutOffStage(exp);

			/** Exit the display thread immediately **/
			DispThreadHasStopped = TRUE;
			printf("\nDisplayThread: Goodbye!\n");
			return 0;

		}

		//UpdateGUI(exp);

	}

	printf("\nDisplayThread: Goodbye!\n");
	DispThreadHasStopped = TRUE;
	return 0;
}



