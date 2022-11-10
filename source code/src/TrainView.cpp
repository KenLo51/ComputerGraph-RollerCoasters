/************************************************************************
     File:        TrainView.cpp

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
#include <iomanip> 
#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "GL/glu.h"

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"


#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif


//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );
	
	resetArcball();

	trainModel = new ModelClass("models/train.obj");
	trainModel->setColor(32, 32, 64);
	headlightModel = new ModelClass("models/headLight.obj");
	headlightModel->setColor(16, 16, 16);
	carModel = new ModelClass("models/car.obj");
	carModel->setColor(32, 64, 64);
	carModel->setInstanceNum(0);
	sleeperModel = new ModelClass("models/sleeper.obj");
	sleeperModel->setColor(12, 12, 6);
	trackModel = new ModelClass();
	trainControl = new CaronTrack(trainModel);
	treeAModel = new ModelClass("models/tree_a.obj");
	treeAModel->setInstanceNum(0);

	const char smokeFrameFiles[][80] = { "models/smoke_0.obj", "models/smoke_1.obj" , "models/smoke_2.obj" , "models/smoke_3.obj" , "models/smoke_4.obj" , "models/smoke_5.obj" };
	for (unsigned int i = 0; i < 6; i++) {
		ModelClass* newFrame = new ModelClass(smokeFrameFiles[i]);
		newFrame->setColor(32, 32, 32);
		smokeFrames.push_back(newFrame);
	}
	const float smokeFrameDelaysVal[] = { 0.2, 0.2, 0.5, 0.5, 0.5, 0.5 };
	std::vector<float> smokeFrameDelays(smokeFrameDelaysVal, smokeFrameDelaysVal + sizeof(smokeFrameDelaysVal)/sizeof(float));
	smokeAnimation = new Animation(smokeFrames, smokeFrameDelays);
	//smokeAnimation->initTransforms(1);

	trackWidth = 5.0f;

	selectedCube = -1;
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event)) 
			return 1;

	// remember what button was used
	static int last_push;

	switch(event) {
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				doPick();
				damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
								static_cast<double>(cp->pos.x), 
								static_cast<double>(cp->pos.y),
								static_cast<double>(cp->pos.z),
								rx, ry, rz,
								(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float) rx;
				cp->pos.y = (float) ry;
				cp->pos.z = (float) rz;

				updateTrackSpline();
				trainReset();
				trainMove(0.0f);
				damage(1);
			}
			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			focus(this);
			break;

		case FL_KEYBOARD:
		 		int k = Fl::event_key();
				int ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				};
				break;
	}

	return Fl_Gl_Window::handle(event);
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0,0,w(),h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0,0,.3f,0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	// top view only needs one light
	//if (tw->topCam->value()) {
	//if(tw->headlightButton->value()){
	//	glDisable(GL_LIGHT1);
	//	glDisable(GL_LIGHT2);
	//} else {
	//	glEnable(GL_LIGHT1);
	//	glEnable(GL_LIGHT2);
	//}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	
	// Directional Light
	glm::vec3 whiteLight;
	GLfloat grayLight[]	= {.1f, .1f, .1f, 1.0};
	sunlightPos = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	sunlightPos = glm::rotateX(sunlightPos, glm::radians((float)tw->SunDegree2->value()));
	sunlightPos = glm::rotateY(sunlightPos, glm::radians((float)tw->SunDegree1->value()));

	whiteLight = glm::vec3(0.5f, 0.5f, 0.5f) * sunlightPos.y + 0.3f;


	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(glm::vec4(sunlightPos, 0.0f)));
	glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(glm::vec4(whiteLight, 1.0f)));
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glm::vec3 subsunlightPos(0.0f, 1.0f, 0.0f);
	subsunlightPos = glm::rotateX(subsunlightPos, glm::radians((float)45));
	subsunlightPos = glm::rotateY(subsunlightPos, glm::radians((float)tw->SunDegree1->value() + 120.0f));
	glEnable(GL_LIGHT3);
	glLightfv(GL_LIGHT3, GL_POSITION, glm::value_ptr(glm::vec4(subsunlightPos, 0.0f)));
	glLightfv(GL_LIGHT3, GL_DIFFUSE, grayLight);

	// Point light
	//if (tw->topCam->value()) {
	//	glDisable(GL_LIGHT1);
	//}
	//else {
	//	GLfloat lightPosition2[] = { 0.0f, 2.0f, 0.0f, 1.0f };
	//	GLfloat yellowLight[] = { 0.5f, 0.5f, 3.0f, 1.0 };
	//	glEnable(GL_LIGHT1);
	//	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	//	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);
	//	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	//	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.07f);
	//	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.017f);
	//}
	if (selectedCube >= 0 ) {
		glm::vec4 lightPosition2 = glm::vec4(m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					1.0f);
		GLfloat yellowLight[] = { 10.0f, 10.0f, 40.0f, 1.0 };
		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_POSITION, glm::value_ptr(lightPosition2));
		glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.22f);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.20f);
	}
	else {
		glDisable(GL_LIGHT1);
	}
	
	// Spot light
	if (tw->headlightButton->value()) {
		GLfloat headlightColor[] = { 40.0f,40.0f,10.0f,1.0 };
		glEnable(GL_LIGHT2);
		glm::vec3 headlightDirection = glm::normalize(trainModel->directions[0]);
		glm::vec4 headlightPosition = glm::vec4(trainModel->positions[0] + 2.0f * headlightDirection + 7.0f * trainModel->ups[0],
			1.0f);
		glLightfv(GL_LIGHT2, GL_POSITION, glm::value_ptr(headlightPosition));
		glLightfv(GL_LIGHT2, GL_DIFFUSE, headlightColor);
		glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, glm::value_ptr(headlightDirection));
		glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 70.0f);

		glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 15.0f);
		glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.07f);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.017f);

		headlightModel->setColor(128, 128, 32);
	}
	else {
		glDisable(GL_LIGHT2);
		headlightModel->setColor(16, 16, 16);
	}

	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);
	setupFloor();
	drawFloor(200,100);

	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	setupObjects();

	drawStuff(false);

	// this time drawing is for shadows (except for top view)
	//if (!tw->topCam->value()) {
	//	setupShadows();
	//	drawStuff(true);
	//	unsetupShadows();
	//}
	if (tw->SunDegree2->value() < 90.0f) {
		//setupShadows();
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
		glStencilMask(0x1);		// only deal with the 1st bit

		glPushMatrix();
		float sm[16] = { 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
		glMultMatrixf(sm);

		glm::mat4 shadowProj = glm::mat4(1.0f);
		float xt = sunlightPos.x / sunlightPos.y;
		float zt = sunlightPos.z / sunlightPos.y;
		shadowProj = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			-xt, 1.0f, -zt, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		glMultMatrixf(glm::value_ptr(shadowProj));
		glColor4f(0, 0, 0, (1.0f - (1.0f-sunlightPos.y)* (1.0f - sunlightPos.y)) * 0.7f);


		// draw shadows
		drawStuff(true);

		//unsetupShadows();
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
	}
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
		float m[] = { 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, -1, 0,
		0, 0, 0, 1 };
		glMultMatrixf(m);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
		if (trainModel != NULL) {
			glMatrixMode(GL_PROJECTION);
			double aspect = ((double)TrainView::w()) / ((double)TrainView::h());
			gluPerspective(120, aspect, .1, 1000);
			glTranslatef(0.0f, -2.0f, 0.0f);
			glm::vec3 eye = glm::vec3(trainModel->positions[0]);
			glm::vec3 center = glm::vec3(eye + trainModel->directions[0]);
			glm::vec3 up = glm::vec3(trainModel->ups[0]);
			gluLookAt(	eye.x, eye.y, eye.z,
				center.x, center.y, center.z,
				up.x, up.y, up.z);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	//std::cout << (int)tw->headlightButton->value() << std::endl;
	if (!tw->trainCam->value()) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif
	drawTrack(doingShadows);


	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
	if (trainModel != NULL) {
		if((tw->trainCam->value() == 0) || doingShadows)
			trainModel->draw(doingShadows);
	}
	if (headlightModel != NULL) {
		if ((tw->trainCam->value() == 0) || doingShadows) {
			if(tw->headlightButton->value()) glDisable(GL_LIGHTING);
			else glEnable(GL_LIGHTING);
			headlightModel->draw(doingShadows);
			glEnable(GL_LIGHTING);
		}
	}
	if (carModel != NULL)
		carModel->draw(doingShadows);

	smokeAnimation->Draw(doingShadows);

	if (treeAModel != NULL) {
		treeAModel->draw(doingShadows);
	}
}
void TrainView::drawTrack(bool doingShadows) {
	//if (trackModel != NULL)
	//	trackModel->draw(doingShadows, GL_QUADS);
	if (trackModel != NULL) {
		for (unsigned int meshIdx = 0; meshIdx < trackModel->meshes.size(); meshIdx++) {
			Mesh& currMesh = trackModel->meshes[meshIdx];

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(trackModel->transforms[0]));
			//glLoadIdentity();

			glBegin(GL_QUADS);
			for (unsigned int i = 0; i < currMesh.posIndices.size(); i++) {
				unsigned int index;

				index = currMesh.normalIndices[i];
				glm::vec3 drawNormal = trackModel->normals[index];
				drawNormal = glm::normalize(drawNormal);
				//glm::vec3 drawNormal = ModelClass::normals[index];
				//std::cout << std::setprecision(2) << drawNormal.x << "\t" << drawNormal.y << "\t" << drawNormal.z << "\n";

				index = currMesh.posIndices[i];
				glm::vec3 drawPos = trackModel->verticesPos[index];
				//glm::vec3 drawPos = ModelClass::transforms[idx] * glm::vec4(ModelClass::verticesPos[index], 1.0f);
				if (!doingShadows) {
					glm::vec3 color = currMesh.color;
					if (tw->ShowAdpsubButton->value()) {
						if ((i % 32) < 16)
							color = color * 1.3f;
						else
							color = color * 0.7f;
					}
					glColor3ub(color.x, color.y, color.z);
				}
				glNormal3f(drawNormal.x, drawNormal.y, drawNormal.z);
				glVertex3f(drawPos.x, drawPos.y, drawPos.z);
			}
			glEnd();
			glPopMatrix();
		}
	}

	if (sleeperModel != NULL)
		sleeperModel->draw(doingShadows);
}


void TrainView::updateTrackSpline() {
	//std::cout << "updateTrackSpline\n";
	glm::mat4 splineMat;
	unsigned int divide_line = 1;

	// init spline matrix
	if (tw->splineBrowser->selected(1)) {// linear
		splineMat = glm::mat4(0, 0, 0, 0,
			0, 0, 0, 0,
			0, -1, 1, 0,
			0, 1, 0, 0);
		divide_line = 1;
	}
	else if (tw->splineBrowser->selected(2)) // Cardinal
	{
		//splineMat = glm::mat4(-1, 3, -3, 1,
		//	2, -5, 4, -1,
		//	-1, 0, 1, 0,
		//	0, 2, 0, 0);
		//splineMat = 0.5f * splineMat;
		float T = tw->tensionSlider->value();
		float s = (1.0f - T) / 2.0f;
		splineMat = glm::mat4(-s, 2-s, s-2, s,
			2*s, s-3, 3-2*s, -s,
			-s, 0, s, 0,
			0, 1, 0, 0);
		divide_line = 100;
	}
	else if (tw->splineBrowser->selected(3)) // Cubic B-Spline
	{
		splineMat = glm::mat4(-1, 3, -3, 1,
			3, -6, 3, 0,
			-3, 0, 3, 0,
			1, 4, 1, 0) / 6.0f;
		divide_line = 100;
	}
	else {
		splineMat = glm::mat4(0, 0, 0, 0,
			0, 0, 0, 0,
			0, -1, 1, 0,
			0, 1, 0, 0);
		divide_line = 1;
	}

	unsigned int verticesNum = m_pTrack->points.size();
	std::vector<glm::vec3> trackPos(verticesNum);
	std::vector<glm::vec3> trackDirect(verticesNum);
	std::vector<glm::vec3> trackOrient(verticesNum);
	std::vector<glm::vec3> trackCross(verticesNum);
	std::vector<glm::vec3> leftTrackPos(verticesNum);
	std::vector<glm::vec3> rightTrackPos(verticesNum);

	if (verticesNum < 4) return;

	for (int i = 0; i < verticesNum; i++) {
		trackPos[i] = glm::vec3(m_pTrack->points[i].pos.x,
			m_pTrack->points[i].pos.y,
			m_pTrack->points[i].pos.z);
	}

	for (int i = 0; i < verticesNum; i++) {
		trackDirect[i] = trackPos[(i + 1) % verticesNum] - trackPos[i];
		trackOrient[i] = glm::vec3(m_pTrack->points[i].orient.x,
			m_pTrack->points[i].orient.y,
			m_pTrack->points[i].orient.z);
	}

	for (int i = 0; i < verticesNum; i++) {
		glm::vec3 cr0 = glm::cross(trackDirect[i], trackOrient[i]);
		glm::vec3 cr1 = glm::cross(trackDirect[(i + 1) % verticesNum], trackOrient[(i + 1) % verticesNum]);
		trackCross[(i + 1) % verticesNum] = glm::normalize(glm::normalize(cr0) + glm::normalize(cr1));
	}

	for (int i = 0; i < verticesNum; i++) {
		leftTrackPos[i] = trackPos[i] - 0.5f * trackWidth * trackCross[i];
	}

	for (int i = 0; i < verticesNum; i++) {
		rightTrackPos[i] = trackPos[i] + 0.5f * trackWidth * trackCross[i];
	}

	// calculate spline 
	std::vector<glm::vec3> leftTrackSplinePosOri = spline(splineMat, leftTrackPos, divide_line);
	std::vector<glm::vec3> rightTrackSplinePosOri = spline(splineMat, rightTrackPos, divide_line);

	std::vector<glm::vec3> trackSplinePosOri = spline(splineMat, trackPos, divide_line);
	std::vector<glm::vec3> trackSplineCrossOri = spline(splineMat, trackCross, divide_line);

	// Adaptive subdivision
	trackSplinePos.clear();
	leftTrackSplinePos.clear();
	rightTrackSplinePos.clear();
	trackSplineCross.clear();
	if (tw->AdaptiveSubdivisionButton->value()) {
		trackSplinePos.push_back(trackSplinePosOri[0]);
		leftTrackSplinePos.push_back(leftTrackSplinePosOri[0]);
		rightTrackSplinePos.push_back(rightTrackSplinePosOri[0]);
		trackSplineCross.push_back(trackSplineCrossOri[0]);
		for (unsigned int i = 1; i < trackSplinePosOri.size(); i++) {
			glm::vec3 prePos = trackSplinePos.back();
			glm::vec3 newPos = trackSplinePosOri[i];

			float trueLen = glm::length(newPos - prePos);
			float newLen = glm::length(trackSplinePos.back() - newPos);
			while ((trueLen - newLen) < 0.001f) {
				i = i + 1;
				if (i >= trackSplinePosOri.size()) break;
				prePos = newPos;
				newPos = trackSplinePosOri[i];
				trueLen = trueLen + glm::length(newPos - prePos);

				newLen = glm::length(trackSplinePos.back() - newPos);
			}
			i = i - 1;
			trackSplinePos.push_back(trackSplinePosOri[i]);
			leftTrackSplinePos.push_back(leftTrackSplinePosOri[i]);
			rightTrackSplinePos.push_back(rightTrackSplinePosOri[i]);
			trackSplineCross.push_back(trackSplineCrossOri[i]);
		}
	}
	else {
		trackSplinePos = trackSplinePosOri;
		leftTrackSplinePos = leftTrackSplinePosOri;
		rightTrackSplinePos = rightTrackSplinePosOri;
		trackSplineCross = trackSplineCrossOri;
	}

	//
	trackLength = 0;
	trackSplineDirect.resize(trackSplinePos.size());
	trackSplineLength.resize(trackSplinePos.size());
	for (int i = 0; i < trackSplinePos.size(); i++) {
		trackSplineDirect[i] = trackSplinePos[(i + 1) % trackSplinePos.size()] - trackSplinePos[i];
		trackLength += glm::length(trackSplineDirect[i]);
		trackSplineLength[i] = trackLength;
	}
	// update track model
	buildTrackModel(leftTrackSplinePos, rightTrackSplinePos, trackSplineCross, trackSplineDirect);
	trackModel->setColor(128, 128, 128);

	// sleepers
	unsigned int sleeperNum = trackLength / 10.0f;
	float stepLength = trackLength / (float)sleeperNum;
	float currLength = 0.0f;
	unsigned int currArcIdx = 0;
	sleeperModel->transforms.resize(sleeperNum);
	for (int i = 0; i < sleeperNum; i++) {
		float targetLength = i * stepLength;
		float arcLength = 0.0f;
		while (1) {
			currLength += arcLength;
			arcLength = glm::length(trackSplineDirect[currArcIdx]);
			if ((currLength + arcLength) >= targetLength) break;
			currArcIdx = (currArcIdx + 1) % trackSplineDirect.size();
		}
		float t = (targetLength - currLength) / arcLength;
		glm::vec3 sleeperPos = (1 - t) * trackSplinePos[currArcIdx] + t * trackSplinePos[(currArcIdx + 1) % trackSplinePos.size()];
		//glm::vec3 sleeperCross = trackSplineCross[currArcIdx];
		glm::vec3 sleeperCross = (1 - t)* trackSplineCross[currArcIdx] + t * trackSplineCross[(currArcIdx + 1) % trackSplineCross.size()];
		glm::vec3 sleeperDirect = trackSplineDirect[currArcIdx];


		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f)) * transform;
		glm::vec3 new_z = glm::normalize(sleeperDirect);
		glm::vec3 new_y = glm::normalize(-glm::cross(sleeperDirect, sleeperCross));
		glm::vec3 new_x = glm::normalize(glm::cross(new_z, new_y));
		glm::mat3 rotate = glm::mat3(new_x, new_y, new_z);
		transform = glm::mat4(
			new_x.x, new_x.y, new_x.z, 0.0f,
			new_y.x, new_y.y, new_y.z, 0.0f,
			new_z.x, new_z.y, new_z.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		) * transform;
		transform = glm::translate(sleeperPos) * transform;
		sleeperModel->transforms[i] = transform;
	}

	TrainView::trainControl->UpdateTruckParameter(&trackSplinePos, &trackSplineDirect, &trackSplineCross, &trackSplineLength);
	for(unsigned int carControlIdx=0; carControlIdx< TrainView::carControl.size(); carControlIdx++)
		TrainView::carControl[carControlIdx]->UpdateTruckParameter(&trackSplinePos, &trackSplineDirect, &trackSplineCross, &trackSplineLength);
	initTrees();
}
void TrainView::buildTrackModel(std::vector<glm::vec3>& positions1, std::vector<glm::vec3>& positions2,
	std::vector<glm::vec3>& crosses, std::vector<glm::vec3>& directs) {
	std::vector<glm::vec3> verticesPosition;
	std::vector<glm::vec3> verticesNormal;
	std::vector<glm::vec3>& positions = positions1;
	for (unsigned int i = 0; i < positions.size(); i++) {
		glm::vec3 begUp = glm::normalize(glm::cross(directs[i], crosses[i]));
		glm::vec3 endUp = glm::normalize(glm::cross(directs[(i + 1) % directs.size()], crosses[(i + 1) % crosses.size()]));
		glm::vec3 begPos = positions[i];
		glm::vec3 endPos = positions[(i + 1) % positions.size()];
		glm::vec3 begCross = crosses[i];
		glm::vec3 endCross = crosses[(i + 1) % crosses.size()];

		//top
		verticesPosition.push_back(begPos + begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f + endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(begUp));
		verticesNormal.push_back(glm::normalize(begUp));
		verticesNormal.push_back(glm::normalize(endUp));
		verticesNormal.push_back(glm::normalize(endUp));
		//bottom
		verticesPosition.push_back(begPos + begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f - endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(-begUp));
		verticesNormal.push_back(glm::normalize(-begUp));
		verticesNormal.push_back(glm::normalize(-endUp));
		verticesNormal.push_back(glm::normalize(-endUp));
		//left
		verticesPosition.push_back(begPos + begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos + begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(begCross));
		verticesNormal.push_back(glm::normalize(begCross));
		verticesNormal.push_back(glm::normalize(endCross));
		verticesNormal.push_back(glm::normalize(endCross));
		//right
		verticesPosition.push_back(begPos - begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(-begCross));
		verticesNormal.push_back(glm::normalize(-begCross));
		verticesNormal.push_back(glm::normalize(-endCross));
		verticesNormal.push_back(glm::normalize(-endCross));
	}
	positions = positions2;
	for (unsigned int i = 0; i < positions.size(); i++) {
		glm::vec3 begUp = glm::normalize(glm::cross(directs[i], crosses[i]));
		glm::vec3 endUp = glm::normalize(glm::cross(directs[(i + 1) % directs.size()], crosses[(i + 1) % crosses.size()]));
		glm::vec3 begPos = positions[i];
		glm::vec3 endPos = positions[(i + 1) % positions.size()];
		glm::vec3 begCross = crosses[i];
		glm::vec3 endCross = crosses[(i + 1) % crosses.size()];

		//top
		verticesPosition.push_back(begPos + begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f + endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(begUp));
		verticesNormal.push_back(glm::normalize(begUp));
		verticesNormal.push_back(glm::normalize(endUp));
		verticesNormal.push_back(glm::normalize(endUp));
		//bottom
		verticesPosition.push_back(begPos + begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f - endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(-begUp));
		verticesNormal.push_back(glm::normalize(-begUp));
		verticesNormal.push_back(glm::normalize(-endUp));
		verticesNormal.push_back(glm::normalize(-endUp));
		//left
		verticesPosition.push_back(begPos + begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos + begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos + endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(begCross));
		verticesNormal.push_back(glm::normalize(begCross));
		verticesNormal.push_back(glm::normalize(endCross));
		verticesNormal.push_back(glm::normalize(endCross));
		//right
		verticesPosition.push_back(begPos - begCross * 0.375f + begUp * 0.375f);
		verticesPosition.push_back(begPos - begCross * 0.375f - begUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f - endUp * 0.375f);
		verticesPosition.push_back(endPos - endCross * 0.375f + endUp * 0.375f);
		verticesNormal.push_back(glm::normalize(-begCross));
		verticesNormal.push_back(glm::normalize(-begCross));
		verticesNormal.push_back(glm::normalize(-endCross));
		verticesNormal.push_back(glm::normalize(-endCross));
	}
	trackModel->loadVertices(verticesPosition, verticesNormal);
}
void TrainView::drawTrackSpline(glm::mat4& splineMat, unsigned int divide_line, bool doingShadows) {
	unsigned int verticesNum = m_pTrack->points.size();
	std::vector<glm::vec3> trackPos(verticesNum);
	std::vector<glm::vec3> trackDirect(verticesNum);
	std::vector<glm::vec3> trackOrient(verticesNum);
	std::vector<glm::vec3> trackCross(verticesNum);
	std::vector<glm::vec3> leftTrackPos(verticesNum);
	std::vector<glm::vec3> rightTrackPos(verticesNum);

	if (verticesNum < 4) return;

	for (int i = 0; i < verticesNum; i++) {
		trackPos[i] = glm::vec3(m_pTrack->points[i].pos.x,
			m_pTrack->points[i].pos.y,
			m_pTrack->points[i].pos.z);
	}

	for (int i = 0; i < verticesNum; i++) {
		trackDirect[i] = trackPos[(i + 1) % verticesNum] - trackPos[i];
		trackOrient[i] = glm::vec3(m_pTrack->points[i].orient.x,
			m_pTrack->points[i].orient.y,
			m_pTrack->points[i].orient.z);
	}

	for (int i = 0; i < verticesNum; i++) {
		glm::vec3 cr0 = glm::cross(trackDirect[i], trackOrient[i]);
		glm::vec3 cr1 = glm::cross(trackDirect[(i + 1) % verticesNum], trackOrient[(i + 1) % verticesNum]);
		trackCross[(i + 1) % verticesNum] = glm::normalize(glm::normalize(cr0) + glm::normalize(cr1));
	}

	for (int i = 0; i < verticesNum; i++) {
		leftTrackPos[i] = trackPos[i] - 0.5f * trackWidth * trackCross[i];
	}

	for (int i = 0; i < verticesNum; i++) {
		rightTrackPos[i] = trackPos[i] + 0.5f * trackWidth * trackCross[i];
	}

	std::vector<glm::vec3> leftTrackSplinePos = spline(splineMat, leftTrackPos, divide_line);
	std::vector<glm::vec3> rightTrackSplinePos = spline(splineMat, rightTrackPos, divide_line);

	drawlinesloop(leftTrackSplinePos, doingShadows);
	drawlinesloop(rightTrackSplinePos, doingShadows);

	std::vector<glm::vec3> trackSplinePos = spline(splineMat, trackPos, divide_line);
	std::vector<glm::vec3> trackSplineCross = spline(splineMat, trackCross, divide_line);


	// sleeper
	float trackLength = 0;
	std::vector<glm::vec3> trackSplineDirect(trackSplinePos.size());
	for (int i = 0; i < trackSplinePos.size(); i++) {
		trackSplineDirect[i] = trackSplinePos[(i + 1) % trackSplinePos.size()] - trackSplinePos[i];
		trackLength += glm::length(trackSplineDirect[i]);
	}
	unsigned int sleeperNum = trackLength / 10.0f;
	//std::cout << sleeperNum << " , " << trackLength << std::endl;
	float stepLength = trackLength / (float)sleeperNum;
	float currLength = 0.0f;
	unsigned int currArcIdx = 0;
	sleeperModel->transforms.resize(sleeperNum);
	for (int i = 0; i < sleeperNum; i++) {
		float targetLength = i * stepLength;
		float arcLength = 0.0f;
		while(1) {
			currLength += arcLength;
			arcLength = glm::length(trackSplineDirect[currArcIdx]);
			if ((currLength + arcLength) >= targetLength) break;
			currArcIdx = (currArcIdx + 1)% trackSplineDirect.size();
		}
		float t = (targetLength - currLength) / arcLength;
		glm::vec3 sleeperPos = (1 - t) * trackSplinePos[currArcIdx] + t * trackSplinePos[(currArcIdx + 1) % trackSplinePos.size()];
		glm::vec3 sleeperLeftPos = (1 - t) * leftTrackSplinePos[currArcIdx] + t * leftTrackSplinePos[(currArcIdx + 1) % leftTrackSplinePos.size()];
		glm::vec3 sleeperRightPos = (1 - t) * rightTrackSplinePos[currArcIdx] + t * rightTrackSplinePos[(currArcIdx + 1) % rightTrackSplinePos.size()];
		//glm::vec3 sleeperCross = (1 - t) * trackSplineCross[currArcIdx] + t * trackSplineCross[(currArcIdx + 1) % trackSplineCross.size()];
		//glm::vec3 sleeperDirect = (1 - t) * trackSplineDirect[currArcIdx] + t * trackSplineDirect[(currArcIdx + 1) % trackSplineDirect.size()];
		glm::vec3 sleeperCross = trackSplineCross[currArcIdx];
		glm::vec3 sleeperDirect = trackSplineDirect[currArcIdx];

		//sleeperModel->transforms[i] = glm::translate(sleeperPos) *
		//	glm::orientation(glm::normalize(sleeperCross), glm::vec3(1.0f, 0.0f, 0.0f)) *
		//	glm::scale(glm::vec3(0.15f, 0.15f, 0.15f)) *
		//	glm::mat4(1.0f);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f)) * transform;
		glm::vec3 new_z = glm::normalize(sleeperDirect);
		glm::vec3 new_y = glm::normalize(-glm::cross(sleeperDirect, sleeperCross));
		glm::vec3 new_x = glm::normalize(glm::cross(new_z, new_y));
		glm::mat3 rotate= glm::mat3(new_x, new_y, new_z);
		transform = glm::mat4(
			new_x.x, new_x.y, new_x.z, 0.0f,
			new_y.x, new_y.y, new_y.z, 0.0f,
			new_z.x, new_z.y, new_z.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			) * transform;
		transform = glm::translate(sleeperPos) * transform;
		sleeperModel->transforms[i] = transform;
		//sleeperModel->transforms[i] = glm::translate(sleeperPos) *
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::normalize(sleeperDirect * glm::vec3(-1.0f, -1.0f, 1.0f)), glm::normalize(glm::cross(sleeperCross, sleeperDirect))) *
		//	glm::scale(glm::vec3(0.15f, 0.15f, 0.15f)) *
		//	glm::mat4(1.0f);
	
		//glLineWidth(3);
		//glBegin(GL_POLYGON);
		////glBegin(GL_LINE);
		//if (!doingShadows)
		//	glColor3ub(255, 255, 255);

		//const float sleeperWidthPad = 5.0f;
		//const float sleeperLength = 8.0f;

		//glm::vec3 drawPos;
		//drawPos = sleeperRightPos + 0.5f * sleeperWidthPad * sleeperCross + 0.5f * sleeperLength * glm::normalize(sleeperDirect);
		//glVertex3f(drawPos.x, drawPos.y, drawPos.z);
		//drawPos = sleeperLeftPos - 0.5f * sleeperWidthPad * sleeperCross + 0.5f * sleeperLength * glm::normalize(sleeperDirect);
		//glVertex3f(drawPos.x, drawPos.y, drawPos.z);
		//drawPos = sleeperLeftPos - 0.5f * sleeperWidthPad * sleeperCross - 0.5f * sleeperLength * glm::normalize(sleeperDirect);
		//glVertex3f(drawPos.x, drawPos.y, drawPos.z);
		//drawPos = sleeperRightPos + 0.5f * sleeperWidthPad * sleeperCross - 0.5f * sleeperLength * glm::normalize(sleeperDirect);
		//glVertex3f(drawPos.x, drawPos.y, drawPos.z);

		//glEnd();
		//glLineWidth(1);
	}
	sleeperModel->draw(doingShadows);

}
std::vector<glm::vec3> TrainView::spline(glm::mat4& splineMat, std::vector<glm::vec3>& vertices, unsigned int divide_line) {
	std::vector<glm::vec3> splinePos(vertices.size() * divide_line);
	unsigned int splinePosIdx = 0;
	for (int i = 0; i < vertices.size(); i++) {
		glm::vec3* controlPositions[4];
		for (int j = 0; j < 4; j++) {
			controlPositions[j] = &(vertices[(i + j) % vertices.size()]);
		}
		float percent = 1.0f / divide_line;
		float t = 0;

		glm::mat4 controlPosMat(controlPositions[0]->x, controlPositions[0]->y, controlPositions[0]->z, 1.0f,
			controlPositions[1]->x, controlPositions[1]->y, controlPositions[1]->z, 1.0f,
			controlPositions[2]->x, controlPositions[2]->y, controlPositions[2]->z, 1.0f,
			controlPositions[3]->x, controlPositions[3]->y, controlPositions[3]->z, 1.0f);

		for (int j = 0; j < divide_line; j++) {
			splinePos[splinePosIdx++] = controlPosMat * splineMat * glm::vec4(powf(t, 3), powf(t, 2), t, 1.0f);
			t += percent;
		}
	}
	return splinePos;
}

//unused
void TrainView::drawSpline(glm::mat4& splineMat, std::vector<glm::vec3>& vertices, bool doingShadows) {
	const int DIVIDE_LINE = 10;
	for (int i = 0; i < vertices.size(); i++) {
		glm::vec3* controlPositions[4];
		for (int j = 0; j < 4; j++) {
			controlPositions[j] = &(vertices[(i + j) % vertices.size()]);
		}
		float percent = 1.0f / DIVIDE_LINE;
		float t = 0;

		glm::mat4 controlPosMat(controlPositions[0]->x, controlPositions[0]->y, controlPositions[0]->z, 1.0f,
								controlPositions[1]->x, controlPositions[1]->y, controlPositions[1]->z, 1.0f,
								controlPositions[2]->x, controlPositions[2]->y, controlPositions[2]->z, 1.0f,
								controlPositions[3]->x, controlPositions[3]->y, controlPositions[3]->z, 1.0f);

		for (int j = 0; j < DIVIDE_LINE; j++) {
			glm::vec3 qt0 = controlPosMat * splineMat * glm::vec4(powf(t, 3), powf(t, 2), t, 1.0f);
			t += percent;
			glm::vec3 qt1 = controlPosMat * splineMat * glm::vec4(powf(t, 3), powf(t, 2), t, 1.0f);


			glLineWidth(3);
			glBegin(GL_LINES);
			if (!doingShadows)
				glColor3ub(32, 32, 64);

			glVertex3f(qt0.x, qt0.y, qt0.z);
			glVertex3f(qt1.x, qt1.y, qt1.z);

			glEnd();
			glLineWidth(1);
		}
	}
}

void TrainView::drawlinesloop(std::vector<glm::vec3>& vertices, bool doingShadows) {
	glLineWidth(3);
	glBegin(GL_LINE_LOOP);
	if (!doingShadows)
		glColor3ub(32, 32, 64);
	for (glm::vec3 pos : vertices) {
		glVertex3fv(glm::value_ptr(pos));
	}
	glEnd();
	glLineWidth(1);
}
void TrainView::drawlinesloopBox(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& direct, std::vector<glm::vec3>& cross, bool doingShadows) {
	if (!doingShadows)
		glColor3ub(32, 32, 64);
	glBegin(GL_QUADS);
	for (unsigned int i = 0; i < positions.size(); i++) {
		glm::vec3 begPos = positions[i];
		glm::vec3 endPos = positions[(i + 1) % positions.size()];
		glm::vec3 leftBegPos = begPos + cross[i] * 0.4f;
		glm::vec3 leftEndPos = endPos + cross[(i + 1)% cross.size()] * 0.4f;
		glm::vec3 RightBegPos = begPos - cross[i] * 0.4f;
		glm::vec3 RightEndPos = endPos - cross[(i + 1) % cross.size()] * 0.4f;
		glm::vec3 normal = glm::normalize(glm::cross(cross[i], direct[i]));
		glVertex3fv(glm::value_ptr(leftBegPos));
		glVertex3fv(glm::value_ptr(leftEndPos));
		glVertex3fv(glm::value_ptr(RightEndPos));
		glVertex3fv(glm::value_ptr(RightBegPos));
		glNormal3fv(glm::value_ptr(normal));
	}
	glEnd();
}


void TrainView::trainMove(float distance) {
	int mode = (tw->splineBrowser->selected(1) == 0) && (tw->tensionSlider->value() < 0.99f);
	TrainView::trainControl->Move(distance, 0, mode);
	for (unsigned int carControlIdx = 0; carControlIdx < TrainView::carControl.size(); carControlIdx++) {
		//std::cout << "move Cars " << carControlIdx << "-" << carModel->transforms.size() << std::endl;
		TrainView::carControl[carControlIdx]->Move(distance, carControlIdx, mode);
	}
	headlightModel->transforms[0] = trainModel->transforms[0];
	//std::cout << "move Done" << std::endl;
}
void TrainView::trainReset() {
	TrainView::trainControl->ResetProcess();
	for (unsigned int carControlIdx = 0; carControlIdx < TrainView::carControl.size(); carControlIdx++) {
		TrainView::carControl[carControlIdx]->ResetProcess();
		TrainView::carControl[carControlIdx]->SetProcess(0.0f);
		TrainView::carControl[carControlIdx]->Move(trainControl->GetProcess() - 11 * carControlIdx - 13);
	}
}
void TrainView::setCars(unsigned int num) {
	//std::cout << "setCars " << carControl.size() << "-" << num << std::endl;
	carModel->setInstanceNum(num);
	if (carControl.size() < num) {
		while (carControl.size() < num) {
			CaronTrack* newControl = new CaronTrack(carModel);
			newControl->UpdateTruckParameter(&trackSplinePos, &trackSplineDirect, &trackSplineCross, &trackSplineLength);
			newControl->Move(trainControl->GetProcess() - 11 * carControl.size()-13);
			carControl.push_back(newControl);
		}
	}
	if (carControl.size() > num) {
		while (carControl.size() > num) {
			CaronTrack* delControl = carControl.back();
			carControl.pop_back();
			delete delControl;
		}
	}
}
// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n",selectedCube);
}


void TrainView::initTrees() {
	glm::vec3 treesPositions[] = {	glm::vec3(-83.0f, 0.0f, 37.0f),
									glm::vec3(-62.0f, 0.0f, -69.0f),
									glm::vec3(-27.0f, 0.0f, 24.0f),
									glm::vec3(-4.0f, 0.0f, 0.0f),
									glm::vec3(2.0f, 0.0f, 83.0f),
									glm::vec3(13.0f, 0.0f, -72.0f),
									glm::vec3(21.0f, 0.0f, 53.0f),
									glm::vec3(39.0f, 0.0f, 46.0f),
									glm::vec3(55.0f, 0.0f, 95.0f),
									glm::vec3(74.0f, 0.0f, 28.0f),
	};
	float treesRotations[] = { 0, 40, 35, 86, 12, 138, 264, 237, 186, 311};
	unsigned int treesNum = sizeof(treesPositions) / sizeof(glm::vec3);
	TrainView::treeAModel->setInstanceNum(treesNum);
	for (unsigned int treesIdx = 0; treesIdx < treesNum; treesIdx++) {
		glm::vec3& currPos = treesPositions[treesIdx];
		glm::mat4 scale = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));
		glm::mat4 rotate = glm::rotate(treesRotations[treesIdx], glm::vec3(0.0f, 1.0f, 0.0f));

		// check if on the truck
		for (unsigned int trackIdx = 0; trackIdx < trackSplineDirect.size(); trackIdx++) {
			glm::mat4 trackTransform(1.0f);

			trackTransform = glm::translate(-trackSplinePos[trackIdx]) * trackTransform;
			glm::vec3 new_z = glm::normalize(trackSplineDirect[trackIdx]);
			glm::vec3 new_y = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 new_x = glm::normalize(glm::cross(new_z, new_y));
			trackTransform = glm::mat4(
				new_x.x, new_x.y, new_x.z, 0.0f,
				new_y.x, new_y.y, new_y.z, 0.0f,
				new_z.x, new_z.y, new_z.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			) * trackTransform;
			trackTransform = glm::scale(glm::vec3(0.1f, 0.04f, 0.1f/glm::length(trackSplineDirect[trackIdx]))) * trackTransform;

			glm::vec3 treeProj = trackTransform * glm::vec4(currPos, 1.0f);
			treeProj = treeProj;
			if (-1.0f < treeProj.x && treeProj.x < 1.0f &&
				-1.0f < treeProj.y && treeProj.y < 1.0f &&
				0.0f < treeProj.z && treeProj.z < 1.0f)
				scale = scale * 0.0f;
		}

		// put transforms
		glm::mat4 transform(1.0f);
		transform = scale * rotate * transform;
		transform = glm::translate(currPos) * transform;
		TrainView::treeAModel->transforms[treesIdx] = transform;
	}
	TrainView::treeAModel->setColor(16, 64, 16);
}


CaronTrack::CaronTrack(ModelClass* targetModel) {
	CaronTrack::model = targetModel;
	CaronTrack::trackPosition = NULL;
	CaronTrack::trackDirect = NULL;
	CaronTrack::trackCross = NULL;
	CaronTrack::trackLength = NULL;
	CaronTrack::runProcess = 0.0f;
	CaronTrack::runSplineIdx = 0;
}
void CaronTrack::UpdateModel(ModelClass* targetModel) {
	CaronTrack::model = targetModel;
}
void CaronTrack::UpdateTruckParameter(std::vector<glm::vec3>* positions, std::vector<glm::vec3>* directions, std::vector<glm::vec3>* crosses, std::vector<float>* lengths) {
	CaronTrack::trackPosition = positions;
	CaronTrack::trackDirect = directions;
	CaronTrack::trackCross = crosses;
	CaronTrack::trackLength = lengths;
}
void CaronTrack::ResetProcess() {
	//CaronTrack::runProcess = 0.0f;
	CaronTrack::runSplineIdx = 0;
}
void CaronTrack::Move(float distance, unsigned int instanceIdx, unsigned int interpolateMode) {
	if (CaronTrack::model == NULL) return;
	if (CaronTrack::trackPosition == NULL) return;
	if (CaronTrack::trackDirect == NULL) return;
	if (CaronTrack::trackCross == NULL) return;
	if (CaronTrack::trackLength == NULL) return;

	runProcess = runProcess + distance;
	while (runProcess < 0) runProcess += (*trackLength)[trackLength->size()-1];
	runProcess = fmod(runProcess, (*trackLength)[trackLength->size() - 1]);
	float begLen, endLen;
	if (distance >= 0) {
		while (1) {
			begLen = (runSplineIdx == 0) ? 0 : (*trackLength)[runSplineIdx - 1];
			endLen = (*trackLength)[runSplineIdx];
			if (begLen <= runProcess && runProcess <= endLen) break;
			runSplineIdx = (runSplineIdx + 1) % trackLength->size();
		}

	}
	else {
		while (1) {
			begLen = (runSplineIdx == 0) ? 0 : (*trackLength)[runSplineIdx - 1];
			endLen = (*trackLength)[runSplineIdx];
			if (begLen <= runProcess && runProcess <= endLen) break;
			runSplineIdx = (runSplineIdx == 0) ? trackLength->size() - 1 : runSplineIdx - 1;
		}
	}

	float t = (runProcess - begLen) / (endLen - begLen);

	glm::vec3 modelPos, modelCross, modleDirect;
	//if (interpolateMode == 1) {
	//float l = (*trackLength)[runSplineIdx];
	if (interpolateMode == 1) {
		modelPos = (1 - t) * (*trackPosition)[runSplineIdx] + t * (*trackPosition)[(runSplineIdx + 1) % trackPosition->size()];
		modelCross = (1 - t) * (*trackCross)[runSplineIdx] + t * (*trackCross)[(runSplineIdx + 1) % trackCross->size()];
		modleDirect = (1 - t) * (*trackDirect)[runSplineIdx] + t * (*trackDirect)[(runSplineIdx + 1) % trackDirect->size()];
	}
	else {
		modelPos = (1 - t) * (*trackPosition)[runSplineIdx] + t * (*trackPosition)[(runSplineIdx + 1) % trackPosition->size()];
		modelCross = (1 - t) * (*trackCross)[runSplineIdx] + t * (*trackCross)[(runSplineIdx + 1) % trackCross->size()];
		modleDirect = (*trackDirect)[runSplineIdx];
	}

	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f)) * transform;
	glm::vec3 new_z = glm::normalize(modleDirect);
	glm::vec3 new_y = glm::normalize(-glm::cross(modleDirect, modelCross));
	glm::vec3 new_x = glm::normalize(glm::cross(new_z, new_y));
	glm::mat3 rotate = glm::mat3(new_x, new_y, new_z);
	transform = glm::mat4(
		new_x.x, new_x.y, new_x.z, 0.0f,
		new_y.x, new_y.y, new_y.z, 0.0f,
		new_z.x, new_z.y, new_z.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * transform;

	CaronTrack::model->positions[instanceIdx] = modelPos;
	CaronTrack::model->directions[instanceIdx] = new_z;
	CaronTrack::model->ups[instanceIdx] = new_y;

	transform = glm::translate(modelPos) * transform;

	CaronTrack::model->transforms[instanceIdx] = transform;
}
float CaronTrack::GetProcess() {
	return CaronTrack::runProcess;
}
unsigned int CaronTrack::GetIndex() {
	return CaronTrack::runSplineIdx;
}
void CaronTrack::SetProcess(float val) {
	CaronTrack::runProcess = val;
}
