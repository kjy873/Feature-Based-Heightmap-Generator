#version 430 core

in vec3 passColor;
out vec4 FragColor;
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
in vec3 normal;
uniform vec3 lightPos;
in vec4 fragPos;

uniform float HighlightWeight;

void main(){

	vec3 N = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));

	float ambientLight = 0.8;
	vec3 ambient = ambientLight * lightColor;

	vec3 normalVector = normalize(normal);
	vec3 lightDirection = normalize(lightPos - fragPos.xyz);

	float diffuseLight = max(dot(N, lightDirection), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	vec3 result = (ambient + diffuse) * passColor;

	result += HighlightWeight * vec3(0.0f, 1.0f, 0.0f);

	FragColor = vec4(result, 1.0f);
}