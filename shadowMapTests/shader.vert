#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexColor;

out vec3 LightIntensity;
out vec3 vertCol;

uniform vec4 LightPosition; // Light position in eye coords.
uniform vec3 Kd; // Diffuse reflectivity
uniform vec3 Ld; // Light source intensity

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP; // Projection * ModelView

// shadow
//uniform int isShadowRenderPass;

void main() {

vertCol = VertexColor;
  // Convert normal and position to eye coords
  vec3 tnorm = normalize( NormalMatrix * VertexNormal);
  vec4 eyeCoords = ModelViewMatrix * vec4(VertexPosition,1.0);
  vec3 s = normalize(vec3(LightPosition - eyeCoords));
  
  // The diffuse shading equation
  LightIntensity = Ld * Kd * max( dot( s, tnorm ), 0.0 );
  
  // Convert position to clip coordinates and pass along
  gl_Position = MVP * vec4(VertexPosition,1.0);
}