


#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <glm\detail\type_vec.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLSL.h"

class Program;

class Shape
{

public:
	//stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
	void loadMesh(const std::string &meshName, std::string *mtlName = NULL, unsigned char *(loadimage)(char const *, int *, int *, int *, int) = NULL);
	void init();
	void initInstance(const std::shared_ptr<Program> prog, const int size, const int distance);
	void resize();
	void draw(const std::shared_ptr<Program> prog, bool use_extern_texures) const;
	void drawInstanced(const std::shared_ptr<Program> prog, bool use_extern_texures) const;
	unsigned int *textureIDs = NULL;

	int obj_count = 0;
	unsigned instanceSize = 0;
	std::vector<unsigned int> *eleBuf = NULL;
	std::vector<float> *posBuf = NULL;
	std::vector<float> *norBuf = NULL;
	std::vector<float> *texBuf = NULL;
	std::vector<glm::vec4> *insBuf = NULL;
	unsigned int *materialIDs = NULL;


	unsigned int *eleBufID = 0;
	unsigned int *posBufID = 0;
	unsigned int *norBufID = 0;
	unsigned int *texBufID = 0;
	unsigned int *insBufID = 0;
	unsigned int *vaoID = 0;
};

#endif // LAB471_SHAPE_H_INCLUDED
