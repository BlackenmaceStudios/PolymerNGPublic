#include <windows.h>
#include <xmmintrin.h>
#define SAFE_DELETE_ARRAY(p){if((p)){ delete[](p);    (p)=NULL; }}