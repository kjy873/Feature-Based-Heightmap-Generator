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

uniform sampler2D DebugTexture2;

const uint MASK_ELEVATION = 1u << 0;
const uint MASK_GRADIENT  = 1u << 1;
const uint MASK_NOISE     = 1u << 2;

float saturate(float x) { return clamp(x, 0.0, 1.0); }

vec4 GetColorFromDebugTexture(ivec2 PixelCoord){
	
	vec4 t0 = texelFetch(DebugTexture0, PixelCoord, 0);
	vec4 t1 = texelFetch(DebugTexture1, PixelCoord, 0);
	vec4 t2 = texelFetch(DebugTexture2, PixelCoord, 0);

	float Alpha = OverlayAlpha;

	if(DebugMode == 1){
		return vec4(vec3(t0.r), Alpha);
	}
	if(DebugMode == 2){
		vec2 xy01 = t0.gb * 0.5 + 0.5;
		return vec4 (xy01, 0.0f, Alpha);
	}
	if(DebugMode == 3){
		float z01 = t0.a * 0.5 + 0.5;     
	float gain = 50.0;                      // 5~50
	z01 = (z01 - 0.5) * gain + 0.5;
	return vec4(vec3(clamp(z01, 0.0, 1.0)), Alpha);
	}	
	if(DebugMode == 4){
		return vec4(t1.r, t1.r, t1.r, Alpha);
	}
	if(DebugMode == 5){
		return vec4(vec3(t1.g), Alpha);
	}
	if(DebugMode == 6){
		float v = t1.b;
		v = clamp(v * 1000.0, 0.0, 1.0);
		return vec4(vec3(v), Alpha);

		//float v = t1.b;  // 값이 있는지 없는지만 검사
		//return (abs(v) > 1e-8) ? vec3(1.0) : vec3(0.0);
	}
	if(DebugMode == 7){
		float v = t1.a;
		v = clamp(v * 1000.0, 0.0, 1.0);
		return vec4(vec3(v), Alpha);

		//float v = t1.b;  // 값이 있는지 없는지만 검사
		//return (abs(v) > 1e-8) ? vec3(1.0) : vec3(0.0);
	}
	if(DebugMode == 8){
		uint mask = uint(t2.r + 0.5);

		if((mask & MASK_ELEVATION) != 0u){
			return vec4(1.0, 0.0, 1.0, 1.0);
		}

		if((mask & MASK_GRADIENT) != 0u){
			int direction = int(round(t2.b));

			vec3 rgb = (direction < 0) ? vec3(1,0,0) :
					   (direction > 0) ? vec3(0,0,1) :
										 vec3(0);

			float denom = max(abs(t2.a), 1e-8);
			float a = saturate(abs(t2.g) / denom);

			return vec4(rgb, a);
		}

		return vec4(0,0,0,0);
	}

	return vec4(0.0f);

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

	vec4 DebugColor = GetColorFromDebugTexture(PixelCoord);

	vec3 Debug = DebugColor.rgb;

	vec3 FinalColor = mix(BaseColor, Debug, clamp(DebugColor.a, 0.0, 1.0));

	FragColor = vec4(FinalColor, 1.0f);



	return;

	
}