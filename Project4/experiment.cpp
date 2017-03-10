/*
* experiment.cpp
*
*  Created on: Sep 5, 2013
*      Author: quan
*/
/*
/*
* experiment.c
*
*  Created on: Sep 3, 2013
*      Author: quan modified from Andy's code
*
*  The experiment.c/.h library is designed to be an extremely high level library.
*	The idea here is to have all of the elements of an experiment laid out, such that
*	a user need only to call a few high level functions to run an experiment.
*/

//Standard C headers
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>

//OpenCV Headers
#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

//DAQ card Header
//#include <NIDAQmx.h>

//Timer Lib
#include "../3rdPartyLibs/tictoc.h"

//Andy's Personal Headers
#include "Talk2Camera.h"
//#include "Talk2Stage.h"
//#include "Talk2FrameGrabber.h"
#include "AndysOpenCVLib.h"
#include "AndysComputations.h"
#include "FishAnalysis.h"
#include "WriteOutFish.h"
#include "version.h"
#include "../API/mc_api_dll.h"
#include "experiment.h"

/*
* Creates a new experiment object and sets values to zero.
*/
Experiment& CreateExperimentStruct() {

	/** Create Experiment Object **/
	Experiment* exp;
	exp = (Experiment*)malloc(sizeof(Experiment));

	/*************************************/
	/**  Set Everything to zero or NULL **/
	/*************************************/

	/** GuiWindowNames **/
	exp->WinDisp = NULL;
	exp->WinCon1 = NULL;

	/** Error Handling **/
	exp->e = 0;

	/** CommandLine Input **/
	exp->argv = NULL;
	exp->argc = 0;
	exp->outfname = NULL;
	exp->infname = NULL;
	exp->dirname = NULL;

	/** Simulation? True/False **/
	exp->VidFromFile = 0;

	exp->MyCamera = NULL;

	/** FrameGrabber Input **/
	//exp->fg = NULL;
	exp->UseFrameGrabber = FALSE;


	/** Camera Input**/
	exp->MyCamera = NULL;

	/** User-configurable Fish-related Parameters **/
	exp->Params = NULL;

	/** Information about Our Fish **/
	exp->Fish = NULL;

	/** internal IplImage **/
	exp->SubSampled = NULL; // Image used to subsample stuff
	exp->HUDS = NULL; //Image used to generate the Heads Up Display
	exp->CurrentSelectedImg = NULL; //The current image selected for display

									/** Timing  Information **/
	exp->now = 0;
	exp->last = 0;

	/** Frame Rate Information **/
	exp->nframes = 0;
	exp->prevFrames = 0;
	exp->prevTime = 0;

	/** Write Data To File **/
	exp->DataWriter = NULL;

	/** Macros **/
	exp->RECORDVID = 0;
	exp->RECORDDATA = 0;

	exp->sm = NULL;

	/** Stage Control **/
	//exp->stageIsPresent = 0;
	//exp->stage = NULL;
	//exp->stageVel = cvPoint(0, 0);
	//exp->stageCenter = cvPoint(0, 0);
	//exp->stageFeedbackTargetOffset = cvPoint(0, 0);
	//exp->stageIsTurningOff = 0;

	return exp;

}

/*
* Load the command line arguments into the experiment object
*/
void LoadCommandLineArguments(Experiment* exp, int argc, char** argv) {
	exp->argc = argc;
	exp->argv = argv;
}

void displayHelp() {
	printf(
		"\n\n this software analyzes each frame, finds the Fish eye and calculates its orientation in real time\n");
	printf(
		"by Quan Wen, qwen@fas.harvard.edu, modified from Andy Leifer's code");
	printf("\nUsage:\n\n");
	printf(
		"If run with no arguments,  it will identify the Fish, but you need to track the Fish by hand.\n\n");
	printf("Optional arguments:\n");

	printf("\t-?\n\t\tDisplay this help.\n\n");
	printf("\nSee shortcutkeys.txt for a list of keyboard shortcuts.\n");
}

/*
* Handle CommandLine Arguments
* Parses commandline arguments.
* Decides if user wants to record video or recorddata
*/

