// PolymerNG_Light.h
//

#pragma once

//
// PolymerNGLight
//
class PolymerNGLightLocal : public PolymerNGLight
{
public:
	PolymerNGLightLocal(PolymerNGLightOpts opts);

	const PolymerNGLightOpts *GetOpts() { return &opts; }
private:
	PolymerNGLightOpts opts;
};