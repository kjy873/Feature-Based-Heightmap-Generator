#version 330 core

in vec3 passColor;
out vec4 FragColor;
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main(){

	float ambientLight = 1.0;

	vec3 ambient = ambientLight * lightColor;

	vec3 result = ambient * passColor;

	FragColor = vec4(result, 1.0f);
}