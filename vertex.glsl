#version 330 core

in vec3 vPos;
in vec3 vColor;
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 passColor;

void main(){
	gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPos, 1.0);
	passColor = vColor;
}