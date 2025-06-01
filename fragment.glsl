#version 330 core

in vec3 passColor;
out vec4 FragColor;
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
in vec3 normal;
uniform vec3 lightPos;
in vec4 fragPos;

void main(){

	float ambientLight = 0.8;
	vec3 ambient = ambientLight * lightColor;

	vec3 normalVector = normalize(normal);
	vec3 lightDirection = normalize(lightPos - fragPos.xyz);

	float diffuseLight = max(dot(normalVector, lightDirection), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	vec3 result = (ambient + diffuse) * passColor;

	FragColor = vec4(result, 1.0f);
}