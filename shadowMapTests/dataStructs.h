#ifndef __data_structs__
#define __data_structs__

#define GLEW_STATIC // link to glew32s instead of including dll
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
// Include GLM extensions
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>

#include "ProtoUtility.h"
#include "ProtoShader.h"
#include "ProtoMath.h"
#include <stack>

using namespace std;
using namespace ijg;

#define BUFFER_OFFSET(i) ((void*)(i))

/*********************************************
*               DATA STRUCTURES              *
*********************************************/
struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec3 col;

	Vertex(){}
	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec3& col) :
		pos(pos), norm(norm), col(col) {}
};

struct Face {
	glm::vec3* v0_ptr, *v1_ptr, *v2_ptr;

	Face(){}

	Face(glm::vec3* v0_ptr, glm::vec3* v1_ptr, glm::vec3* v2_ptr) :
		v0_ptr(v0_ptr), v1_ptr(v1_ptr), v2_ptr(v2_ptr) {}

	glm::vec3 getNorm() {
		glm::vec3 a = *v2_ptr - *v0_ptr;
		glm::vec3 b = *v1_ptr - *v0_ptr;
		return glm::normalize(glm::cross(b, a));
	}
};

struct Index {
	int i0, i1, i2;
	Index() {}
	Index(int i0, int i1, int i2) :
		i0(i0), i1(i1), i2(i2) {}
};

struct Cube{
	GLuint cubeVAO = 0;
	float w, h, d;
	Index inds[12];
	glm::vec3 vecs[8];
	Face faces[12];
	Vertex verts[8];

	Cube() :
		Cube(1, 1, 1) {
	}

	Cube(float sz) :
		Cube(sz, sz, sz) {
	}

	Cube(float w, float h, float d) :
		w(w), h(h), d(d) {
		__init();
	}

	void __init(){
		vecs[0] = { -.5*w, .5*h, .5*d };
		vecs[1] = { -.5*w, -.5*h, .5*d };
		vecs[2] = { .5*w, -.5*h, .5*d };
		vecs[3] = { .5*w, .5*h, .5*d };
		vecs[4] = { .5*w, .5*h, -.5*d };
		vecs[5] = { .5*w, -.5*h, -.5*d };
		vecs[6] = { -.5*w, -.5*h, -.5*d };
		vecs[7] = { -.5*w, .5*h, -.5*d };

		inds[0] = { 0, 1, 2 };
		inds[1] = { 0, 2, 3 };
		inds[2] = { 7, 4, 5 };
		inds[3] = { 7, 5, 6 };
		inds[4] = { 0, 7, 6 };
		inds[5] = { 0, 6, 1 };
		inds[6] = { 4, 3, 2 };
		inds[7] = { 4, 2, 5 };
		inds[8] = { 0, 4, 7 };
		inds[9] = { 0, 3, 4 };
		inds[10] = { 1, 6, 5 };
		inds[11] = { 1, 5, 2 };

		for (int i = 0; i < 12; i++) {
			faces[i] = Face(&vecs[inds[i].i0], &vecs[inds[i].i1], &vecs[inds[i].i2]);
		}

		// calc vertex normals
		for (int i = 0; i < 8; i++) {
			glm::vec3 v{ 0, 0, 0 };
			for (int j = 0; j < 12; j++) {
				if (&vecs[i] == faces[j].v0_ptr || &vecs[i] == faces[j].v1_ptr || &vecs[i] == faces[j].v2_ptr){
					v += faces[j].getNorm();
				}
			}
			v = glm::normalize(v);
			verts[i].pos = vecs[i];
			verts[i].norm = v;
			verts[i].col = { random(1.0), random(1.0), random(1.0) };
		}

		// 1. Create and bind VAO
		glGenVertexArrays(1, &cubeVAO); // Create VAO
		glBindVertexArray(cubeVAO); // Bind VAO (making it active)
		//2. Create and bind VBO
		GLuint cubeVBO = 0;
		// a. Vertex attributes
		glGenBuffers(1, &cubeVBO); // Create VBO ID
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // Bind vertex attributes VBO
		int vertsDataSize = 8 * 9 * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, vertsDataSize, NULL, GL_STREAM_DRAW); // allocate space
		//trace("vertsDataSize =", vertsDataSize);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertsDataSize, &verts[0]); // upload the data


		// b. Indices  uses ELEMENT_ARRAY_BUFFER
		GLuint cubeIndexVBO = 0;
		glGenBuffers(1, &cubeIndexVBO); // Generate buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexVBO); // Bind indices VBO
		int indsDataSize = 12 * 3 * sizeof (GL_UNSIGNED_INT);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indsDataSize, NULL, GL_STATIC_DRAW); // allocate
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indsDataSize, &inds[0]); // upload the data


		const int STRIDE = 9;
		for (int i = 0; i < 3; i++) {
			glEnableVertexAttribArray(i);
		}
		// (x, y, z, nx, ny, nz, r, g, b)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(0)); // pos
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(12)); // norm
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(24)); // col

		// Disable VAO
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	void display(){
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, 8 * 9, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

};
/****************************
*    END DATA STRUCTURES    *
****************************/

#endif //__data_structs__