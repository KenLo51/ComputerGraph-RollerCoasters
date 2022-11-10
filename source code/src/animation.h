#pragma once

#include "model.h"
#include <glad/glad.h>
#include "GL/glu.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class Animation {
public:
	std::vector <unsigned int> currFrame;
	std::vector <ModelClass*> models;
	std::vector <float> delayTime;

	std::vector <float> currTime;
public:
	std::vector<bool> repeat;
	std::vector<glm::mat4> transforms;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> directions;
	std::vector<glm::vec3> ups;
public:
	Animation();
	Animation(std::vector <ModelClass*>& frames, std::vector <float>& delayTimes);
	void initTransforms(unsigned int num = 1);
	void Load(std::vector <ModelClass*>& frames, std::vector <float>& delayTimes);
	void addInstance(glm::mat4 transforms = glm::mat4(1.0f), bool repeat = false);
	void removeInstance(unsigned int index = 1);
	void timeAdd(float t);
	void Draw(bool doingShadows = false);
};
