#pragma once

//
// EditorLightSystem
//
class EditorLightSystem
{
public:
	EditorLightSystem();

	void			SetLightOpts(int spriteNum, PolymerNGLightOpts &opts);
private:
	PolymerNGLight *lights[MAXSPRITES];
};

extern EditorLightSystem editorLightSystem;