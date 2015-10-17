#version 400

out vec4 FragColor;
out float FragDepth;

in vec3 vPos;
in vec3 vNorm;
in vec3 vertCol;

in vec4 shadowCoords;

uniform mat4 MV;
uniform vec4 LightPos; // Light position in eye coords.
uniform vec3 Kd; // Diffuse reflectivity
uniform vec3 Ld; // Light source intensity
uniform mat3 N;

uniform vec3 camera; // camera

// shadow map
uniform bool isShadowRenderPass; // shadow map pass flag
uniform mat4 LMV;
uniform mat4 LP;
uniform mat4 LMVP;

uniform sampler2DShadow shadowMap;


void main() {
	
  if(isShadowRenderPass){
	return;
  }

  vec3 n = normalize(N * vNorm);
  // Convert normal and position to eye coords
  vec4 eyeCoords = MV * vec4(vPos,1.0);
  vec3 lightVec = normalize(vec3(LightPos - eyeCoords));
  
   // ambient
  vec3 ambient = vertCol * 0.105;

  // diffuse
   float diff = max(dot(lightVec, n), 0.0);
   vec3 diffuse = diff * vertCol;
  
  // specular
  vec3 viewDir = normalize(camera - vPos);
  vec3 halfDir = normalize(lightVec + viewDir);
  float spec = pow(max(dot(n, halfDir), 0.0), 32.0);
  vec3 specular = vec3(0.9) * spec;


  if (shadowCoords.w>1){
    float shadow = textureProj(shadowMap, shadowCoords);
    diffuse = mix(diffuse, diffuse*shadow, 0.5);
	 FragColor = vec4(ambient + diffuse + specular, 1.0f);
  } else {
   FragColor = vec4(1, 1, 1, 1.0f);
  }



}