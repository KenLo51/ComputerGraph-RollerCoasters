/************************************************************************
     File:        TrainView.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#pragma once

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <vector>
#include <Fl/Fl_Gl_Window.h>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
//#include <glm/gtx/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.h"
#include "animation.h"
#pragma warning(pop)

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"


class CaronTrack {
public:
	CaronTrack(ModelClass* targetModel);
	void UpdateModel(ModelClass* targetModel);
	void UpdateTruckParameter(std::vector<glm::vec3>* positions, std::vector<glm::vec3>* directions, std::vector<glm::vec3>* crosses, std::vector<float>* lengths);
	void Move(float distance, unsigned int instanceIdx = 0, unsigned int interpolateMode = 0);
	void ResetProcess();
	float GetProcess();
	unsigned int GetIndex();
	void SetProcess(float val);
private:
	std::vector<float>* trackLength;
	std::vector<glm::vec3>* trackPosition;
	std::vector<glm::vec3>* trackDirect;
	std::vector<glm::vec3>* trackCross;
	ModelClass* model;
	float runProcess;
	unsigned int runSplineIdx;
};

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//
		void drawTrack(bool doingShadows = false);
		void trackLinear(bool doingShadows = false);
		void trackBSpline(bool doingShadows = false);
		void trackCardinal(bool doingShadows = false);

		void updateTrackSpline();

		void drawTrackSpline(glm::mat4& splineMat, unsigned int divide_line, bool doingShadows = false);
		std::vector<glm::vec3> spline(glm::mat4& splineMat, std::vector<glm::vec3>& vertices, unsigned int divide_line);
		void drawSpline(glm::mat4& splineMat, std::vector<glm::vec3>& vertices, bool doingShadows = false);
		void drawlinesloop(std::vector<glm::vec3>& vertices, bool doingShadows = false);
		void drawlinesloopBox(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& direct, std::vector<glm::vec3>& cross , bool doingShadows = false);

		void trainMove(float distance);
		void trainReset();
		void setCars(unsigned int num);

		void buildTrackModel(std::vector<glm::vec3>& positions1, std::vector<glm::vec3>& positions2, 
			std::vector<glm::vec3>& crosses, std::vector<glm::vec3>& directs);

		void initTrees();

	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube;  // simple - just remember which cube is selected

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene

		float trackWidth;
		float trackLength;
		std::vector<glm::vec3> leftTrackSplinePos;
		std::vector<glm::vec3> rightTrackSplinePos;
		std::vector<glm::vec3> trackSplinePos;
		std::vector<glm::vec3> trackSplineDirect;
		std::vector<glm::vec3> trackSplineCross;
		std::vector<float> trackSplineLength;

		CaronTrack* trainControl;
		std::vector < CaronTrack* > carControl;

		ModelClass* trainModel;
		ModelClass* headlightModel;
		ModelClass* carModel;
		ModelClass* sleeperModel;
		ModelClass* trackModel;

		std::vector < ModelClass* > smokeFrames;
		Animation* smokeAnimation;

		ModelClass* treeAModel;

		glm::vec3 sunlightPos;
};