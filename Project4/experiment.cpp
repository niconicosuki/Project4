
//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//C++ header
#include <iostream>

//
#include "experiment.h"

/*
* Creates a new experiment object and sets values to zero.
*/


Experiment::Experiment()
	:Params(),
	 Fish()
	{

	/** Simulation? True/False **/
	VidFromFile = 0;



	/** GuiWindowNames **/
	 WinDisp = NULL;
	 WinCon1 = NULL;

	/** Error Handling **/
	 e = 0;

	/** CommandLine Input **/
	 argv = NULL;
	 argc = 0;
	 outfname = NULL;
	 infname = NULL;
	 dirname = NULL;


	 /** FrameGrabber Input **/
	 UseFrameGrabber = false;

	 /** Camera Input **/
	 MyCamera = NULL;


	/** User-configurable Fish-related Parameters
	params is inited in the list above
	
	**/
	 

	/** Information about Our Fish 
	fish is inited in the list above
	
	**/
	

	/** internal IplImage **/
	 SubSampled = NULL; // Image used to subsample stuff
	 HUDS = NULL; //Image used to generate the Heads Up Display
	 CurrentSelectedImg = NULL; //The current image selected for display

									/** Timing  Information **/
	 now = 0;
	 last = 0;

	/** Frame Rate Information **/
	 nframes = 0;
	 prevFrames = 0;
	 prevTime = 0;

	/** Write Data To File **/
	 DataWriter = NULL;

	/** Macros **/
	 RECORDVID = 0;
	 RECORDDATA = 0;

	 //sm = NULL;

}


