#pragma once
#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

#ifndef FISHANALYSIS_H_
#error "#include FishAnalysis.h" must appear in source files before "#include experiment.h"
#endif

#ifndef TALK2CAMERA_H_
#error "#include Talk2Camera.h" must appear in source files before "#include experiment.h"
#endif

//#ifndef TALK2FRAMEGRABBER_H_
// #error "#include Talk2FrameGrabber.h" must appear in source files before "#include experiment.h"
//#endif

#ifndef WRITEOUTFISH_H_
#error "#include WriteOutFish.h" must appear in source files before "#include experiment.h"
#endif

//#ifndef TALK2STAGE_H_
// #error "#include Talk2Stage.h" must appear in source files before "#include experiment.h"
//#endif



#define EXP_ERROR -1
#define EXP_SUCCESS 0
#define EXP_VIDEO_RAN_OUT 1

#define NSIZEX 512
#define NSIZEY 512


class Experiment {

private:


public:

	
	int VidFromFile; // 1 =Video from File, 0=Video From Camera

					 /** GuiWindowNames **/
	char* WinDisp;
	char* WinCon1;

	/** CommandLine Input **/
	char** argv;
	int argc;
	char* dirname;
	char* outfname;
	char* infname;

	/** FrameGrabber Input**/
	//FrameGrabber* fg;
	bool UseFrameGrabber;//framegrabber is about -g option, whether use the high resolution camera;

	/** Video Capture (for simulation mode) **/
	//use video capture class to deal with the video;
	VideoCapture capture;

	/** Camera Input **/
	CamData* MyCamera;

	/** MostRecently Observed CameraFrameNumber **/
	unsigned long lastFrameSeenOutside;

	/** User-configurable Fish-related Parameters **/
	FishAnalysisParam Params;

	/** Information about Our Fish **/
	FishAnalysisData Fish;

	/** internal IplImage **/
	Mat SubSampled;
	Mat HUDS;
	Mat CurrentSelectedImg;
	//IplImage* SubSampled; // Image used to subsample stuff
	//IplImage* HUDS;  //Image used to generate the Heads Up Display
	//IplImage* CurrentSelectedImg;

	/** Internal Frame data types **/
	Frame* fromCCD;


	/** Write Data To File **/
	WriteOut* DataWriter;

	/** Write Video To File **/
	VideoWriter Vid;  //Video Writer
	VideoWriter VidHUDS;


	/** Timing  Information **/
	clock_t now;
	clock_t last;


	/** Frame Rate Information **/
	int nframes;
	int prevFrames;
	long prevTime;

	/** Macros **/
	int RECORDVID;
	int RECORDDATA;

	/** MindControl API **/
	SharedMemory_handle sm;



	/** Stage Control **/
	//int stageIsPresent;
	//TaskHandle stage; // Handle to stage object
	//CvPoint stageVel; //Current velocity of stage
	//CvPoint stageCenter; // Point indicating center of stage.
	//CvPoint stageFeedbackTargetOffset; //Target of the stage feedback loop as a delta distance in pixels from the center of the image
	//int stageIsTurningOff; //1 indicates stage is turning off. 0 indicates stage is on or off.

	/** Error Handling **/
	int e;

	/*
	
	listed above is main value used in the whole loop
	
	
	
	*/
	/*
	* Creates a new experiment object and sets values to zero.
	*/
	/*
	* This function allocates images and frames
	* And a Fish Object
	*
	* And a Parameter Object
	* For internal manipulation
	*
	*
	*/
	Experiment ();

	/*
	* Free up all of the different allocated memory for the
	* experiment.
	*
	*/
	~Experiment();

	/*
	* Load the command line arguments into the experiment object
	*/
	void LoadCommandLineArguments(int argc, char** argv);

	/*
	* Handle CommandLine Arguments
	* Parses commandline arguments.
	* Decides if user wants to record video or recorddata
	*/
	int HandleCommandLineArguments();


	/*
	* Initialize camera library
	* Allocate Camera Data
	* Select Camera and Show Properties dialog box
	* Start Grabbing Frames as quickly as possible
	*
	* OR open up the video file for reading.
	*/

	void RollCameraInput();


	/*
	* Setsup data recording and video recording
	* Will record video if exp->RECORDVID is 1
	* and record data if exp->RECORDDATA is 1
	*
	*/


	int SetupRecording();

	/************************************************/
	/*   Frame Rate Routines
	*
	*/
	/************************************************/

	/*
	*This is the frame rate timer.
	*/


	void StartFrameRateTimer();


	/*
	* If more than a second has elapsed
	* Calculate the frame rate and print it out
	*
	*/

	void CalculateAndPrintFrameRate();

	/* Assigns Default window names to the experiment object
	*
	*/
	void AssignWindowNames();

	/** Grab a Frame from either camera or video source
	*
	*/
	int GrabFrame();

	void CalculateAndPrintFrameRate();



	//fish img is loaded into fishptr->fromCCD,
	int LoadFishImg();

	//this part is about analysis the fish
	int FindFishBoundary();

	void find_fish_head();

	void cvThin();//the function may be changed 

	void CreateFishHUDS();

	void DoWriteToDisk();
	/*
	* Finish writing video and  and data
	* and release
	*
	*/
	void FinishRecording();

	/*
	* Release the memopry associated with window names
	* and set their pointers to Null
	*/
	void ReleaseWindowNames(Experiment* exp);

} Experiment;



//other functions listed below is about display thread, I don't want to change it 


void displayhelp();

/*
* SetupGui
*
*/
void SetupGUI(Experiment exp);

/*
* Update's trackbar positions for variables that can be changed by the software
*
*/
void UpdateGUI(Experiment exp);

/*********************************************
*
* Image Acquisition
*
*/

/*
* Is a frame ready from the camera?
*/
int isFrameReady(Experiment exp);



/************************************************/
/*   Action Chunks
*
*/
/************************************************/

/*
* Given an image in teh Fish object, segment the Fish
*
*/



void PrepareSelectedDisplay(Experiment exp);


/*
*
* Handle KeyStroke
*
* Returns 1 when the user is trying to exit
*
*/
int HandleKeyStroke(int c, Experiment exp);




/*
* Setsup data recording and video recording
* Will record video if exp->RECORDVID is 1
* and record data if exp->RECORDDATA is 1
*
*/
int SetupRecording(Experiment exp);






/**************************************************
* Stage Tracking and FEedback System
*
* This should really probably go in a special library called Stage Tracking
* that depends on both OpenCV AND Talk2STage.c, but its a huge pain to modify the makefile
* to create a new library that has only one function in it.
*
* Alternatively this could conceivably go in Talk2Stage.c, but then I find it weird
* that Talk2Stage.c should depend on OpenCV, because ultimatley it should be more general.
*
* It doesn't really belong in experiment.c either because it is not a method of experiment.c
* But for now that is where it will sit.
*
*/

/*
* Scan for the USB device.
*/
//void InvokeStage(Experiment* exp);

/*
* Update the Stage Tracker.
* If the Stage tracker is not initialized, don't do anything.
* If the stage tracker is initialized then either do the tracking,
* or if we are in the process of turning off tracking off, then tell
* the stage to halt and update flags.
*/
//int HandleStageTracker(Experiment* exp);

//void ShutOffStage(Experiment* exp);

#endif /* EXPERIMENT_H_ */

