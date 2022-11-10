#include "animation.h"

#include <iostream>
#include <iomanip>
Animation::Animation() {
	initTransforms(0);
}
Animation::Animation(std::vector <ModelClass*>& frames, std::vector <float>& delayTimes) {
	initTransforms(0);
	Load(frames, delayTimes);
}

void Animation::initTransforms(unsigned int num) {
	Animation::repeat.resize(num);
	Animation::currTime.resize(num);
	Animation::currFrame.resize(num);
	Animation::transforms.resize(num);
	Animation::positions.resize(num);
	Animation::directions.resize(num);
	Animation::ups.resize(num);

	for (unsigned int i = 0; i < num; i++) {
		Animation::repeat[i] = false;
		Animation::currTime[i] = 0.0f;
		Animation::currFrame[i] = 0;
		Animation::transforms[i] = glm::mat4(1.0f);
		Animation::positions[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		Animation::directions[i] = glm::vec3(0.0f, 0.0f, 1.0f);
		Animation::ups[i] = glm::vec3(0.0f, 1.0f, 0.0f);
	}
}
void Animation::Load(std::vector <ModelClass*>& frames, std::vector <float>& delayTimes) {
	Animation::models.assign(frames.begin(), frames.end());
	Animation::delayTime.assign(delayTimes.begin(), delayTimes.end());
}
void Animation::timeAdd(float t) {
	for (unsigned int instanceIdx = 0; instanceIdx < Animation::transforms.size(); instanceIdx++) {
		Animation::currTime[instanceIdx] += t;
		while (Animation::currTime[instanceIdx] >= Animation::delayTime[Animation::currFrame[instanceIdx]]) {
			Animation::currTime[instanceIdx] -= Animation::delayTime[Animation::currFrame[instanceIdx]];
			Animation::currFrame[instanceIdx] = Animation::currFrame[instanceIdx] + 1;
			if (Animation::repeat[instanceIdx]) {
				Animation::currFrame[instanceIdx] = Animation::currFrame[instanceIdx] % Animation::delayTime.size();
			}
			else {
				break;
			}
		}
		if ((!Animation::repeat[instanceIdx]) && (Animation::currFrame[instanceIdx] >= Animation::delayTime.size())) {
			removeInstance(instanceIdx);
			instanceIdx--;
			continue;
		}
	}
}
void Animation::Draw(bool doingShadows) {
	for (unsigned int instanceIdx = 0; instanceIdx < Animation::transforms.size(); instanceIdx++) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(transforms[instanceIdx]));
		models[Animation::currFrame[instanceIdx]]->draw(doingShadows);
		glPopMatrix();
	}
}

void Animation::addInstance(glm::mat4 transforms, bool repeat) {
	Animation::repeat.push_back(repeat);
	Animation::currTime.push_back(0.0f);
	Animation::currFrame.push_back(0);
	Animation::transforms.push_back(transforms);
	Animation::positions.push_back(glm::vec3(0.0f));
	Animation::directions.push_back(glm::vec3(0.0f));
	Animation::ups.push_back(glm::vec3(0.0f));
}

void Animation::removeInstance(unsigned int index) {
	Animation::repeat.erase(Animation::repeat.begin() + index);
	Animation::currTime.erase(Animation::currTime.begin() + index);
	Animation::currFrame.erase(Animation::currFrame.begin() + index);
	Animation::transforms.erase(Animation::transforms.begin() + index);
	Animation::positions.erase(Animation::positions.begin() + index);
	Animation::directions.erase(Animation::directions.begin() + index);
	Animation::ups.erase(Animation::ups.begin() + index);
}