int HandleCommandLineArguments(Experiment* exp) {
	int dflag = 0;
	opterr = 0;

	int c;
	while ((c = getopt(exp->argc, exp->argv, "si:d:o:p:gtx:y:?")) != -1) {
		switch (c) {

		case 'i': /** specify input video file **/
			exp->VidFromFile = 1;
			exp->infname = optarg;
			printf("Read from file %s \n", exp->infname);
			if (optarg == NULL) {
				printf(
					"Error. Given -i switch but no input video file was specified.\n");
				return -1;
			}
			break;

		case 'd': /** specifiy directory **/
			dflag = 1;
			if (optarg != NULL) {
				exp->dirname = optarg;
			}
			else {
				exp->dirname = "./"; // set to default, local directory;
			}
			break;

		case 'o': /** specify base filename of output **/
			if (optarg != NULL) {
				exp->outfname = optarg;
			}
			else {
				exp->outfname = "Fish"; // set the base filename to the default of fish;
			}
			exp->RECORDVID = 1;
			exp->RECORDDATA = 1;
			break;

		case 'g': /** Use frame grabber **/
			exp->UseFrameGrabber = TRUE;
			break;

			//case 't': /** Use the stage tracking software **/
			//	exp->stageIsPresent = 1;
			//	break;

			//case 'x': /** adjust the target for stage feedback loop by these certain number of pixels **/
			//	if (optarg != NULL) {
			//		exp->stageFeedbackTargetOffset.x = atoi(optarg);
			//	}
			//	printf(
			//			"Adjusting target for stage feedback loop by x= %d pixels.\n",
			//			exp->stageFeedbackTargetOffset.x);
			//	break;

			//case 'y': /** adjust the target for stage feedback loop by these certain number of pixels **/
			//	if (optarg != NULL) {
			//		exp->stageFeedbackTargetOffset.y = atoi(optarg);
			//	}
			//	printf(
			//			"Adjusting target for stage feedback loop by y= %d pixels.\n",
			//			exp->stageFeedbackTargetOffset.y);
			//	break;

		case '?':
			displayHelp();
			return -1;
			break;

		default:
			displayHelp();
			return -1;
		} // end of switch

	} // end of while loop
	return 1;
}

/** GUI **/

/* Assigns Default window names to the experiment object
*
*/
void AssignWindowNames(Experiment* exp) {

	char* disp1 = (char*)malloc(strlen("Display"));
	char* control1 = (char*)malloc(strlen("Controls"));

	disp1 = "Display";
	control1 = "Controls";

	exp->WinDisp = disp1;
	exp->WinCon1 = control1;

}

/*
* Release the memopry associated with window names
* and set their pointers to Null
*/
void ReleaseWindowNames(Experiment* exp) {
	if (exp->WinDisp != NULL)
		free(exp->WinDisp);
	if (exp->WinCon1 != NULL)
		free(exp->WinCon1);

	exp->WinDisp = NULL;
	exp->WinCon1 = NULL;

}

/*
* SetupGui
*
*/

