#version 330 core

layout(location = 0)in vec3 vPos;
layout(location = 1)in vec3 vColor;
layout(location = 2)in vec3 vNormal;
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 passColor;
out vec3 normal;
out vec4 fragPos;

void main(){
	gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPos, 1.0);
	
	fragPos = vec4(modelTransform * vec4(vPos, 1.0));
	passColor = vColor;
	normal = vNormal;
}