#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexColor;
layout (location = 3) in vec2 VertexTexture;

out vec3 vPos;
out vec3 vNorm;
out vec3 vCol;
out vec2 vTex;

// for shadow
//out vec3 vEyeSpaceNormal;
//out vec3 vEyeSpacePos;
out vec4 shadowCoords;

uniform mat4 M;
uniform mat4 MV;
//uniform mat3 N;
//uniform mat4 P;
uniform mat4 MVP; // Projection * ModelView

uniform bool isShadowRenderPass; // shadow map pass flag
uniform mat4 LMVP;

void main() {
	
  // set up outs
  vPos = VertexPosition;
  vNorm = VertexNormal;
  vCol = VertexColor;
  vTex = VertexTexture;

  // for shadows
	shadowCoords = LMVP * vec4(VertexPosition,1.0);
    gl_Position = MVP * vec4(VertexPosition,1.0);

	//gl_Position = shadowCoords;

}