void SetupGUI(Experiment* exp) {

	printf("Beginning to setup GUI\n");

	cvNamedWindow(exp->WinDisp, 0); // <-- This goes into the thread.
	cvNamedWindow(exp->WinCon1);
	cvResizeWindow(exp->WinCon1, 500, 500);

	/** SelectDisplay **/
	cvCreateTrackbar("SelectDisplay", "Controls", &(exp->Params->Display), 2,
		NULL);
	printf("Pong\n");

	/** On Off **/
	cvCreateTrackbar("On", exp->WinCon1, &(exp->Params->OnOff), 1, NULL);

	/** Segmentation Parameters**/
	cvCreateTrackbar("Gauss=x*2+1", exp->WinCon1, &(exp->Params->GaussSize),
		15, (int)NULL);

	cvCreateTrackbar("BinaryThreshold", exp->WinCon1, &(exp->Params->BinThresh), 255,
		NULL);

	//cvCreateTrackbar("LowestThreshold", exp->WinCon1, &(exp->Params->BinThresh_LL), 255,
	//		NULL);
	//cvCreateTrackbar("HighThreshold", exp->WinCon1, &(exp->Params->BinThresh_H), 255,
	//		NULL);

	//cvCreateTrackbar("EyeSize", exp->WinCon1, &(exp->Params->EyeSize), 100,
	//		NULL);
	cvCreateTrackbar("MaxFishArea", exp->WinCon1, &(exp->Params->AreaUpBound), 7000,
		NULL);
	cvCreateTrackbar("MinFishArea", exp->WinCon1, &(exp->Params->AreaBottomBound), 5000,
		NULL);
	cvCreateTrackbar("MaxFishLength", exp->WinCon1, &(exp->Params->LengthUpBound), 700,
		NULL);
	cvCreateTrackbar("MinFishLength", exp->WinCon1, &(exp->Params->LengthBottomBound), 300,
		NULL);
	cvCreateTrackbar("CornerWidth", exp->WinCon1, &(exp->Params->CornerWidth), 500,
		NULL);

	//cvCreateTrackbar("MinEyeArea", exp->WinCon1, &(exp->Params->Area_S), 5000,
	//		NULL);

	/** Record Data **/
	cvCreateTrackbar("RecordOn", exp->WinCon1, &(exp->Params->Record), 1,
		(int)NULL);

	//cvCreateTrackbar("Diameter", exp->WinCon1, &(exp->Params->MaskDiameter),NSIZEY,NULL);

	//if (exp->stageIsPresent) {
	//	cvCreateTrackbar("StageSpeedFactor", exp->WinCon1,
	//			&(exp->Params->stageSpeedFactor), 100, NULL);
	//}

	//if (exp->stageIsPresent) {
	//	cvCreateTrackbar("MaxStageSpeed", exp->WinCon1, &(exp->Params->maxstagespeed),10,NULL);
	//}

	printf("Created trackbars and windows\n");
	return;

}

/*
* Update's trackbar positions for variables that can be changed by the software
*
*/
void UpdateGUI(Experiment* exp) {

	/** Threshold **/
	cvSetTrackbarPos("LowThreshold", exp->WinCon1, (exp->Params->BinThresh_L));
	cvSetTrackbarPos("LowestThreshold", exp->WinCon1, (exp->Params->BinThresh_LL));
	cvSetTrackbarPos("HighThreshold", exp->WinCon1, (exp->Params->BinThresh_H));
	cvSetTrackbarPos("Gauss=x*2+1", exp->WinCon1, exp->Params->GaussSize);
	cvSetTrackbarPos("EyeSize", exp->WinCon1, exp->Params->EyeSize);
	cvSetTrackbarPos("MaxEyeArea", exp->WinCon1, exp->Params->Area_L);
	cvSetTrackbarPos("MinEyeArea", exp->WinCon1, exp->Params->Area_S);

	//cvSetTrackbarPos("Diameter", exp->WinCon1, (exp->Params->MaskDiameter));

	cvSetTrackbarPos("On", exp->WinCon1, (exp->Params->OnOff));

	cvSetTrackbarPos("SelectDisplay", exp->WinCon1, (exp->Params->Display));
	cvSetTrackbarPos("RecordOn", exp->WinCon1, (exp->Params->Record));

	/**Stage Speed **/
	//if (exp->stageIsPresent){
	//	cvSetTrackbarPos("StageSpeedFactor", exp->WinCon1,
	//			(exp->Params->stageSpeedFactor));

	//	cvSetTrackbarPos("MaxStageSpeed", exp->WinCon1,
	//					(exp->Params->maxstagespeed));


	//}

	return;

}

/*** Start Video Camera ***/

/*
* Initialize camera library
* Allocate Camera Data
* Select Camera and Show Properties dialog box
* Start Grabbing Frames as quickly as possible
* *
* OR open up the video file for reading.
*/
void RollCameraInput(Experiment* exp) {

	if (exp->VidFromFile) { /** Use source from file **/
							/** Define the File catpure **/
		exp->capture = cvCreateFileCapture(exp->infname);

	}
	else {
		/** Use source from camera **/
		if (exp->UseFrameGrabber) {
			//exp->fg = TurnOnFrameGrabber();

			//printf("Checking frame size of frame grabber..\n");
			/** Check to see that our image sizes are all the same. **/
			//if ((int) exp->fg->xsize != exp->fromCCD->size.width
			//		|| (int) exp->fg->ysize != exp->fromCCD->size.height) {
			//	printf("Error in RollVideoInput!\n");
			//	printf(
			//			"Size from framegrabber does not match size in IplImage fromCCD!\n");
			//	printf(" exp->fg->xsize=%d\n", (int) exp->fg->xsize);
			//	printf(" exp->fromCCD->size.width=%d\n",
			//			exp->fromCCD->size.width);
			//	printf(" exp->fg->ysize=%d\n", (int) exp->fg->ysize);
			//	printf(" exp->fromCCD->size.height=%d\n",
			//			exp->fromCCD->size.height);
			//	exp->e = EXP_ERROR;
			//	return;
			//}

			//printf("Frame size checks out..");

			/**Use Frame Grabber **/
		}
		else {
			/** Use Basler USB Camera **/

			/** Turn on Camera **/

			if (T2Cam_Initialize(exp->MyCamera) != EXP_SUCCESS) exp->e = EXP_ERROR;


		}

	}
}

