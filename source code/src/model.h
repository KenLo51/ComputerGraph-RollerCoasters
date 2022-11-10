#pragma once

#include <glad/glad.h>
#include "GL/glu.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>
#include <vector>
#include <tiny_obj_loader.h>
typedef struct {
	glm::u8vec3 color;
	std::vector <unsigned int> posIndices;
	std::vector <unsigned int> normalIndices;
}Mesh;
class ModelClass {
public:
	std::vector <Mesh> meshes;
	std::vector <glm::vec3> verticesPos;
	std::vector <glm::vec3> normals;
public:
	std::vector<glm::mat4> transforms;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> directions;
	std::vector<glm::vec3> ups;
public:
	ModelClass();
	ModelClass(const char*);
	int loadObjFile(const char*);
	int loadVertices(std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals);
	void clearVertices();
	void setColor(glm::u8vec3, int idx = -1);
	void setColor(unsigned char, unsigned char, unsigned char, int idx = -1);
	void setInstanceNum(unsigned int num);
	unsigned int getInstanceNum(unsigned int num);
	//void update(glm::vec3, glm::vec3, glm::vec3);
	//void update();

	void draw(bool doingShadows = false, GLenum glBeginMode = GL_TRIANGLES);
	void drawOne(unsigned int idx, bool doingShadows = false, GLenum glBeginMode = GL_TRIANGLES);
};
