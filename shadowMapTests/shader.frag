#version 400

in vec3 LightIntensity;
in vec3 vertCol;

layout( location = 0 ) out vec4 FragColor;

// shadow
uniform bool isShadowRenderPass;
//uniform int isShadowRenderPass;

void main() {
if (isShadowRenderPass) {
		FragColor = vec4(LightIntensity*vertCol, 1.0);
	} else {
		FragColor = vec4(LightIntensity, 1.0);
	}
}