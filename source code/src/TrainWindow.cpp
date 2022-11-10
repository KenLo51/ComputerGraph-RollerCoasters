/************************************************************************
     File:        TrainWindow.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						this class defines the window in which the project 
						runs - its the outer windows that contain all of 
						the widgets, including the "TrainView" which has the 
						actual OpenGL window in which the train is drawn

						You might want to modify this class to add new widgets
						for controlling	your train

						This takes care of lots of things - including installing 
						itself into the FlTk "idle" loop so that we get periodic 
						updates (if we're running the train).


     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <FL/fl.h>
#include <FL/Fl_Box.h>
#include <Fl/Fl.H>

// for using the real time clock
#include <time.h>

#include "TrainWindow.H"
#include "TrainView.H"
#include "CallBacks.H"



//************************************************************************
//
// * Constructor
//========================================================================
TrainWindow::
TrainWindow(const int x, const int y) 
	: Fl_Double_Window(x,y,800,600,"Train and Roller Coaster")
//========================================================================
{
	// make all of the widgets
	begin();	// add to this widget
	{
		int pty=5;			// where the last widgets were drawn
		
		trainView = new TrainView(5,5,590,590);
		trainView->tw = this;
		trainView->m_pTrack = &m_Track;
		this->resizable(trainView);

		// to make resizing work better, put all the widgets in a group
		widgets = new Fl_Group(600,5,190,590);
		widgets->begin();

		runButton = new Fl_Button(605,pty,60,20,"Run");
		togglify(runButton);

		Fl_Button* fb = new Fl_Button(700,pty,25,20,"@>>");
		fb->callback((Fl_Callback*)forwCB,this);
		Fl_Button* rb = new Fl_Button(670,pty,25,20,"@<<");
		rb->callback((Fl_Callback*)backCB,this);
		
		arcLength = new Fl_Button(730,pty,65,20,"ArcLength");
		togglify(arcLength,1);
  
		pty+=25;
		speed = new Fl_Value_Slider(655,pty,140,20,"speed");
		speed->range(0,50);
		speed->value(2);
		speed->align(FL_ALIGN_LEFT);
		speed->type(FL_HORIZONTAL);

		pty += 30;

		// camera buttons - in a radio button group
		Fl_Group* camGroup = new Fl_Group(600,pty,195,20);
		camGroup->begin();
		worldCam = new Fl_Button(605, pty, 60, 20, "World");
        worldCam->type(FL_RADIO_BUTTON);		// radio button
        worldCam->value(1);			// turned on
        worldCam->selection_color((Fl_Color)3); // yellow when pressed
		worldCam->callback((Fl_Callback*)damageCB,this);
		trainCam = new Fl_Button(670, pty, 60, 20, "Train");
        trainCam->type(FL_RADIO_BUTTON);
        trainCam->value(0);
        trainCam->selection_color((Fl_Color)3);
		trainCam->callback((Fl_Callback*)damageCB,this);
		topCam = new Fl_Button(735, pty, 60, 20, "Top");
        topCam->type(FL_RADIO_BUTTON);
        topCam->value(0);
        topCam->selection_color((Fl_Color)3);
		topCam->callback((Fl_Callback*)damageCB,this);
		camGroup->end();

		pty += 30;

		// browser to select spline types
		// TODO: make sure these choices are the same as what the code supports
		splineBrowser = new Fl_Browser(605,pty,120,75,"Spline Type");
		splineBrowser->type(2);		// select
		splineBrowser->callback((Fl_Callback*)splineChangedCB,this);
		splineBrowser->add("Linear");
		splineBrowser->add("Cardinal Cubic");
		splineBrowser->add("Cubic B-Spline");
		splineBrowser->select(2);

		pty += 90;
		AdaptiveSubdivisionButton = new Fl_Button(605, pty, 150, 20, "Adaptive Subdivision");
		AdaptiveSubdivisionButton->type(FL_TOGGLE_BUTTON);
		AdaptiveSubdivisionButton->selection_color((Fl_Color)3);
		AdaptiveSubdivisionButton->callback((Fl_Callback*)splineChangedCB, this);

		ShowAdpsubButton = new Fl_Button(760, pty, 35, 20, "Show");
		ShowAdpsubButton->type(FL_TOGGLE_BUTTON);
		ShowAdpsubButton->selection_color((Fl_Color)3);
		ShowAdpsubButton->callback((Fl_Callback*)trainViewRedraw, this);

		pty += 35;

		// add and delete points
		Fl_Button* ap = new Fl_Button(605,pty,80,20,"Add Point");
		ap->callback((Fl_Callback*)addPointCB,this);
		Fl_Button* dp = new Fl_Button(690,pty,80,20,"Delete Point");
		dp->callback((Fl_Callback*)deletePointCB,this);

		pty += 25;
		// reset the points
		resetButton = new Fl_Button(735,pty,60,20,"Reset");
		resetButton->callback((Fl_Callback*)resetCB,this);
		Fl_Button* loadb = new Fl_Button(605,pty,60,20,"Load");
		loadb->callback((Fl_Callback*) loadCB, this);
		Fl_Button* saveb = new Fl_Button(670,pty,60,20,"Save");
		saveb->callback((Fl_Callback*) saveCB, this);

		pty += 25;
		tensionSlider = new Fl_Value_Slider(655, pty, 140, 20, "tension");
		tensionSlider->range(-1, 1);
		tensionSlider->value(0);
		tensionSlider->align(FL_ALIGN_LEFT);
		tensionSlider->type(FL_HORIZONTAL);
		tensionSlider->callback((Fl_Callback*)splineChangedCB, this);

		pty += 25;
		// roll the points
		Fl_Button* rx = new Fl_Button(605,pty,30,20,"R+X");
		rx->callback((Fl_Callback*)rpxCB,this);
		Fl_Button* rxp = new Fl_Button(635,pty,30,20,"R-X");
		rxp->callback((Fl_Callback*)rmxCB,this);
		Fl_Button* rz = new Fl_Button(670,pty,30,20,"R+Z");
		rz->callback((Fl_Callback*)rpzCB,this);
		Fl_Button* rzp = new Fl_Button(700,pty,30,20,"R-Z");
		rzp->callback((Fl_Callback*)rmzCB,this);

		pty+=30;
		carsNumSpinner = new Fl_Spinner(635, pty, 60, 20, "Cars");
		carsNumSpinner->range(0, 20);
		carsNumSpinner->value(0);
		carsNumSpinner->callback((Fl_Callback*)carNumChange, this);

		headlightButton = new Fl_Button(705, pty, 100, 20, "Headlight");
		headlightButton->type(FL_TOGGLE_BUTTON);
		headlightButton->selection_color((Fl_Color)3);
		headlightButton->callback((Fl_Callback*)trainViewRedraw, this);

		pty += 30;
		physicsButton = new Fl_Button(600, pty, 100, 20, "Physics");
		physicsButton->type(FL_TOGGLE_BUTTON);
		physicsButton->selection_color((Fl_Color)3);
		physicsButton->callback((Fl_Callback*)physicsButtonCB, this);

		smokeButton = new Fl_Button(705, pty, 100, 20, "Smoke");
		smokeButton->type(FL_TOGGLE_BUTTON);
		smokeButton->selection_color((Fl_Color)3);

		pty += 30;
		SunDegree1 = new Fl_Value_Slider(655, pty, 140, 20, "Sun θ");
		SunDegree1->range(0, 360);
		SunDegree1->value(180);
		SunDegree1->align(FL_ALIGN_LEFT);
		SunDegree1->type(FL_HORIZONTAL);
		SunDegree1->callback((Fl_Callback*)carNumChange, this);

		pty += 30;
		SunDegree2 = new Fl_Value_Slider(655, pty, 140, 20, "Sun ϕ");
		SunDegree2->range(0, 90);
		SunDegree2->value(30);
		SunDegree2->align(FL_ALIGN_LEFT);
		SunDegree2->type(FL_HORIZONTAL);
		SunDegree2->callback((Fl_Callback*)trainViewRedraw, this);


		pty += 30;

		// TODO: add widgets for all of your fancier features here
#ifdef EXAMPLE_SOLUTION
		makeExampleWidgets(this,pty);
#endif
		trainView->updateTrackSpline();
		trainView->trainMove(0.0f);
		prevSpeed = 0;

		// we need to make a little phantom widget to have things resize correctly
		Fl_Box* resizebox = new Fl_Box(600,595,200,5);
		widgets->resizable(resizebox);

		widgets->end();
	}
	end();	// done adding to this widget

	// set up callback on idle
	Fl::add_timeout(1.0 / 30.0, (Fl_Timeout_Handler)runTimerCB, (void*)this);
	//Fl::add_idle((void (*)(void*))runButtonCB,this);

}

//************************************************************************
//
// * handy utility to make a button into a toggle
//========================================================================
void TrainWindow::
togglify(Fl_Button* b, int val)
//========================================================================
{
	b->type(FL_TOGGLE_BUTTON);		// toggle
	b->value(val);		// turned off
	b->selection_color((Fl_Color)3); // yellow when pressed	
	b->callback((Fl_Callback*)damageCB,this);
}

//************************************************************************
//
// *
//========================================================================
void TrainWindow::
damageMe()
//========================================================================
{
	if (trainView->selectedCube >= ((int)m_Track.points.size()))
		trainView->selectedCube = 0;
	trainView->damage(1);
}

//************************************************************************
//
// * This will get called (approximately) 30 times per second
//   if the run button is pressed
//========================================================================
void TrainWindow::
advanceTrain(float dir, int mode, float DeltaTime)
//========================================================================
{
	//#####################################################################
	// TODO: make this work for your train
	//#####################################################################
#ifdef EXAMPLE_SOLUTION
	// note - we give a little bit more example code here than normal,
	// so you can see how this works

	if (arcLength->value()) {
		float vel = ew.physics->value() ? physicsSpeed(this) : dir * (float)speed->value();
		world.trainU += arclenVtoV(world.trainU, vel, this);
	} else {
		world.trainU +=  dir * ((float)speed->value() * .1f);
	}

	float nct = static_cast<float>(world.points.size());
	if (world.trainU > nct) world.trainU -= nct;
	if (world.trainU < 0) world.trainU += nct;
#endif
	if (mode == 1) {
		trainView->trainMove(dir* 5);
		return;
	}

	if (mode == 0) {
		if (arcLength->value()) {
			float targetSpeed = speed->value() / 30.0f * dir;
			float nowSpeed = targetSpeed;
			if (physicsButton->value()) {

				float slop = glm::normalize(trainView->trainModel->directions[0]).y;
				float force = targetSpeed * 10.0f;
				if (speed->value() == 0.0f)//brakes
					force = signbit(prevSpeed) ? 2.0f: -2.0f;
				float gravity = 0.98f;
				float mass = 40.0f;
				float drag = prevSpeed * 10.0f;
				float totalForce = force - gravity * mass * slop - drag;

				float acc = totalForce / mass;

				//if (speed->value() == 0.0f) {//brakes
				//	if (acc < 0 && (-acc) > prevSpeed)	nowSpeed = 0.0f;
				//	else if (acc > 0 && acc < (-prevSpeed))	nowSpeed = 0.0f;
				//	else nowSpeed = prevSpeed + acc * DeltaTime;
				//}
				//else {
				//	nowSpeed = prevSpeed + acc * DeltaTime;
				//}
				nowSpeed = prevSpeed + acc * DeltaTime;
				prevSpeed = nowSpeed;
			}
			else {
				nowSpeed = targetSpeed;
			}

			trainView->trainMove(nowSpeed);
			prevSpeed = nowSpeed;
		}
		else {
			float moveLength = glm::length(trainView->trackSplineDirect[trainView->trainControl->GetIndex()]);
			trainView->trainMove(moveLength * speed->value() * dir * DeltaTime);

			if (physicsButton->value()) {
				physicsButton->value(0);
			}
		}
	}
	trainView->smokeAnimation->timeAdd(DeltaTime);
	if ((clock() - lastSmokeTime) > (5000 / speed->value())) {
		lastSmokeTime = clock();
		if (smokeButton->value()) {

			trainView->smokeAnimation->addInstance(trainView->trainModel->transforms[0]);
		}
	}
}
