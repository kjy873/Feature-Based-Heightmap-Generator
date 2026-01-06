#version 330 core

layout(location = 0)in vec3 vPos;
layout(location = 1)in vec3 vColor;
layout(location = 2)in vec3 vNormal;
layout(location = 3)in float inHeight;
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 passColor;
out vec3 normal;
out vec4 fragPos;

void main(){
	vec3 pos = vec3(vPos.x, inHeight, vPos.z);
	gl_Position = projectionTransform * viewTransform * modelTransform * vec4(pos, 1.0);
	
	fragPos = vec4(modelTransform * vec4(pos, 1.0));
	passColor = vColor;
	normal = vNormal;
}