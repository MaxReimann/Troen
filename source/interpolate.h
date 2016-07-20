// Interpolate

#define troenlinearInterpolation(t)       (t)

#define troensmoothstep(t)   ((t) * (t) * (3 - 2 * (t)))
#define troensmoothstep2(t)  (troensmoothstep(troensmoothstep(t)))
#define troensmoothstep3(t)  (troensmoothstep(troensmoothstep2(t)))

#define smootherstep(t) ((t) * (t) * (t) * ((t) * (6 * (t) - 15) + 10))

#define troensquared(t)      ((t) * (t))
#define troeninvsquared(t)   (1 - (1 - (t)) * (1 - (t)))

#define troencubed(t)        ((t) * (t) * (t))
#define troeninvcubed(t)     (1 - (1 - (t)) * (1 - (t)) * (1 - (t)))

#define troensin(t)          (sin(t * 1.57079632679489661923))
#define troeninvsin(t)       (1 - sin((1 - (t)) * 1.57079632679489661923))


#define troensmoothstep_ext(t, l, r) \
	((t) < (l) ? 0 : (r) < (t) ? 1 : smoothstep(((t)-(l)) / ((r)-(l))))



// Several interpolation methods in action: http://sol.gfxile.net/interpolation/

enum InterpolationMethod
{
	InterpolateLinear
	, InterpolateSmoothStep
	, InterpolateSmoothStep2
	, InterpolateSmoothStep3
	, InterpolateSmootherStep // Ken Perlin
	, InterpolateSquared
	, InterpolateInvSquared
	, InterpolateCubed
	, InterpolateInvCubed
	, InterpolateSin          // strong in, soft out
	, InterpolateInvSin       // soft in, strong out
};


template<typename T>
inline const T interpolate(
	const T t
	, const InterpolationMethod function = InterpolateLinear)
{
	switch (function)
	{
	case InterpolateSmoothStep:
		return troensmoothstep(t);
	case InterpolateSmoothStep2:
		return troensmoothstep2(t);
	case InterpolateSmoothStep3:
		return troensmoothstep3(t);
	case InterpolateSmootherStep:
		return smootherstep(t);
	case InterpolateSquared:
		return troensquared(t);
	case InterpolateInvSquared:
		return troeninvsquared(t);
	case InterpolateCubed:
		return troencubed(t);
	case InterpolateInvCubed:
		return troeninvcubed(t);
	case InterpolateSin:
		return static_cast<T>(troensin(t));
	case InterpolateInvSin:
		return static_cast<T>(troeninvsin(t));
	default:
	case InterpolateLinear:
		return troenlinearInterpolation(t);
	}
}