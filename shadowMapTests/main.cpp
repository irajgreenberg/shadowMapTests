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
#include <stack>


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

// data structures
struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec3 col;

	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec3& col):
	pos(pos), norm(norm), col(col) {}
};

struct Face {
	glm::vec3 v0, v1, v2;

	Face(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) :
		v0(v0), v1(v1), v2(v2) {}

	glm::vec3 getNorm() {
		glm::vec3 a = v2 - v0;
		glm::vec3 b = v1 - v0;
		return glm::normalize(glm::cross(b, a));
	}
};


// matrices
glm::mat4 M, V, P, MV, MVP;
glm::mat3 N;

// matrix uniforms
GLuint MV_U, P_U, MVP_U, N_U;

// lights
glm::vec4 LPOS;
glm::vec3 KD, LD;

// Light Uniforms
GLuint LPOS_U, KD_U, LD_U; 


GLuint vaoID, vboID;
vector<glm::vec3> vecs;
vector<Vertex> verts;
void initGeom();
void drawRect(glm::vec3);
void drawCube(glm::vec3);

std::stack <glm::mat4> matrixStack; 
void push();
void pop();
void concat();
void translate(const glm::vec3& v);
void rotate(float ang, const glm::vec3& axis);
void scale(const glm::vec3& v);

