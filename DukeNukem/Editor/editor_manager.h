// editor_manager.h
//

#pragma once

//
// EditorManager
//
class EditorManager
{
public:
	void		SetIsEditorMode(bool isEditorMode) 
	{
		this->isEditorMode = isEditorMode;
	}

	bool		IsEditorModeEnabled()
	{
		return isEditorMode;
	}
private:
	bool				isEditorMode;
};

extern EditorManager editorManager;