// thread.h
//

#pragma once

#include <windows.h>

//
// BuildThread
//
class BuildThread
{
public:
	BuildThread(int core);

	virtual int		Execute() = 0;
private:
	static DWORD WINAPI MyThreadFunction(LPVOID lpParam);

	HANDLE _sysHandle;
};
