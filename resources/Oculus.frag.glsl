#version 130

uniform sampler2D warpTexture;

out vec4 fragmentColour;

/* Parameters from mt3d forums */
const vec2 LeftLensCenter = vec2(0.2863248, 0.5);
const vec2 RightLensCenter = vec2(0.7136753, 0.5);
const vec2 LeftScreenCenter = vec2(0.25, 0.5);
const vec2 RightScreenCenter = vec2(0.75, 0.5);
const vec2 Scale = vec2(0.1469278, 0.2350845);
const vec2 ScaleIn = vec2(4, 2.5);
const vec4 HmdWarpParam   = vec4(1, 0.22, 0.24, 0);


/* Main warp */
vec2 HmdWarp(vec2 in01, vec2 LensCenter)
{
	vec2 theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
		HmdWarpParam.z * rSq * rSq +
		HmdWarpParam.w * rSq * rSq * rSq);
	return LensCenter + Scale * rvector;
}

void main()
{
	vec2 LensCenter, ScreenCenter;

	if (gl_FragCoord.x < 640) {
		LensCenter =  LeftLensCenter;
		ScreenCenter = LeftScreenCenter; 
	} else {
		LensCenter = RightLensCenter;
		ScreenCenter = RightScreenCenter;
	}

	vec2 oTexCoord = gl_FragCoord.xy / vec2(1280, 800);

	vec2 tc = HmdWarp(oTexCoord, LensCenter);
	if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))
	{
		fragmentColour = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	tc.x = gl_FragCoord.x < 640 ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));
 	fragmentColour = texture(warpTexture, vec2(tc.x, 1-tc.y));
}
