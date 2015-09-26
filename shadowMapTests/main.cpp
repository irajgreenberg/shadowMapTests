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

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}


// load shader
//unsigned long getFileLength(ifstream& file);
//int loadshader(char* filename, GLchar** ShaderSource, unsigned long* len);
//int unloadshader(GLubyte** ShaderSource);



int main(void)
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		//...
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION)); 

	//ijg::ProtoShader shader("shader.vert", "shader.frag");

	std::ifstream file("../shader.vert");
	std::string line, block;
	if (file.is_open()) {

		while (!file.eof()) {
			std::getline(file, line);
			block.append(line);
			block.append(std::string("\n"));
		}
		std::cout << block;
		file.close();
	}
	else {
		std::cout << "Unable to open file";
	}

	
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


//
//unsigned long getFileLength(ifstream& file)
//{
//	if (!file.good()) return 0;
//
//	unsigned long pos = file.tellg();
//	file.seekg(0, ios::end);
//	unsigned long len = file.tellg();
//	file.seekg(ios::beg);
//
//	return len;
//}
//
//int loadshader(char* filename, GLchar** ShaderSource, unsigned long* len)
//{
//	ifstream file;
//	file.open(filename, ios::in); // opens as ASCII!
//	if (!file) return -1;
//
//	len = getFileLength(file);
//
//	if (len == 0) return -2;   // Error: Empty File 
//
//	*ShaderSource = (GLubyte*) new char[len + 1];
//	if (*ShaderSource == 0) return -3;   // can't reserve memory
//
//	// len isn't always strlen cause some characters are stripped in ascii read...
//	// it is important to 0-terminate the real length later, len is just max possible value... 
//	*ShaderSource[len] = 0;
//
//	unsigned int i = 0;
//	while (file.good())
//	{
//		*ShaderSource[i] = file.get();       // get character from file.
//		if (!file.eof())
//			i++;
//	}
//
//	*ShaderSource[i] = 0;  // 0-terminate it at the correct position
//
//	file.close();
//
//	return 0; // No Error
//}
//
//
//int unloadshader(GLubyte** ShaderSource)
//{
//	if (*ShaderSource != 0)
//		delete[] * ShaderSource;
//	*ShaderSource = 0;
//}