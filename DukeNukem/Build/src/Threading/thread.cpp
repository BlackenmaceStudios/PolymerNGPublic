#include "thread.h"

BuildThread::BuildThread(int core)
{
	DWORD   dwThreadIdArray;
	_sysHandle = ::CreateThread( NULL, 0,MyThreadFunction, this, 0, &dwThreadIdArray);
//	SetThreadAffinityMask(_sysHandle, core);
}

DWORD WINAPI BuildThread::MyThreadFunction(LPVOID lpParam)
{
	BuildThread *thread = (BuildThread *)lpParam;

	return thread->Execute();
}