/*
* Load the command line arguments into the experiment object
*/
void LoadCommandLineArguments(Experiment* exp, int argc, char** argv) {
	 argc = argc;
	 argv = argv;
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
	while ((c = getopt( argc,  argv, "si:d:o:p:gtx:y:?")) != -1) {
		switch (c) {

		case 'i': /** specify input video file **/
			 VidFromFile = 1;
			 infname = optarg;
			printf("Read from file %s \n",  infname);
			if (optarg == NULL) {
				printf(
					"Error. Given -i switch but no input video file was specified.\n");
				return -1;
			}
			break;

		case 'd': /** specifiy directory **/
			dflag = 1;
			if (optarg != NULL) {
				 dirname = optarg;
			}
			else {
				 dirname = "./"; // set to default, local directory;
			}
			break;

		case 'o': /** specify base filename of output **/
			if (optarg != NULL) {
				 outfname = optarg;
			}
			else {
				 outfname = "Fish"; // set the base filename to the default of fish;
			}
			 RECORDVID = 1;
			 RECORDDATA = 1;
			break;

		case 'g': /** Use frame grabber **/
			 UseFrameGrabber = TRUE;
			break;

			//case 't': /** Use the stage tracking software **/
			//	 stageIsPresent = 1;
			//	break;

			//case 'x': /** adjust the target for stage feedback loop by these certain number of pixels **/
			//	if (optarg != NULL) {
			//		 stageFeedbackTargetOffset.x = atoi(optarg);
			//	}
			//	printf(
			//			"Adjusting target for stage feedback loop by x= %d pixels.\n",
			//			 stageFeedbackTargetOffset.x);
			//	break;

			//case 'y': /** adjust the target for stage feedback loop by these certain number of pixels **/
			//	if (optarg != NULL) {
			//		 stageFeedbackTargetOffset.y = atoi(optarg);
			//	}
			//	printf(
			//			"Adjusting target for stage feedback loop by y= %d pixels.\n",
			//			 stageFeedbackTargetOffset.y);
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

	 WinDisp = disp1;
	 WinCon1 = control1;

}

/*
* Release the memopry associated with window names
* and set their pointers to Null
*/
void ReleaseWindowNames(Experiment* exp) {
	if ( WinDisp != NULL)
		free( WinDisp);
	if ( WinCon1 != NULL)
		free( WinCon1);

	 WinDisp = NULL;
	 WinCon1 = NULL;

}

/*
* SetupGui
*
*/

void SetupGUI(Experiment* exp) {

	printf("Beginning to setup GUI\n");

	cvNamedWindow( WinDisp, 0); // <-- This goes into the thread.
	cvNamedWindow( WinCon1);
	cvResizeWindow( WinCon1, 500, 500);

	/** SelectDisplay **/
	cvCreateTrackbar("SelectDisplay", "Controls", &( Params->Display), 2,
		NULL);
	printf("Pong\n");

	/** On Off **/
	cvCreateTrackbar("On",  WinCon1, &( Params->OnOff), 1, NULL);

	/** Segmentation Parameters**/
	cvCreateTrackbar("Gauss=x*2+1",  WinCon1, &( Params->GaussSize),
		15, (int)NULL);

	cvCreateTrackbar("BinaryThreshold",  WinCon1, &( Params->BinThresh), 255,
		NULL);

	//cvCreateTrackbar("LowestThreshold",  WinCon1, &( Params->BinThresh_LL), 255,
	//		NULL);
	//cvCreateTrackbar("HighThreshold",  WinCon1, &( Params->BinThresh_H), 255,
	//		NULL);

	//cvCreateTrackbar("EyeSize",  WinCon1, &( Params->EyeSize), 100,
	//		NULL);
	cvCreateTrackbar("MaxFishArea",  WinCon1, &( Params->AreaUpBound), 7000,
		NULL);
	cvCreateTrackbar("MinFishArea",  WinCon1, &( Params->AreaBottomBound), 5000,
		NULL);
	cvCreateTrackbar("MaxFishLength",  WinCon1, &( Params->LengthUpBound), 700,
		NULL);
	cvCreateTrackbar("MinFishLength",  WinCon1, &( Params->LengthBottomBound), 300,
		NULL);
	cvCreateTrackbar("CornerWidth",  WinCon1, &( Params->CornerWidth), 500,
		NULL);

	//cvCreateTrackbar("MinEyeArea",  WinCon1, &( Params->Area_S), 5000,
	//		NULL);

	/** Record Data **/
	cvCreateTrackbar("RecordOn",  WinCon1, &( Params->Record), 1,
		(int)NULL);

	//cvCreateTrackbar("Diameter",  WinCon1, &( Params->MaskDiameter),NSIZEY,NULL);

	//if ( stageIsPresent) {
	//	cvCreateTrackbar("StageSpeedFactor",  WinCon1,
	//			&( Params->stageSpeedFactor), 100, NULL);
	//}

	//if ( stageIsPresent) {
	//	cvCreateTrackbar("MaxStageSpeed",  WinCon1, &( Params->maxstagespeed),10,NULL);
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
	cvSetTrackbarPos("LowThreshold",  WinCon1, ( Params->BinThresh_L));
	cvSetTrackbarPos("LowestThreshold",  WinCon1, ( Params->BinThresh_LL));
	cvSetTrackbarPos("HighThreshold",  WinCon1, ( Params->BinThresh_H));
	cvSetTrackbarPos("Gauss=x*2+1",  WinCon1,  Params->GaussSize);
	cvSetTrackbarPos("EyeSize",  WinCon1,  Params->EyeSize);
	cvSetTrackbarPos("MaxEyeArea",  WinCon1,  Params->Area_L);
	cvSetTrackbarPos("MinEyeArea",  WinCon1,  Params->Area_S);

	//cvSetTrackbarPos("Diameter",  WinCon1, ( Params->MaskDiameter));

	cvSetTrackbarPos("On",  WinCon1, ( Params->OnOff));

	cvSetTrackbarPos("SelectDisplay",  WinCon1, ( Params->Display));
	cvSetTrackbarPos("RecordOn",  WinCon1, ( Params->Record));

	/**Stage Speed **/
	//if ( stageIsPresent){
	//	cvSetTrackbarPos("StageSpeedFactor",  WinCon1,
	//			( Params->stageSpeedFactor));

	//	cvSetTrackbarPos("MaxStageSpeed",  WinCon1,
	//					( Params->maxstagespeed));


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

	if ( VidFromFile) { /** Use source from file **/
							/** Define the File catpure **/
		 capture = cvCreateFileCapture( infname);

	}
	else {
		/** Use source from camera **/
		if ( UseFrameGrabber) {
			// fg = TurnOnFrameGrabber();

			//printf("Checking frame size of frame grabber..\n");
			/** Check to see that our image sizes are all the same. **/
			//if ((int)  fg->xsize !=  fromCCD->size.width
			//		|| (int)  fg->ysize !=  fromCCD->size.height) {
			//	printf("Error in RollVideoInput!\n");
			//	printf(
			//			"Size from framegrabber does not match size in IplImage fromCCD!\n");
			//	printf("  fg->xsize=%d\n", (int)  fg->xsize);
			//	printf("  fromCCD->size.width=%d\n",
			//			 fromCCD->size.width);
			//	printf("  fg->ysize=%d\n", (int)  fg->ysize);
			//	printf("  fromCCD->size.height=%d\n",
			//			 fromCCD->size.height);
			//	 e = EXP_ERROR;
			//	return;
			//}

			//printf("Frame size checks out..");

			/**Use Frame Grabber **/
		}
		else {
			/** Use Basler USB Camera **/

			/** Turn on Camera **/

			if (T2Cam_Initialize( MyCamera) != EXP_SUCCESS)  e = EXP_ERROR;


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


	 CurrentSelectedImg = cvCreateImage(cvSize(NSIZEX, NSIZEY), IPL_DEPTH_8U, 1);

	 SubSampled = SubSampled;
	 HUDS = HUDS;

	/*** Create Frames **/
	Frame* fromCCD = CreateFrame(cvSize(NSIZEX, NSIZEY));

	CamData* MyCamera = T2Cam_CreateCamData();

	 MyCamera = MyCamera;

	 fromCCD = fromCCD;

	/** Create Fish Data Struct and Fish Parameter Struct **/
	FishAnalysisData* Fish = CreateFishAnalysisDataStruct();
	FishAnalysisParam* Params = CreateFishAnalysisParam();
	InitializeEmptyFishImages(Fish, cvSize(NSIZEX, NSIZEY));
	InitializeFishMemStorage(Fish);

	 Fish = Fish;
	 Params = Params;

	/** Create API shared memory for fish feature vectors analysis and behavioral prediction **/
	 sm = MC_API_StartServer();

}

/*
* Free up all of the different allocated memory for the
* experiment.
*
*/
void ReleaseExperiment(Experiment* exp) {
	/** Free up Frames **/
	if ( fromCCD != NULL)
		DestroyFrame(&( fromCCD));

	/** Free up Strings **/
	 dirname = NULL;
	 infname = NULL;
	 outfname = NULL;

	/** Free up Fish Objects **/
	if ( Fish != NULL) {
		DestroyFishAnalysisDataStruct(( Fish));
		 Fish = NULL;
	}

	if ( Params != NULL) {
		DestroyFishAnalysisParam(( Params));
		 Params = NULL;
	}

	/** Free up internal iplImages **/
	if ( CurrentSelectedImg != NULL)
		cvReleaseImage(&( CurrentSelectedImg));
	if ( SubSampled != NULL)
		cvReleaseImage(&( SubSampled));
	if ( HUDS != NULL)
		cvReleaseImage(&( HUDS));

	if ( fromCCD != NULL)
		DestroyFrame(&( fromCCD));

	/** Stop MindControl API Shared Memory Server **/
	if ( sm != NULL) {
		MC_API_StopServer( sm);
		 sm = NULL;
	}

	if ( VidFromFile) {
		cvReleaseCapture(&( capture));
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

	if (!( VidFromFile)) {

		if ( UseFrameGrabber) {
			/** Use BitFlow SDK to acquire from Frame Grabber **/
			//if (AcquireFrame( fg)==T2FG_ERROR){
			//	return EXP_ERROR;
			//}

			/** Check to see if file sizes match **/

			//LoadFrameWithBin( fg->HostBuf,  fromCCD);

		}
		else {


			/** Acqure from ImagingSource USB Cam **/

			if (T2Cam_GrabFrame( MyCamera) == EXP_SUCCESS) {

				if ((int) MyCamera->grabResult.SizeX !=  fromCCD->size.width || (int) MyCamera->grabResult.SizeY !=  fromCCD->size.height) {


					printf("Size from Camera does not match size in IplImage fromCCD!\n");

					return EXP_ERROR;
				}

				LoadFrameWithBin( MyCamera->ImageRawData,  fromCCD);
				 Fish->timestamp_on_camera =  MyCamera->grabResult.TimeStamp;


			}

			else return EXP_ERROR;



		}


	}
	else {

		/** Acquire  from file **/

		IplImage* tempImg;
		/** Grab the frame from the video **/


		tempImg = cvQueryFrame( capture);



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
		LoadFrameWithImage(tempImgGray,  fromCCD);
		cvReleaseImage(&tempImgGray);
		//cvReleaseImage(&tempImg);
		/*
		* Note: for some reason thinks crash when you go cvReleaseImage(&tempImg)
		* And there don't seem to be memory leaks if you leave it. So I'm going to leave it in place.
		*
		*/
	}

	 Fish->frameNum++;
	return EXP_SUCCESS;
}

/*
* Is a frame ready from the camera?
*
*/
int isFrameReady(Experiment* exp) {

	if (!( VidFromFile) && !( UseFrameGrabber)) {
		/** If This isn't a simulation.. **/
		/** And if we arent using the frame grabber **/
		return 1;
	}
	else {
		/** Otherwise just keep chugging... **/

		/** Unless we're reading from video, in which case we should fake like we're waiting for something **/
		if ( VidFromFile)
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
	 prevTime = clock();
	 prevFrames = 0;

}

/*
* If more than a second has elapsed
* Calculate the frame rate and print i tout
*
*/
void CalculateAndPrintFrameRate(Experiment* exp) {
	/*** Print out Frame Rate ***/
	if (( Fish->timestamp -  prevTime) > CLOCKS_PER_SEC) {
		printf("%d fps\n",  Fish->frameNum -  prevFrames);
		 prevFrames =  Fish->frameNum;
		 prevTime =  Fish->timestamp;
		//printf("crossed_angle is %d \n", Fish->Eyes->mean_CrossedAngle);
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


	//TICTOC::timer().tic("_FindFishCenter", e);

	//if (!( e))

	//	 e = FindFishCenter( Fish,  Params);


	//TICTOC::timer().toc("_FindFishCenter", e);


	TICTOC::timer().tic("_SegmentingEye",  e);

	if (!( e))

		 e = SegmentingEye( Fish,  Params);


	TICTOC::timer().toc("_SegmentingEye",  e);




	//_TICTOC_TOC_FUNC
}

/*
* Prepare the Selected Display
*
*/
void PrepareSelectedDisplay(Experiment* exp) {
	/** There are no errors and we are displaying a frame **/
	switch ( Params->Display) {
	case 0:
		//
		// CurrentSelectedImg =  Fish->ImgOrig;
		cvShowImage( WinDisp,  Fish->ImgOrig);

		break;
	case 2:
		// CurrentSelectedImg =  Fish->ImgThresh;
		cvShowImage( WinDisp,  Fish->ImgThresh);
		break;
	case 1:
		cvShowImage( WinDisp,  HUDS);
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
		//	Increment(&( Params->BinThresh), 200);
		//	break;
		//case '[':
		//	Decrement(&( Params->BinThresh), 0);
		//	break;

	case 'r': /** record **/
		Toggle(&( Params->Record));
		if ( Params->Record == 0) {
			printf("Turning Record off!\n");
		}
		else {
			printf("Turning Record on!\n");
		}
		break;

		/** Tracker **/
		//case '\t':
		//	Toggle(&( Params->stageTrackingOn));
		//	if ( Params->stageTrackingOn == 0) {
		/** If we are turning the stage off, let the rest of the code know **/
		//		printf("Turning tracking off!\n");
		//		 stageIsTurningOff = 1;
		//	} else {
		//		printf("Turning tracking on!\n");
		//	}
		//	break;

	case 'o':
		Toggle(&( Params->OnOff));
		if ( Params->OnOff == 0) {
			printf("Turning Analysis off!\n");
		}
		else {
			printf("Turning Analysis on!\n");
		}
		break;

		//case '+':
		//	Increment(&( Params->stageSpeedFactor), 50);
		//	printf("stageSpeedFactor=%d\n",  Params->stageSpeedFactor);
		//	break;
		//case '-':
		//	Decrement(&( Params->stageSpeedFactor), 0);
		//	printf("stageSpeedFactor=%d\n",  Params->stageSpeedFactor);
		//	break;

	case 127: /** Delete key **/
	case 8: /** Backspace key **/
			//	 Params->stageTrackingOn = 0;
			//	 stageIsTurningOff = 1;
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
	MC_API_SetCurrentFrame( sm,  Fish->frameNum);
	MC_API_SetTailBendingAngle( sm,  Fish->BendingAngle);



	return;

}

/*********************** RECORDING *******************/

/*
* Sets up data recording and video recording
* Will record video if  RECORDVID is 1
* and record data if  RECORDDATA is 1
*
*/
int SetupRecording(Experiment* exp) {

	printf("About to setup recording\n");
	char* DataFileName;
	if ( RECORDDATA) {
		if ( dirname == NULL ||  outfname == NULL)
			printf(" dirname or  outfname is NULL!\n");

		/** Setup Writing and Write Out Comments **/
		 DataWriter = SetUpWriteToDisk( dirname,  outfname,  Fish->MemStorage);

		/** We should Quit Now if any of the data Writing is not working **/
		if ( DataWriter->error < 0) return -1;

		/** Write the Command Line argument Out for reference **/
		WriteOutCommandLineArguments( DataWriter,  argc,  argv);

		/**  Write out the default grid size for non-protocol based illumination **/
		//WriteOutDefaultGridSize( DataWriter,  Params);

		/** Write the Protocol Out for reference **/
		//if ( pflag) {
		//	WriteProtocol( p,  DataWriter->fs);
		//}

		BeginToWriteOutFrames( DataWriter);

		printf("Initialized data recording\n");
		DestroyFilename(&DataFileName);
	}

	/** Set Up Video Recording **/
	char* MovieFileName;
	char* HUDSFileName;

	if ( RECORDVID) {
		if ( dirname == NULL ||  outfname == NULL)
			printf(" dirname or  outfname is NULL!\n");

		MovieFileName = CreateFileName( dirname,  outfname, ".avi");
		HUDSFileName = CreateFileName( dirname,  outfname, "_HUDS.avi");

		 Vid = cvCreateVideoWriter(MovieFileName,
			CV_FOURCC('M', 'J', 'P', 'G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
			0);
		 VidHUDS = cvCreateVideoWriter(HUDSFileName,
			CV_FOURCC('M', 'J', 'P', 'G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
			0);
		if ( Vid == NULL) printf("\tERROR in SetupRecording!  Vid is NULL\n");
		if ( VidHUDS == NULL) printf("\tERROR in SetupRecording!  VidHUDS is NULL\n");
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
	if ( Vid != NULL)
		cvReleaseVideoWriter(&( Vid));
	if ( VidHUDS != NULL)
		cvReleaseVideoWriter(&( VidHUDS));

	/** Finish Writing to Disk **/
	if ( RECORDDATA)
		FinishWriteToDisk(&( DataWriter));

}




/*
* Write video and data to Disk
*
*/
void DoWriteToDisk(Experiment* exp) {

	/** Throw error if the user has asked to record, but the system is not in record mode **/
	if ( Params->Record && ( RECORDVID != 1)) {
		printf("ERROR!! THE SYSTEM IS NOT IN RECORD MODE!\n");
		printf("restart the system to record.\n");
	}

	/** Record VideoFrame to Disk**/
	if ( RECORDVID &&  Params->Record) {
		TICTOC::timer().tic("cvResize");
		cvResize( Fish->ImgOrig,  SubSampled, CV_INTER_LINEAR);
		TICTOC::timer().toc("cvResize");

		TICTOC::timer().tic("cvWriteFrame");
		cvWriteFrame( Vid,  SubSampled);
		//cvWriteFrame( Vid,  Fish->ImgOrig);
		if ( Vid == NULL) printf("\tERROR in DoWriteToDisk!\n\t Vid is NULL\n");
		if ( SubSampled == NULL) printf("\tERROR in DoWriteToDisk!\n\t  Subsampled==NULL\n");

		TICTOC::timer().toc("cvWriteFrame");

		cvResize( HUDS,  SubSampled, CV_INTER_LINEAR);
		if ( VidHUDS == NULL) printf("\tERROR in DoWriteToDisk!\n\t VidHUDS is NULL\n");
		if ( SubSampled == NULL) printf("\tERROR in DoWriteToDisk!\n\t  Subsampled==NULL\n");

		cvWriteFrame( VidHUDS,  SubSampled);
	}

	/** Record data frame to diskl **/

	if ( RECORDDATA &&  Params->Record) {
		TICTOC::timer().tic("AppendFishFrameToDisk");
		AppendFishFrameToDisk( Fish,  Params,  DataWriter);
		TICTOC::timer().toc("AppendFishFrameToDisk");
	}
}
