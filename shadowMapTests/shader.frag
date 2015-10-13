#version 400

out vec4 FragColor;
out float FragDepth;

in vec3 vPos;
in vec3 vNorm;
in vec3 vertCol;

uniform mat4 ModelViewMatrix;
uniform vec4 LightPosition; // Light position in eye coords.
uniform vec3 Kd; // Diffuse reflectivity
uniform vec3 Ld; // Light source intensity
uniform mat3 NormalMatrix;

uniform vec3 camera; // camera

// shadow map
uniform bool isShadowRenderPass; // shadow map pass flag
uniform mat4 lightModelView;
uniform mat4 lightProjection;
uniform mat4 lightModelViewProjection;


void main() {

  vec3 n = normalize(NormalMatrix * vNorm);
  // Convert normal and position to eye coords
  vec4 eyeCoords = ModelViewMatrix * vec4(vPos,1.0);
  vec3 lightVec = normalize(vec3(LightPosition - eyeCoords));
  
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

  // The diffuse shading equation
  // vec3 LightIntensity = Ld * Kd * max( dot( lightVec, n ), 0.0 ) + ambient;

	if(isShadowRenderPass){
		 FragDepth = gl_FragCoord.z;
	} else {
		FragDepth = gl_FragCoord.z;
		FragColor = vec4(FragDepth, FragDepth, FragDepth, 1.0f);
		//FragColor = vec4(ambient + diffuse + specular, 1.0f);
		//FragDepth = gl_FragCoord.z;
	}

}