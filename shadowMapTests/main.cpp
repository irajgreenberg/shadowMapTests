#include "dataStructs.h"


static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}



// matrices
glm::mat4 M, V, P, MV, MVP;
glm::mat3 N;
// Light matrices
glm::mat4 LV, LP, LMV, LMVP, B, BP;

// matrix uniforms
GLuint M_U, MV_U, P_U, MVP_U, N_U;
GLuint LMV_U, LP_U, LMVP_U;

// lights
glm::vec4 LPOS;
glm::vec3 KD, LD;

// Light Uniforms
GLuint LPOS_U, KD_U, LD_U;

// camera
GLuint camera_U;


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

// For shadowmap
GLuint colorMapFBO = 0, depthMapFBO = 0;
GLuint colorMap = 0, depthMap = 0;
const int SHADOW_WIDTH = 1024;
const int SHADOW_HEIGHT = 1024;
void createShadowMap();
GLint isShadowRenderPass = 0; // flag for shader during shadowing pass
GLuint isShadowRenderPass_U = 0;

GLuint myFBO, myColTex, myDepthTex;


int main(void) {

	// initiaze GLFW window
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	window = glfwCreateWindow(1024, 768, "Simple example", NULL, NULL);
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
	//ProtoShader* shadowShader = new ProtoShader("shadow.vert", "shadow.frag");
	shader->bind();


	// matrix uniforms
	M_U = glGetUniformLocation(ProtoShader::getID_2(), "M");
	MV_U = glGetUniformLocation(ProtoShader::getID_2(), "MV");
	P_U = glGetUniformLocation(ProtoShader::getID_2(), "P");
	N_U = glGetUniformLocation(ProtoShader::getID_2(), "N");
	MVP_U = glGetUniformLocation(ProtoShader::getID_2(), "MVP");

	// Light Matrix Uniforms
	LMV_U = glGetUniformLocation(ProtoShader::getID_2(), "LMV");
	LP_U = glGetUniformLocation(ProtoShader::getID_2(), "LP");
	LMVP_U = glGetUniformLocation(ProtoShader::getID_2(), "LMVP");

	// Light Uniforms
	LPOS_U = glGetUniformLocation(ProtoShader::getID_2(), "LightPos");
	//KD_U = glGetUniformLocation(ProtoShader::getID_2(), "Kd");
	//LD_U = glGetUniformLocation(ProtoShader::getID_2(), "Ld");


	camera_U = glGetUniformLocation(ProtoShader::getID_2(), "camera");
	// shadow flag boolean
	isShadowRenderPass_U = glGetUniformLocation(ProtoShader::getID_2(), "isShadowRenderPass");

	glm::vec3 camera(0.0, 50, 67);
	glm::vec4 light(-23, 80, 77, 1.0);

	// matrices
	V = glm::lookAt(camera, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	P = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	//P = glm::ortho<float>(-50, 50, -50, 50, -1, 200);

	MV = V * M;
	N = glm::transpose(glm::inverse(glm::mat3(MV)));
	MVP = P * MV;


	// lights
	LPOS = light;
	//KD = glm::vec3(.75, .75, .75);
	//LD = glm::vec3(1, 1, 1);

	//LP = glm::ortho(-10, 10, -10, 10, -10, 20);
	LV = glm::lookAt(glm::vec3(LPOS), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	LMV = LV * glm::mat4(1.0);
	LP = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	//LP = glm::ortho<float>(-50, 50, -50, 50, -1, 200);
	B = glm::mat4(
		0.5f*1.01, 0.0, 0.0, 0.0,
		0.0, 0.5f * 1.01, 0.0, 0.0,
		0.0, 0.0, 0.5f * 1.01, 0.0,
		0.5f * 1.01, 0.5f * 1.01, 0.5f * 1.01, 1.0
		);

	//B = glm::mat4(1.0);

	BP = B * LP;
	LMVP = BP*LMV;

	// light view for testing
	//V = glm::lookAt(glm::vec3(LPOS), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));






	// CUBES
	const int cubeCount = 170;
	Cube cubes[cubeCount];

	glm::vec3 locs[cubeCount];
	for (auto i = 0; i < cubeCount; ++i) {
		float y = 0;
		locs[i] = { random(-30.0f, 30.0f), y, random(-30.0f, 30.0f) };
		cubes[i] = Cube(random(1, 3), random(1, 17), random(1, 3));
	}

	// TOROID
	Toroid t(24, 24, 4, 14);





	// update in shader
	glUniformMatrix4fv(M_U, 1, GL_FALSE, &M[0][0]);
	glUniformMatrix4fv(MV_U, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(MVP_U, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix3fv(N_U, 1, GL_FALSE, &N[0][0]);

	glUniformMatrix4fv(LMV_U, 1, GL_FALSE, &LMV[0][0]);
	glUniformMatrix4fv(LP_U, 1, GL_FALSE, &LP[0][0]);
	glUniformMatrix4fv(LMVP_U, 1, GL_FALSE, &LMVP[0][0]);


	// create shadow map
	createShadowMap();
	GLuint texID = glGetUniformLocation(ProtoShader::getID_2(), "shadowMap");

	while (!glfwWindowShouldClose(window))
	{
		M = glm::mat4(1.0f); // set to identity;

		//MV = V * M;
		//N = glm::transpose(glm::inverse(glm::mat3(MV)));
		//MVP = P * MV;
		//// update in shader
		//glUniformMatrix4fv(MV_U, 1, GL_FALSE, &MV[0][0]);
		//glUniformMatrix4fv(MVP_U, 1, GL_FALSE, &MVP[0][0]);
		//glUniformMatrix3fv(N_U, 1, GL_FALSE, &N[0][0]);


		glUniform4fv(LPOS_U, 1, &LPOS.x);
		//	glUniform3fv(KD_U, 1, &KD.x);
		//	glUniform3fv(LD_U, 1, &LD.x);

		glUniform3fv(camera_U, 1, &camera.x);

		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);


		static float rot = 0;
		//rotate(rot += .0025, glm::vec3(0, 1, 0));

		/*****************************
		* render pass 1 to depth map *
		*****************************/
		isShadowRenderPass = 1;
		glUniform1i(isShadowRenderPass_U, isShadowRenderPass);


		glBindFramebuffer(GL_FRAMEBUFFER, myFBO); // bind depth buffer
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // bind depth buffer
	
		glCullFace(GL_FRONT);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glClear(GL_DEPTH_BUFFER_BIT);

		drawRect({ 135, 1, 135 });
		for (auto i = 0; i < cubeCount; i++) {
			push();
			translate({ locs[i].x, locs[i].y + cubes[i].h / 2.0, locs[i].z });
			cubes[i].display();
			pop();
		}

		static float cntr = 0;
		push();
		translate({ 0, 14.0, 0 });
		rotate(cntr += .02, glm::vec3(.85, .45, .25));
		t.display();
		pop();

		// unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// prepare for screen rendering
		glCullFace(GL_BACK);


		/**************************
		* render pass 2 to screen *
		**************************/
		//glDrawBuffer(GL_BACK_LEFT);
		isShadowRenderPass = 0;
		glUniform1i(isShadowRenderPass_U, isShadowRenderPass);

		//glDrawBuffer(GL_BACK_LEFT);
		//glActiveTexture(myDepthTex);
		//glBindTexture(GL_TEXTURE_2D, m_shadowMap);

		//glBindTexture(GL_TEXTURE_2D, myColTex);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawRect({ 135, 1, 135 });


		for (auto i = 0; i < cubeCount; i++) {
			push();
			translate({ locs[i].x, locs[i].y + cubes[i].h / 2.0, locs[i].z });
			cubes[i].display();
			pop();
			}

			static float cntr2 = 0;
			push();
			translate({ 0, 14.0, 0 });
			rotate(cntr2 += .02, glm::vec3(.85, .45, .25));
			t.display();
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

	Face f1(&vecs.at(0), &vecs.at(1), &vecs.at(2));
	verts.push_back(Vertex(vecs.at(0), f1.getNorm(), glm::vec3(1.0, 0.0, 0.0), { 0.0, 0.0 }));
	verts.push_back(Vertex(vecs.at(1), f1.getNorm(), glm::vec3(0.0, 1.0, 0.0), { 1.0, 0.0 }));
	verts.push_back(Vertex(vecs.at(2), f1.getNorm(), glm::vec3(0.0, 0.0, 1.0), { 1.0, 1.0 }));
	verts.push_back(Vertex(vecs.at(3), f1.getNorm(), glm::vec3(1.0, 0.0, 1.0), { 0.0, 1.0 }));

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
		prims.push_back(verts.at(i).tex.x);
		prims.push_back(verts.at(i).tex.y);
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

	const int STRIDE = 11;
	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(i);
	}
	// (x, y, z, nx, ny, nz, r, g, b)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(0)); // pos
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(12)); // norm
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(24)); // col
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof (GLfloat), BUFFER_OFFSET(36)); // tex

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
	glUniformMatrix4fv(M_U, 1, GL_FALSE, &M[0][0]);
	glUniformMatrix4fv(MV_U, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(MVP_U, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix3fv(N_U, 1, GL_FALSE, &N[0][0]);

	LMV = LV * M;// glm::mat4(1.0);
	BP = B * LP;
	LMVP = BP * LMV;
	glUniformMatrix4fv(LMV_U, 1, GL_FALSE, &LMV[0][0]);
	glUniformMatrix4fv(LP_U, 1, GL_FALSE, &LP[0][0]);
	glUniformMatrix4fv(LMVP_U, 1, GL_FALSE, &LMVP[0][0]);
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

void createShadowMap() {
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &myDepthTex);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, myDepthTex);

	GLfloat border[] = { 1.0f, .0f, .0f, .0f };
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	
	//GLuint FramebufferName = 0;
	glGenFramebuffers(1, &myFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, myFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, myDepthTex, 0);

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO is broken \n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void createShadowMap() {
//	glGenFramebuffers(1, &depthMapFBO); // create FBO
//	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // bind FBO
//	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
//
//	glGenTextures(1, &depthMap);
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, depthMap);
//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
//		//SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
//		1024, 1024, 0, GL_RGB, GL_FLOAT, NULL);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
//
//	
//	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0); // set texture as color attachment
//	//glDrawBuffer(GL_NONE); // dont need color, but FBO needs color buffer
//	//glReadBuffer(GL_NONE);
//	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
//	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
//
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//		std::cout << "FBO for texture setup failure" << std::endl;
//
//	// check staus
//	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//	if (status == GL_FRAMEBUFFER_COMPLETE){
//		std::cout << "FBO setup successful" << std::endl;
//	}
//	else {
//		std::cout << "FBO setup failure" << std::endl;
//	}
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}