/*
* This function allocates images and frames
* And a Fish Object
*
* And a Parameter Object
* For internal manipulation
*
*
*/
void InitializeExperiment(Experiment* exp) {

	/*** Create IplImage **/
	IplImage* SubSampled = cvCreateImage(cvSize(NSIZEX / 2, NSIZEY / 2),
		IPL_DEPTH_8U, 1);
	IplImage* HUDS = cvCreateImage(cvSize(NSIZEX, NSIZEY), IPL_DEPTH_8U, 1);


	exp->CurrentSelectedImg = cvCreateImage(cvSize(NSIZEX, NSIZEY), IPL_DEPTH_8U, 1);

	exp->SubSampled = SubSampled;
	exp->HUDS = HUDS;

	/*** Create Frames **/
	Frame* fromCCD = CreateFrame(cvSize(NSIZEX, NSIZEY));

	CamData* MyCamera = T2Cam_CreateCamData();

	exp->MyCamera = MyCamera;

	exp->fromCCD = fromCCD;

	/** Create Fish Data Struct and Fish Parameter Struct **/
	FishAnalysisData* Fish = CreateFishAnalysisDataStruct();
	FishAnalysisParam* Params = CreateFishAnalysisParam();
	InitializeEmptyFishImages(Fish, cvSize(NSIZEX, NSIZEY));
	InitializeFishMemStorage(Fish);

	exp->Fish = Fish;
	exp->Params = Params;

	/** Create API shared memory for fish feature vectors analysis and behavioral prediction **/
	exp->sm = MC_API_StartServer();

}

/*
* Free up all of the different allocated memory for the
* experiment.
*
*/
void ReleaseExperiment(Experiment* exp) {
	/** Free up Frames **/
	if (exp->fromCCD != NULL)
		DestroyFrame(&(exp->fromCCD));

	/** Free up Strings **/
	exp->dirname = NULL;
	exp->infname = NULL;
	exp->outfname = NULL;

	/** Free up Fish Objects **/
	if (exp->Fish != NULL) {
		DestroyFishAnalysisDataStruct((exp->Fish));
		exp->Fish = NULL;
	}

	if (exp->Params != NULL) {
		DestroyFishAnalysisParam((exp->Params));
		exp->Params = NULL;
	}

	/** Free up internal iplImages **/
	if (exp->CurrentSelectedImg != NULL)
		cvReleaseImage(&(exp->CurrentSelectedImg));
	if (exp->SubSampled != NULL)
		cvReleaseImage(&(exp->SubSampled));
	if (exp->HUDS != NULL)
		cvReleaseImage(&(exp->HUDS));

	if (exp->fromCCD != NULL)
		DestroyFrame(&(exp->fromCCD));

	/** Stop MindControl API Shared Memory Server **/
	if (exp->sm != NULL) {
		MC_API_StopServer(exp->sm);
		exp->sm = NULL;
	}

	if (exp->VidFromFile) {
		cvReleaseCapture(&(exp->capture));
	}


	/** Release Window Names **/
	ReleaseWindowNames(exp);

}

/* Destroy the experiment object.
* To be run after ReleaseExperiment()
*/
void DestroyExperiment(Experiment** exp) {
	free(*exp);
	*exp = NULL;
}

/*********************************************
*
* Image Acquisition
*
*/

