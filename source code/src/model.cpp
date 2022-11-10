#include "model.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "GL/glu.h"

#include <iostream>
#include <iomanip>
//ModelClass::ModelClass() {
//	ModelClass::transforms.resize(1);
//	ModelClass::positions.resize(1);
//	ModelClass::directions.resize(1);
//	ModelClass::ups.resize(1);
//
//	ModelClass::transforms[0] = glm::mat4(1.0f);
//	ModelClass::positions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
//	ModelClass::directions[0] = glm::vec3(0.0f, 0.0f, 1.0f);
//	ModelClass::ups[0] = glm::vec3(0.0f, 1.0f, 0.0f);
//}
//ModelClass::ModelClass(const char* fileName) {
//	ModelClass::ModelClass();
//	loadObjFile(fileName);
//}
ModelClass::ModelClass() {
	ModelClass::transforms.resize(1);
	ModelClass::positions.resize(1);
	ModelClass::directions.resize(1);
	ModelClass::ups.resize(1);

	ModelClass::transforms[0] = glm::mat4(1.0f);
	ModelClass::positions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	ModelClass::directions[0] = glm::vec3(0.0f, 0.0f, 1.0f);
	ModelClass::ups[0] = glm::vec3(0.0f, 1.0f, 0.0f);
}
ModelClass::ModelClass(const char* fileName) {
	ModelClass::transforms.resize(1);
	ModelClass::positions.resize(1);
	ModelClass::directions.resize(1);
	ModelClass::ups.resize(1);

	ModelClass::transforms[0] = glm::mat4(1.0f);
	ModelClass::positions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	ModelClass::directions[0] = glm::vec3(0.0f, 0.0f, 1.0f);
	ModelClass::ups[0] = glm::vec3(0.0f, 1.0f, 0.0f);
	loadObjFile(fileName);
}
int ModelClass::loadObjFile(const char* fileName) {
	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(fileName)) return -1;


	const tinyobj::attrib_t & attrib = reader.GetAttrib();
	const std::vector<tinyobj::shape_t> & shapes = reader.GetShapes();

	//for (unsigned int meshIdx = 0; meshIdx < shapes.size(); meshIdx++){
	//	const tinyobj::mesh_t &curMesh = shapes[meshIdx].mesh;
	//	Mesh newMesh;
	//	newMesh.color = glm::u8vec3(192, 192, 192);

	//	for (int vertexIdx = 0; vertexIdx < attrib.vertices.size(); vertexIdx+=3){
	//		
	//		glm::vec3 newVertex(attrib.vertices[vertexIdx+0],
	//			attrib.vertices[vertexIdx+1],
	//			attrib.vertices[vertexIdx+2]);

	//		ModelClass::verticesPos.push_back(newVertex);
	//	}
	//	for (int j = 0; j < curMesh.indices.size(); j ++) {
	//		newMesh.indices.push_back((unsigned int)(curMesh.indices[j].vertex_index));
	//	}


	//	ModelClass::meshes.push_back(newMesh);
	//}

	ModelClass::verticesPos.clear();
	for (int vertexIdx = 0; vertexIdx < attrib.vertices.size(); vertexIdx += 3) {
		glm::vec3 newVertex(attrib.vertices[vertexIdx + 0],
			attrib.vertices[vertexIdx + 1],
			attrib.vertices[vertexIdx + 2]);

		ModelClass::verticesPos.push_back(newVertex);
	}

	ModelClass::normals.clear();
	for (int normalIdx = 0; normalIdx < attrib.normals.size(); normalIdx += 3) {
		glm::vec3 newNormal(attrib.normals[normalIdx + 0],
			attrib.normals[normalIdx + 1],
			attrib.normals[normalIdx + 2]);

		ModelClass::normals.push_back(newNormal);
	}

	ModelClass::meshes.clear();
	std::cout << "loading model " << fileName << std::endl;
	for (unsigned int meshIdx = 0; meshIdx < shapes.size(); meshIdx++){
		const tinyobj::mesh_t &curMesh = shapes[meshIdx].mesh;
		Mesh newMesh;
		newMesh.color = glm::u8vec3(192, 192, 192);

		for (int j = 0; j < curMesh.indices.size(); j ++) {
			newMesh.posIndices.push_back((unsigned int)(curMesh.indices[j].vertex_index));
			newMesh.normalIndices.push_back((unsigned int)(curMesh.indices[j].normal_index));
		}
		//std::cout << newMesh.normalIndices.size() <<" , "<< newMesh.posIndices.size() << std::endl;
		ModelClass::meshes.push_back(newMesh);
	}
	

	return 0;
}
int ModelClass::loadVertices(std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals) {
	ModelClass::verticesPos = std::vector<glm::vec3>(positions.begin(), positions.end());
	ModelClass::normals = std::vector<glm::vec3>(normals.begin(), normals.end());

	ModelClass::meshes.clear();
	Mesh newMesh;
	newMesh.posIndices.resize(ModelClass::verticesPos.size());
	for (unsigned int i = 0; i < ModelClass::verticesPos.size(); i++)
		newMesh.posIndices[i] = i;

	newMesh.normalIndices.resize(ModelClass::normals.size());
	for (unsigned int i = 0; i < ModelClass::normals.size(); i++)
		newMesh.normalIndices[i] = i;
	ModelClass::meshes.push_back(newMesh);

	return 0;
}


