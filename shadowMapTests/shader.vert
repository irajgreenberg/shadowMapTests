#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexColor;

out vec3 vPos;
out vec3 vNorm;
out vec3 vertCol;

//uniform mat4 ModelViewMatrix;
//uniform mat3 NormalMatrix;
//uniform mat4 ProjectionMatrix;
uniform mat4 MVP; // Projection * ModelView

uniform bool isShadowRenderPass; // shadow map pass flag
uniform mat4 lightModelViewProjection;

void main() {
	
  // set up outs
  vPos = VertexPosition;
  vNorm = VertexNormal;
  vertCol = VertexColor;

  // Convert position to clip coordinates and pass along
  if(isShadowRenderPass){
	gl_Position = lightModelViewProjection * vec4(VertexPosition,1.0);
  } else {
	gl_Position = MVP * vec4(VertexPosition,1.0);
  }
}