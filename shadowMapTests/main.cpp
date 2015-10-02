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
	glm::vec3 loc;
	float w, h, d;
	Index inds[12];
	glm::vec3 vecs[8];
	Face faces[12];
	Vertex verts[8];
	//GLfloat prims[36 * 9];
	
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

	Cube(glm::vec3 pos, glm::vec3 dim) :
		w(dim.x), h(dim.y), d(dim.z) {
		__init();
	}

	void __init(){
		vecs[0] = { -.5, .5, .5 };
		vecs[1] = { -.5, -.5, .5 };
		vecs[2] = { .5, -.5, .5 };
		vecs[3] = { .5, .5, .5 };
		vecs[4] = { .5, .5, -.5 };
		vecs[5] = { .5, -.5, -.5 };
		vecs[6] = { -.5, -.5, -.5 };
		vecs[7] = { -.5, .5, -.5 };

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
			faces[i] = Face( &vecs[inds[i].i0], &vecs[inds[i].i1], &vecs[inds[i].i2] );
		}

		// calc vertex normals
		for (int i = 0; i < 8; i++) {
			glm::vec3 v{0, 0, 0};
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
		int vertsDataSize = 8*9*sizeof(GLfloat);
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
		glDrawElements(GL_TRIANGLES, 8*9, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	
};


// matrices
glm::mat4 M, V, P, MV, MVP;
glm::mat3 N;
// Light matrices
glm::mat4 LV, LP, LMV, LMVP;

// matrix uniforms
GLuint MV_U, P_U, MVP_U, N_U;
GLuint LMV_U, LP_U, LMVP_U;

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
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);


	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearStencil(0); // clear stencil buffer
	//glClearDepth(1.0f); // 0 is near, 1 is far
	//glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// shader
	ProtoShader* shader = new ProtoShader("shader.vert", "shader.frag");
	shader->bind();


	// matrices
	V = glm::lookAt(glm::vec3(0.0, 25.0, 55.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
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

	// light view for testing
	//V = glm::lookAt(glm::vec3(-0.5, 0.5, 100.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	
	// Light Matrix Uniforms
	LMV_U = glGetUniformLocation(ProtoShader::getID_2(), "LMV");
	LP_U = glGetUniformLocation(ProtoShader::getID_2(), "LP");
	LMVP_U = glGetUniformLocation(ProtoShader::getID_2(), "LMVP");

	// Light Uniforms
	LPOS_U = glGetUniformLocation(ProtoShader::getID_2(), "LightPosition");
	KD_U = glGetUniformLocation(ProtoShader::getID_2(), "Kd");
	LD_U = glGetUniformLocation(ProtoShader::getID_2(), "Ld");

	

	const int cubeCount = 1000;
	Cube cube(5);

	glm::vec3 locs[cubeCount];
	for (auto i = 0; i < cubeCount; ++i) {
		float y = random(0.125f, 2.0f);
		locs[i] = { random(-20.0f, 20.0f), y, random(-20.0f, 20.0f) };
	}

	while (!glfwWindowShouldClose(window))
	{
		LV = glm::lookAt(glm::vec3(LPOS.x, LPOS.y, LPOS.z), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		LP = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
		LMV = LV * M;
		LMVP = LP * LMV;
		glUniformMatrix4fv(LMV_U, 1, GL_FALSE, &LMV[0][0]);
		glUniformMatrix4fv(LP_U, 1, GL_FALSE, &LP[0][0]);
		glUniformMatrix4fv(LMVP_U, 1, GL_FALSE, &LMVP[0][0]);
			
		glUniform4fv(LPOS_U, 1, &LPOS.x);
		glUniform3fv(KD_U, 1, &KD.x);
		glUniform3fv(LD_U, 1, &LD.x);
		
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		M = glm::mat4(1.0f); // set to identity;
		static float rot = 0;
		rotate(rot += .02, glm::vec3(0, 1, 0));

		drawRect({ 65, 1, 65 });


		for (auto i = 0; i < cubeCount; i++) {
			push();
			translate({ locs[i].x, locs[i].y, locs[i].z });
			cube.display();
			pop();
		}
		

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

	Face f1(&vecs.at(0), &vecs.at(1), &vecs.at(2));
	verts.push_back(Vertex(vecs.at(0), f1.getNorm(), glm::vec3(1.0, 0.0, 0.0)));
	verts.push_back(Vertex(vecs.at(1), f1.getNorm(), glm::vec3(0.0, 1.0, 0.0)));
	verts.push_back(Vertex(vecs.at(2), f1.getNorm(), glm::vec3(0.0, 0.0, 1.0)));
	verts.push_back(Vertex(vecs.at(3), f1.getNorm(), glm::vec3(1.0, 0.0, 1.0)));

	vector<GLfloat> prims;
	for (int i = 0; i < verts.size(); i++) {
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