/** Grab a Frame from either camera or video source
*
*/
int GrabFrame(Experiment* exp) {

	if (!(exp->VidFromFile)) {

		if (exp->UseFrameGrabber) {
			/** Use BitFlow SDK to acquire from Frame Grabber **/
			//if (AcquireFrame(exp->fg)==T2FG_ERROR){
			//	return EXP_ERROR;
			//}

			/** Check to see if file sizes match **/

			//LoadFrameWithBin(exp->fg->HostBuf, exp->fromCCD);

		}
		else {


			/** Acqure from ImagingSource USB Cam **/

			if (T2Cam_GrabFrame(exp->MyCamera) == EXP_SUCCESS) {

				if ((int)exp->MyCamera->grabResult.SizeX != exp->fromCCD->size.width || (int)exp->MyCamera->grabResult.SizeY != exp->fromCCD->size.height) {


					printf("Size from Camera does not match size in IplImage fromCCD!\n");

					return EXP_ERROR;
				}

				LoadFrameWithBin(exp->MyCamera->ImageRawData, exp->fromCCD);
				exp->Fish->timestamp_on_camera = exp->MyCamera->grabResult.TimeStamp;


			}

			else return EXP_ERROR;



		}


	}
	else {

		/** Acquire  from file **/

		IplImage* tempImg;
		/** Grab the frame from the video **/


		tempImg = cvQueryFrame(exp->capture);



		/** Stall for a little bit **/
		//Sleep(50);


		if (tempImg == NULL) {
			printf("There was an error querying the frame from video!\n");
			return EXP_VIDEO_RAN_OUT;
		}

		/** Create a new temp image that is grayscale and of the same size **/
		IplImage* tempImgGray = cvCreateImage(cvGetSize(tempImg), IPL_DEPTH_8U,
			1);

		/** Convert Color to GrayScale **/
		cvCvtColor(tempImg, tempImgGray, CV_RGB2GRAY);

		/** Load the frame into the fromCCD frame object **/
		/*** ANDY! THIS WILL FAIL BECAUSE THE SIZING ISN'T RIGHT **/
		LoadFrameWithImage(tempImgGray, exp->fromCCD);
		cvReleaseImage(&tempImgGray);
		//cvReleaseImage(&tempImg);
		/*
		* Note: for some reason thinks crash when you go cvReleaseImage(&tempImg)
		* And there don't seem to be memory leaks if you leave it. So I'm going to leave it in place.
		*
		*/
	}

	exp->Fish->frameNum++;
	return EXP_SUCCESS;
}

/*
* Is a frame ready from the camera?
*
*/
int isFrameReady(Experiment* exp) {

	if (!(exp->VidFromFile) && !(exp->UseFrameGrabber)) {
		/** If This isn't a simulation.. **/
		/** And if we arent using the frame grabber **/
		return 1;
	}
	else {
		/** Otherwise just keep chugging... **/

		/** Unless we're reading from video, in which case we should fake like we're waiting for something **/
		if (exp->VidFromFile)
			//cvWaitKey(10);
			return 1;
	}
}



/************************************************/
/*   Frame Rate Routines
*
*/
/************************************************/

/*
*This is the frame rate timer.
*/
void StartFrameRateTimer(Experiment* exp) {
	exp->prevTime = clock();
	exp->prevFrames = 0;

}

/*
* If more than a second has elapsed
* Calculate the frame rate and print i tout
*
*/
void CalculateAndPrintFrameRate(Experiment* exp) {
	/*** Print out Frame Rate ***/
	if ((exp->Fish->timestamp - exp->prevTime) > CLOCKS_PER_SEC) {
		printf("%d fps\n", exp->Fish->frameNum - exp->prevFrames);
		exp->prevFrames = exp->Fish->frameNum;
		exp->prevTime = exp->Fish->timestamp;
		//printf("crossed_angle is %d \n",exp->Fish->Eyes->mean_CrossedAngle);
	}
}



/************************************************/
/*   Action Chunks
*
*/
/************************************************/

/*
* Given an image in teh Fish object, segment the Fish
*
*/
void DoSegmentation(Experiment* exp) {
	//_TICTOC_TIC_FUNC/
	/*** <segmentFish> ***/

	/*** Find Fish Boundary ***/


	//TICTOC::timer().tic("_FindFishCenter",exp->e);

	//if (!(exp->e))

	//	exp->e = FindFishCenter(exp->Fish, exp->Params);


	//TICTOC::timer().toc("_FindFishCenter",exp->e);


	TICTOC::timer().tic("_SegmentingEye", exp->e);

	if (!(exp->e))

		exp->e = SegmentingEye(exp->Fish, exp->Params);


	TICTOC::timer().toc("_SegmentingEye", exp->e);




	//_TICTOC_TOC_FUNC
}