int main(void) {
	
	// initiaze GLFW window
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

	// load modern GL drivers
	//glewExperimental = GL_TRUE;
	//GLenum err = glewInit();

	//// get version info
	//const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	//const GLubyte* version = glGetString(GL_VERSION); // version as a string
	//printf("Renderer: %s\n", renderer);
	//printf("OpenGL version supported %s\n", version);


	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glClearStencil(0); // clear stencil buffer
	//glClearDepth(1.0f); // 0 is near, 1 is far
	//glDepthFunc(GL_LEQUAL);
		//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// shader
	ProtoShader* shader = new ProtoShader("shader.vert", "shader.frag");
	shader->bind();


	// matrices
	V = glm::lookAt(glm::vec3(0.0, 10.0, 25.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	P = glm::perspective(45.0f, 4.0f/3.0f, 0.1f, 1000.0f);

	// matrix uniforms
	MV_U = glGetUniformLocation(ProtoShader::getID_2(), "ModelViewMatrix");
	P_U = glGetUniformLocation(ProtoShader::getID_2(), "ProjectionMatrix");
	N_U = glGetUniformLocation(ProtoShader::getID_2(), "NormalMatrix");
	MVP_U = glGetUniformLocation(ProtoShader::getID_2(), "MVP");

	// lights
	LPOS = glm::vec4(-0.5, 0.5, 100.0, 1.0);
	KD = glm::vec3(1, 1, 1);
	LD = glm::vec3(1, 1, 1);

	// Light Uniforms
	LPOS_U = glGetUniformLocation(ProtoShader::getID_2(), "LightPosition");
	KD_U = glGetUniformLocation(ProtoShader::getID_2(), "Kd");
	LD_U = glGetUniformLocation(ProtoShader::getID_2(), "Ld");

	while (!glfwWindowShouldClose(window))
	{
		glUniform4fv(LPOS_U, 1, &LPOS.x);
		glUniform3fv(KD_U, 1, &KD.x);
		glUniform3fv(LD_U, 1, &LD.x);
		
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		M = glm::mat4(1.0f); // set to identity
		//translate(glm::vec3(0, 0, -100));
		static float rot = 0;

		rotate(rot += .02, glm::vec3(0, 1, 0));

		drawRect({ 65, 1, 65 });

		push();
		translate({ -2, 2, 0 });
		drawCube({ 4, 4, 4 });
		pop();

		push();
		translate({ 4, 1, 1 });
		drawCube({ 2, 2, 2 });
		pop();

		push();
		translate({ -3, 1, -5 });
		drawCube({ 2, 2, 2 });
		pop();

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void drawRect(glm::vec3 scl){
	if (vecs.size() > 0) vecs.clear();
	if (verts.size() > 0) verts.clear();

	vecs.push_back(glm::vec3(-0.5*scl.x, 0.0, 0.5*scl.z));
	vecs.push_back(glm::vec3(0.5*scl.x, 0.0, 0.5*scl.z));
	vecs.push_back(glm::vec3(0.5*scl.x, 0.0, -0.5*scl.z));
	vecs.push_back(glm::vec3(-0.5*scl.x, 0.0, -0.5*scl.z));

	Face f1(vecs.at(0), vecs.at(1), vecs.at(2));
	verts.push_back(Vertex(vecs.at(0), f1.getNorm(), glm::vec3(1.0, 0.0, 0.0)));
	verts.push_back(Vertex(vecs.at(1), f1.getNorm(), glm::vec3(0.0, 1.0, 0.0)));
	verts.push_back(Vertex(vecs.at(2), f1.getNorm(), glm::vec3(0.0, 0.0, 1.0)));
	verts.push_back(Vertex(vecs.at(3), f1.getNorm(), glm::vec3(1.0, 0.0, 1.0)));

	vector<GLfloat> prims;
	for (int i = 0; i < verts.size(); i++) {
		//trace(&verts.at(i));
		prims.push_back(verts.at(i).pos.x);
		prims.push_back(verts.at(i).pos.y);
		prims.push_back(verts.at(i).pos.z);
		prims.push_back(verts.at(i).norm.x);
		prims.push_back(verts.at(i).norm.y);
		prims.push_back(verts.at(i).norm.z);
		prims.push_back(verts.at(i).col.x);
		prims.push_back(verts.at(i).col.y);
		prims.push_back(verts.at(i).col.z);
	}

	// 1. Create and bind VAO
	glGenVertexArrays(1, &vaoID); // Create VAO
	glBindVertexArray(vaoID); // Bind VAO (making it active)
	//2. Create and bind VBO
	// a. Vertex attributes
	glGenBuffers(1, &vboID); // Create VBO ID
	glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind vertex attributes VBO
	int primsDataSize = prims.size() * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, primsDataSize, NULL, GL_STREAM_DRAW); // allocate space
	//trace("vertsDataSize =", vertsDataSize);
	glBufferSubData(GL_ARRAY_BUFFER, 0, primsDataSize, &prims[0]); // upload the data

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

	glBindVertexArray(vaoID);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void drawCube(glm::vec3 scl){
	//glm::vec3 v(100, 100, 100);
	// top
	push();
	translate(glm::vec3(0, scl.y / 2, 0));
	drawRect({ scl.x, 1, scl.x });
	pop();

	// bottom
	push();
	translate(glm::vec3(0, -scl.y / 2, 0));
	rotate(3.124, glm::vec3(1.0, 0, 0));
	drawRect({ scl.x, 1, scl.z });
	pop();

	// left
	push();
	translate(glm::vec3(-scl.x / 2, 0, 0));
	rotate(3.124 / 2, glm::vec3(0, 0, 1));
	drawRect({ scl.x, 1, scl.z });
	pop();

	// right
	push();
	translate(glm::vec3(scl.x / 2, 0, 0));
	rotate(-3.124 / 2, glm::vec3(0, 0, 1));
	drawRect({ scl.x, 1, scl.z });
	pop();

	// front
	push();
	translate(glm::vec3(0, 0, scl.z / 2));
	rotate(3.124 / 2, glm::vec3(1, 0, 0));
	drawRect({ scl.x, 1, scl.z });
	pop();

	// back
	push();
	translate(glm::vec3(0, 0, -scl.z / 2));
	rotate(-3.124 / 2, glm::vec3(1, 0, 0));
	drawRect({ scl.x, 1, scl.z });
	pop();
}

void push(){
	// push current transformation matrix onto stack
	matrixStack.push(M);

}

// reset transformation matrix with stored matrix on stack
void pop(){

	// reset current transformation matrix with one on top of stack
	M = matrixStack.top();

	// pop transformation matrix off top of stack
	matrixStack.pop();

	// rebuild matrices and update on GPU
	concat();
}

void concat(){
	//M = glm::mat4(1.0f);
	//push();
	MV = V * M;
	N = glm::transpose(glm::inverse(glm::mat3(MV)));
	MVP = P * MV;
	// update in shader
	glUniformMatrix4fv(MV_U, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(MVP_U, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix3fv(N_U, 1, GL_FALSE, &N[0][0]);
}

void translate(const glm::vec3& v){
	M = glm::translate(M, v);
	concat();
}
void rotate(float ang, const glm::vec3& axis) {
	M = glm::rotate(M, ang, axis);
	concat();
}
void scale(const glm::vec3& v) {
	M = glm::scale(M, v);
	concat();
}
