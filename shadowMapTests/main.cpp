#define GLEW_STATIC
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

// lazy
using namespace std;
using namespace ijg;

#define BUFFER_OFFSET(i) ((void*)(i))

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}




glm::vec3 getNorm(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
	glm::vec3 a = v2 - v0;
	glm::vec3 b = v1 - v0;
	return glm::normalize(glm::cross(a, b));
}

// data structures
//vector<glm::vec3> vecs;


void initGeom();

int main(void) {
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		//...
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION)); 
	
	ProtoShader* shader = new ProtoShader("shader.vert", "shader.vert");

	initGeom();

	// matrices
	glm::mat4x4 M(1.0);
	glm::mat4x4 V = glm::lookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4x4 P = glm::perspective(75.0, 640.0 / 480.0, 0.1, 1000.0);
	glm::mat4x4 MV = V * M;
	glm::mat4x4 MVP = P * MV;

	// matrix uniforms
	GLuint MV_U = glGetUniformLocation(ProtoShader::getID_2(), "ModelViewMatrix");
	GLuint P_U = glGetUniformLocation(ProtoShader::getID_2(), "ProjectionMatrix");
	GLuint N_U = glGetUniformLocation(ProtoShader::getID_2(), "NormalMatrix");
	GLuint MVP_U = glGetUniformLocation(ProtoShader::getID_2(), "MVP");

	// lights
	glm::vec4 LPOS(-0.5, 0.5, 0.5, 1.0); 
	glm::vec3 KD(0.5, 0.5, 0.5);
	glm::vec3 LD(0.5, 0.5, 0.5);

	// Light Uniforms
	GLuint LPOS_U = glGetUniformLocation(ProtoShader::getID_2(), "LightPosition");
	GLuint KD_U = glGetUniformLocation(ProtoShader::getID_2(), "kd");
	GLuint LD_U = glGetUniformLocation(ProtoShader::getID_2(), "ld");


	
	
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);
	while (!glfwWindowShouldClose(window))
	{
		// update matrices
		glUniformMatrix4fv(MV_U, 1, GL_FALSE, &MV[0][0]);
		glUniformMatrix4fv(MVP_U, 1, GL_FALSE, &MVP[0][0]);
		glm::mat4x4 N(M);
		glUniformMatrix3fv(N_U, 1, GL_FALSE, &N[0][0]); 

		glUniformMatrix4fv(LPOS_U, 1, GL_FALSE, &LPOS[0]);
		glUniformMatrix4fv(KD_U, 1, GL_FALSE, &KD[0]);
		glUniformMatrix3fv(LD_U, 1, GL_FALSE, &LD[0]);
		
		
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
		glBegin(GL_TRIANGLES);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(-0.6f, -0.4f, 0.f);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(0.6f, -0.4f, 0.f);
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(0.f, 0.6f, 0.f);
		glEnd();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void initGeom(){
	// ground plane - faces {0, 1 2}, {0, 2, 3}
	glm::vec3 plane[] { { -0.5, 0.0, 0.5 }, { 0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }, { -0.5, 0.0, -0.5 } };
	glm::vec3 vecs[18] 
	{
		{ -0.5, 0.0, 0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }), { 1.0, 0.0, 0.0 },
		{ 0.5, 0.0, 0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }), { 0.0, 1.0, 0.0 },
		{ 0.5, 0.0, -0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }), { 0.0, 0.0, 1.0 },
		{ -0.5, 0.0, 0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }, { -0.5, 0.0, -0.5 }), { 1.0, 0.0, 0.0 },
		{ 0.5, 0.0, -0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }, { -0.5, 0.0, -0.5 }), { 0.0, 0.0, 1.0 },
		{ -0.5, 0.0, -0.5 }, getNorm({ -0.5, 0.0, 0.5 }, { 0.5, 0.0, -0.5 }, { -0.5, 0.0, -0.5 }), { 1.0, 1.0, 0.0 }
	};
	
	
	GLuint vaoID, vboID;
	// 1. Create and bind VAO
	glGenVertexArrays(1, &vaoID); // Create VAO
	glBindVertexArray(vaoID); // Bind VAO (making it active)
	//2. Create and bind VBO
	// a. Vertex attributes
	glGenBuffers(1, &vboID); // Create VBO ID
	glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind vertex attributes VBO
	int vertsDataSize = 8*3*sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, vertsDataSize, NULL, GL_STREAM_DRAW); // allocate space
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertsDataSize, &vecs); // upload the data

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
	// cube


}