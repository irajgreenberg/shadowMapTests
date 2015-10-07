#version 400

in vec3 LightIntensity;
in vec3 vertCol;

layout( location = 0 ) out vec4 FragColor;

void main() {
    // FragColor = vec4(LightIntensity*vertCol, 1.0);
	FragColor = vec4(LightIntensity, 1.0);
}