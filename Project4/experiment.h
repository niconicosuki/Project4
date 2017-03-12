//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <time.h>

//3rd headers
#include "tictoc.h"

using namespace cv;

struct CamData;

struct WriteOut;

class Frame {};

class FishAnalysisParam {};

class FishAnalysisData {};

class Experiment {

private:


public:

	
	int VidFromFile; // 1 =Video from File, 0=Video From Camera

	 /** GuiWindowNames **/
	char* WinDisp;
	char* WinCon1;


	/** Error Handling **/
	int e;



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
	CamData* MyCamera;// talk to camera,we may change it;

	/** MostRecently Observed CameraFrameNumber **/
	/*
	unsigned long lastFrameSeenOutside;//I think the value may be useless
	*/
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
	/*
	* Frame contains a binary representation and an IplImage representation of a frame.
	* This is useful in conjuncture with the TransformLib.h library that converts
	* from CCD to DLP space.
	*
	*/
	Frame* fromCCD;


	/** Write Data To File(yaml)  **/
	
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
	//SharedMemory_handle sm;
	//may be we can use other functions to replace share_memory?
	

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
	* Will record video if  RECORDVID is 1
	* and record data if  RECORDDATA is 1
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
* Will record video if  RECORDVID is 1
* and record data if  RECORDDATA is 1
*
*/
int SetupRecording(Experiment exp);






#endif /* EXPERIMENT_H_ */