/*
* Prepare the Selected Display
*
*/
void PrepareSelectedDisplay(Experiment* exp) {
	/** There are no errors and we are displaying a frame **/
	switch (exp->Params->Display) {
	case 0:
		//
		//exp->CurrentSelectedImg = exp->Fish->ImgOrig;
		cvShowImage(exp->WinDisp, exp->Fish->ImgOrig);

		break;
	case 2:
		//exp->CurrentSelectedImg = exp->Fish->ImgThresh;
		cvShowImage(exp->WinDisp, exp->Fish->ImgThresh);
		break;
	case 1:
		cvShowImage(exp->WinDisp, exp->HUDS);
		break;

	default:
		break;
	}
	//cvWaitKey(1); // Pause one millisecond for things to display onscreen.

}

/*
*
* Handle KeyStroke
*
* Returns 1 when the user is trying to exit
*
*/
int HandleKeyStroke(int c, Experiment* exp) {
	switch (c) {
	case 27:
		printf("User has pressed escape!\n");
		return 1;
		break;
		/** Threshold **/
		//case ']':
		//	Increment(&(exp->Params->BinThresh), 200);
		//	break;
		//case '[':
		//	Decrement(&(exp->Params->BinThresh), 0);
		//	break;

	case 'r': /** record **/
		Toggle(&(exp->Params->Record));
		if (exp->Params->Record == 0) {
			printf("Turning Record off!\n");
		}
		else {
			printf("Turning Record on!\n");
		}
		break;

		/** Tracker **/
		//case '\t':
		//	Toggle(&(exp->Params->stageTrackingOn));
		//	if (exp->Params->stageTrackingOn == 0) {
		/** If we are turning the stage off, let the rest of the code know **/
		//		printf("Turning tracking off!\n");
		//		exp->stageIsTurningOff = 1;
		//	} else {
		//		printf("Turning tracking on!\n");
		//	}
		//	break;

	case 'o':
		Toggle(&(exp->Params->OnOff));
		if (exp->Params->OnOff == 0) {
			printf("Turning Analysis off!\n");
		}
		else {
			printf("Turning Analysis on!\n");
		}
		break;

		//case '+':
		//	Increment(&(exp->Params->stageSpeedFactor), 50);
		//	printf("stageSpeedFactor=%d\n", exp->Params->stageSpeedFactor);
		//	break;
		//case '-':
		//	Decrement(&(exp->Params->stageSpeedFactor), 0);
		//	printf("stageSpeedFactor=%d\n", exp->Params->stageSpeedFactor);
		//	break;

	case 127: /** Delete key **/
	case 8: /** Backspace key **/
			//	exp->Params->stageTrackingOn = 0;
			//	exp->stageIsTurningOff = 1;
			//	printf("Instructing stage to turn off..");
			//	break;

	default:
		return 0;
		break;
	}
	return 0;
}



void SyncAPI(Experiment* exp) {

	/** Write worm features to the MindControl API **/
	MC_API_SetCurrentFrame(exp->sm, exp->Fish->frameNum);
	MC_API_SetTailBendingAngle(exp->sm, exp->Fish->BendingAngle);



	return;

}

/*********************** RECORDING *******************/

