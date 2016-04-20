#include "pch.h"
#include <cstdio>
#include "mdump.h"
#include <tchar.h>
LPCSTR MiniDumper::m_szAppName;

MiniDumper g_dumper("eduke32");

MiniDumper::MiniDumper( LPCSTR szAppName )
{

}

LONG MiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	return 0;
}