void ModelClass::setColor(glm::u8vec3 color, int idx) {
	if (idx < 0) {
		for (unsigned int meshIdx = 0; meshIdx < ModelClass::meshes.size(); meshIdx++) {
			ModelClass::meshes[meshIdx].color = color;
		}
	}
	else {
		ModelClass::meshes[idx % ModelClass::meshes.size()].color = color;
	}
}
void ModelClass::setColor(unsigned char r, unsigned char g, unsigned char b, int idx) {
	setColor(glm::u8vec3(r,g,b), idx);
}

void ModelClass::draw(bool doingShadows, GLenum glBeginMode) {
	for (unsigned int repeat = 0; repeat < ModelClass::transforms.size(); repeat++) {
		drawOne(repeat, doingShadows, glBeginMode);
	}
}


void ModelClass::drawOne(unsigned int idx, bool doingShadows, GLenum glBeginMode) {
	//for (unsigned int meshIdx = 0; meshIdx < ModelClass::meshes.size(); meshIdx++) {
	//	Mesh& currMesh = ModelClass::meshes[meshIdx];
	//	glBegin(GL_TRIANGLES);
	//	if (!doingShadows)
	//		glColor3ub(currMesh.color.x, currMesh.color.y, currMesh.color.z);
	//	for (unsigned int i = 0; i < currMesh.posIndices.size(); i++) {
	//		unsigned int index;

	//		index = currMesh.normalIndices[i];
	//		glm::vec3 drawNormal = ModelClass::transforms[idx] * glm::vec4(ModelClass::normals[index], 0.0f);
	//		drawNormal = glm::normalize(drawNormal);
	//		//glm::vec3 drawNormal = ModelClass::normals[index];
	//		//std::cout << std::setprecision(2) << drawNormal.x << "\t" << drawNormal.y << "\t" << drawNormal.z << "\n";
	//		glNormal3f(drawNormal.x, drawNormal.y, drawNormal.z);

	//		index = currMesh.posIndices[i];
	//		glm::vec3 drawPos = ModelClass::transforms[idx] * glm::vec4(ModelClass::verticesPos[index], 1.0f);
	//		glVertex3f(drawPos.x, drawPos.y, drawPos.z);
	//	}
	//	glEnd();
	//}


	for (unsigned int meshIdx = 0; meshIdx < ModelClass::meshes.size(); meshIdx++) {
		Mesh& currMesh = ModelClass::meshes[meshIdx];

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(ModelClass::transforms[idx]));
		//glLoadIdentity();

		glBegin(glBeginMode);
		for (unsigned int i = 0; i < currMesh.posIndices.size(); i++) {
			unsigned int index;

			index = currMesh.normalIndices[i];
			glm::vec3 drawNormal = ModelClass::normals[index];
			drawNormal = glm::normalize(drawNormal);
			//glm::vec3 drawNormal = ModelClass::normals[index];
			//std::cout << std::setprecision(2) << drawNormal.x << "\t" << drawNormal.y << "\t" << drawNormal.z << "\n";

			index = currMesh.posIndices[i];
			glm::vec3 drawPos = ModelClass::verticesPos[index];
			//glm::vec3 drawPos = ModelClass::transforms[idx] * glm::vec4(ModelClass::verticesPos[index], 1.0f);
			if (!doingShadows)
				glColor3ub(currMesh.color.x, currMesh.color.y, currMesh.color.z);
			glNormal3f(drawNormal.x, drawNormal.y, drawNormal.z);
			glVertex3f(drawPos.x, drawPos.y, drawPos.z);
		}
		glEnd();
		glPopMatrix();
	}
}

void ModelClass::setInstanceNum(unsigned int num) {
	transforms.resize(num);
	positions.resize(num);
	directions.resize(num);
	ups.resize(num);
}
unsigned int ModelClass::getInstanceNum(unsigned int num) {
	return transforms.size();
}