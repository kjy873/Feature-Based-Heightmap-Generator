#version 430 core

in vec3 passColor;
out vec4 FragColor;
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
in vec3 normal;
uniform vec3 lightPos;
in vec4 fragPos;

uniform float HighlightWeight;

uniform int DebugMode;
uniform float OverlayAlpha;
uniform ivec2 Res;

uniform sampler2D DebugTexture0;
uniform sampler2D DebugTexture1;

float saturate(float x) { return clamp(x, 0.0, 1.0); }

vec3 GetColorFromDebugTexture(ivec2 PixelCoord){
	
	vec4 t0 = texelFetch(DebugTexture0, PixelCoord, 0);
	vec2 t1 = texelFetch(DebugTexture1, PixelCoord, 0).rg;

	if(DebugMode == 1){
		return vec3 (t0.r);
	}
	if(DebugMode == 2){
		vec2 xy01 = t0.gb * 0.5 + 0.5;
		return vec3 (xy01, 0.0f);
	}
	if(DebugMode == 3){
		float z01 = t0.a * 0.5 + 0.5;     
	float gain = 50.0;                      // 5~50
	z01 = (z01 - 0.5) * gain + 0.5;
	return vec3(clamp(z01, 0.0, 1.0));
	}	
	if(DebugMode == 4){
		return vec3(t1.r);
	}
	if(DebugMode == 5){
		return vec3(t1.g);
	}


	return vec3(0.0f);

}

void main(){

	vec3 N = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));

	float ambientLight = 0.8;
	vec3 ambient = ambientLight * lightColor;

	vec3 normalVector = normalize(normal);
	vec3 lightDirection = normalize(lightPos - fragPos.xyz);

	float diffuseLight = max(dot(N, lightDirection), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	vec3 BaseColor = (ambient + diffuse) * passColor;

	BaseColor += HighlightWeight * vec3(0.0f, 1.0f, 0.0f);

	if(DebugMode == 0){
		FragColor = vec4(BaseColor, 1.0f);
		return;
	}

	vec2 uv = clamp(fragPos.xz, 0.0, 1.0);
	ivec2 PixelCoord = clamp(ivec2(uv * vec2(Res)), ivec2(0), Res - ivec2(1));

	vec3 DebugColor = GetColorFromDebugTexture(PixelCoord);

	vec3 FinalColor = mix(BaseColor, DebugColor, clamp(OverlayAlpha, 0.0, 1.0));

	FragColor = vec4(FinalColor, 1.0f);

	return;

	
}