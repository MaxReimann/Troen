#version 130

uniform sampler2D sceneLayer;
uniform sampler2D idLayer;
// uniform sampler2D pongLayer;
uniform sampler2D oldLayer;
uniform float healthNormalized;
uniform float time;
uniform float velocity;
uniform float timeFactor;
uniform float timeSinceLastHit;
uniform float timeSinceLastBeat;

float sigma =
		+ 4
		+ 3 * (sin(time*2.f) + 1.f)/2.f
		+ 4 * max(10, 1000 - timeSinceLastBeat) / 1000;

const float pi = 3.14159265f;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

//////////////////////////////////////////////////////////////////////////
// Final Color Composition
//////////////////////////////////////////////////////////////////////////


//// [DataArena hack]: For some reason, writing and reading flip flop from framebuffers passes does not work when using omegalib
// Therefore, we implement the blur shader in one single pass, which uses much more texture reads than the two pass filter did 
// before, and results are not as good as before. There should be a way to use the framebuffers, the reflection framebuffer did work.
// Not sure whats causing this.. 
void main(void)
{
	vec2 st = gl_TexCoord[0].st;

	vec4 sceneColor = texture2D(sceneLayer, st);

	vec4 attrib = texture2D(idLayer, st);
	float glowIntensity = attrib.y;
	float objID = attrib.x;
	vec4 blurredPix;

	if (objID == 1.0f && glowIntensity > 0.f)
	{
		float blurSizeV;
		float blurSizeH;
		float numBlurPixelsPerSide;
		vec2 blurMultiplyVecH;
		vec2 blurMultiplyVecV;
		vec2 textureSize = textureSize(sceneLayer, 0);

		numBlurPixelsPerSide = 8.0f;
		blurMultiplyVecH      = vec2(1.0f, 0.0f);
		blurSizeH = 1.f / textureSize.y * glowIntensity;

		blurMultiplyVecV      = vec2(0.0f, 1.0f);
		blurSizeV = 1.f / textureSize.x * glowIntensity;

		// Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
		vec3 incrementalGaussian;
		incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
		incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
		incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

		vec4 avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		float coefficientSum = 0.0f;

		// Take the central sample first...
		avgValue += texture2D(sceneLayer, st.xy) * incrementalGaussian.x;
		coefficientSum += incrementalGaussian.x;
		incrementalGaussian.xy *= incrementalGaussian.yz;

		// Go through the remaining 8 vertical samples (4 on each side of the center)
		for (float i = 1.0f; i <= numBlurPixelsPerSide; i++)
		{
			avgValue += texture2D(sceneLayer, st.xy - i * blurSizeH *
							  blurMultiplyVecH) * incrementalGaussian.x;
			avgValue += texture2D(sceneLayer, st.xy + i * blurSizeH *
							  blurMultiplyVecH) * incrementalGaussian.x;
			
			avgValue += texture2D(sceneLayer, st.xy - i * blurSizeV *
							  blurMultiplyVecV) * incrementalGaussian.x;
			avgValue += texture2D(sceneLayer, st.xy + i * blurSizeV *
							  blurMultiplyVecV) * incrementalGaussian.x;

			avgValue += texture2D(sceneLayer, st.xy - i * blurSizeV *
							  vec2(1,1)) * incrementalGaussian.x;
			avgValue += texture2D(sceneLayer, st.xy + i * blurSizeV *
							  vec2(-1,-1)) * incrementalGaussian.x;

			avgValue += texture2D(sceneLayer, st.xy - i * blurSizeV *
							  vec2(1,-1)) * incrementalGaussian.x;
			avgValue += texture2D(sceneLayer, st.xy + i * blurSizeV *
							  vec2(-1,1)) * incrementalGaussian.x;

			coefficientSum += 8 * incrementalGaussian.x;
			incrementalGaussian.xy *= incrementalGaussian.yz;
		}

		blurredPix = avgValue / coefficientSum;


	} else
	{
		blurredPix = sceneColor;
	}




	// blur again to eliminate pixelation which is caused by implicit upscaling
	// const float diff = 0.001;
	// vec4 pongColor =  (4 * texture2D(pongLayer, st)
	// 	+ texture2D(pongLayer, st + vec2(diff))
	// 	+ texture2D(pongLayer, st + vec2(-diff))
	// 	+ texture2D(pongLayer, st + vec2(diff, -diff))
	// 	+ texture2D(pongLayer, st + vec2(-diff, diff))
	// ) / 8;

	vec4 oldColor = texture2D(oldLayer, st);

	// motion blur is framerate independent
	float motionBlurFactor = smoothstep(300.0, 450.0, velocity);
	float oldFrameWeight = max(0.1, pow(motionBlurFactor * 0.8, timeFactor));
	float newFrameWeight = 1 - oldFrameWeight;

	st = 1 * (st - vec2(0.5));

	float enableDamageHUD = healthNormalized;
	float circleTransparency = st.x * st.x + st.y * st.y;
	float pulsating = (sin(time*2.f) + 1.f)/3.f + 0.4 ;
	circleTransparency = (1-enableDamageHUD)*pulsating *  min(1, 2 * circleTransparency);
	float hitPulse = 20.0 / max(timeSinceLastHit, 50);
	circleTransparency += hitPulse;

	vec3 reddedColor = vec3(1.0,0.0,0.0);


	vec4 newColor = blurredPix * glowIntensity * 1.5 + sceneColor * (1 - glowIntensity);
	newColor = mix(newColor, vec4(reddedColor, 1.f), circleTransparency);

	gl_FragColor = newFrameWeight * newColor + oldFrameWeight * oldColor;

	return;
}