/*
* Sets up data recording and video recording
* Will record video if exp->RECORDVID is 1
* and record data if exp->RECORDDATA is 1
*
*/
int SetupRecording(Experiment* exp) {

	printf("About to setup recording\n");
	char* DataFileName;
	if (exp->RECORDDATA) {
		if (exp->dirname == NULL || exp->outfname == NULL)
			printf("exp->dirname or exp->outfname is NULL!\n");

		/** Setup Writing and Write Out Comments **/
		exp->DataWriter = SetUpWriteToDisk(exp->dirname, exp->outfname, exp->Fish->MemStorage);

		/** We should Quit Now if any of the data Writing is not working **/
		if (exp->DataWriter->error < 0) return -1;

		/** Write the Command Line argument Out for reference **/
		WriteOutCommandLineArguments(exp->DataWriter, exp->argc, exp->argv);

		/**  Write out the default grid size for non-protocol based illumination **/
		//WriteOutDefaultGridSize(exp->DataWriter, exp->Params);

		/** Write the Protocol Out for reference **/
		//if (exp->pflag) {
		//	WriteProtocol(exp->p, exp->DataWriter->fs);
		//}

		BeginToWriteOutFrames(exp->DataWriter);

		printf("Initialized data recording\n");
		DestroyFilename(&DataFileName);
	}

	/** Set Up Video Recording **/
	char* MovieFileName;
	char* HUDSFileName;

	if (exp->RECORDVID) {
		if (exp->dirname == NULL || exp->outfname == NULL)
			printf("exp->dirname or exp->outfname is NULL!\n");

		MovieFileName = CreateFileName(exp->dirname, exp->outfname, ".avi");
		HUDSFileName = CreateFileName(exp->dirname, exp->outfname, "_HUDS.avi");

		exp->Vid = cvCreateVideoWriter(MovieFileName,
			CV_FOURCC('M', 'J', 'P', 'G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
			0);
		exp->VidHUDS = cvCreateVideoWriter(HUDSFileName,
			CV_FOURCC('M', 'J', 'P', 'G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
			0);
		if (exp->Vid == NULL) printf("\tERROR in SetupRecording! exp->Vid is NULL\n");
		if (exp->VidHUDS == NULL) printf("\tERROR in SetupRecording! exp->VidHUDS is NULL\n");
		DestroyFilename(&MovieFileName);
		DestroyFilename(&HUDSFileName);
		printf("Initialized video recording\n");
	}
	return 0;

}

/*
* Finish writing video and  and data
* and release
*
*/
void FinishRecording(Experiment* exp) {
	/** Finish Writing Video to File and Release Writer **/
	if (exp->Vid != NULL)
		cvReleaseVideoWriter(&(exp->Vid));
	if (exp->VidHUDS != NULL)
		cvReleaseVideoWriter(&(exp->VidHUDS));

	/** Finish Writing to Disk **/
	if (exp->RECORDDATA)
		FinishWriteToDisk(&(exp->DataWriter));

}




/*
* Write video and data to Disk
*
*/
void DoWriteToDisk(Experiment* exp) {

	/** Throw error if the user has asked to record, but the system is not in record mode **/
	if (exp->Params->Record && (exp->RECORDVID != 1)) {
		printf("ERROR!! THE SYSTEM IS NOT IN RECORD MODE!\n");
		printf("restart the system to record.\n");
	}

	/** Record VideoFrame to Disk**/
	if (exp->RECORDVID && exp->Params->Record) {
		TICTOC::timer().tic("cvResize");
		cvResize(exp->Fish->ImgOrig, exp->SubSampled, CV_INTER_LINEAR);
		TICTOC::timer().toc("cvResize");

		TICTOC::timer().tic("cvWriteFrame");
		cvWriteFrame(exp->Vid, exp->SubSampled);
		//cvWriteFrame(exp->Vid, exp->Fish->ImgOrig);
		if (exp->Vid == NULL) printf("\tERROR in DoWriteToDisk!\n\texp->Vid is NULL\n");
		if (exp->SubSampled == NULL) printf("\tERROR in DoWriteToDisk!\n\texp->exp->Subsampled==NULL\n");

		TICTOC::timer().toc("cvWriteFrame");

		cvResize(exp->HUDS, exp->SubSampled, CV_INTER_LINEAR);
		if (exp->VidHUDS == NULL) printf("\tERROR in DoWriteToDisk!\n\texp->VidHUDS is NULL\n");
		if (exp->SubSampled == NULL) printf("\tERROR in DoWriteToDisk!\n\texp->exp->Subsampled==NULL\n");

		cvWriteFrame(exp->VidHUDS, exp->SubSampled);
	}

	/** Record data frame to diskl **/

	if (exp->RECORDDATA && exp->Params->Record) {
		TICTOC::timer().tic("AppendFishFrameToDisk");
		AppendFishFrameToDisk(exp->Fish, exp->Params, exp->DataWriter);
		TICTOC::timer().toc("AppendFishFrameToDisk");
	}
}
