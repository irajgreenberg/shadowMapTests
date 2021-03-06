﻿#version 400

layout(location = 0) out vec4 FragColor;
//layout(location = 0) out float FragDepth;


in vec3 vPos;
in vec3 vNorm;
in vec3 vCol;
in vec2 vTex;

// depth data form light perspective
in vec4 shadowCoords;

uniform mat4 MV;
uniform vec4 LightPos; // Light position in eye coords.
uniform mat3 N;

uniform vec3 camera; // camera

// shadow map
uniform bool isShadowRenderPass; // shadow map pass flag
uniform mat4 LMV;
uniform mat4 LP;
uniform mat4 LMVP;

//uniform sampler2DShadow shadowMap;
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
  vec3 ambient = vCol * 0.105;


//float visibility = texture( shadowMap, vec3(shadowCoords.xy, (shadowCoords.z)/shadowCoords.w) );

  vec3 diffuse = max(dot(lightVec, n), 0.0) * vCol;
  
  // specular
  vec3 viewDir = normalize(camera - vPos);
  vec3 halfDir = normalize(lightVec + viewDir);
  float spec = pow(max(dot(n, halfDir), 0.0), 32.0);
  vec3 specular = vec3(0.9) * spec;

if(shadowCoords.w>1) {
		//check the shadow map texture to see if the fragment is in shadow
		float shadow = textureProj(shadowMap, shadowCoords);
		//darken the diffuse component apprpriately
		diffuse = mix(diffuse, diffuse*shadow, 0.35);
}

FragColor = vec4(ambient + diffuse + specular, 1.0f); 


  //FragDepth = gl_FragCoord.z;
}