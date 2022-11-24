#include "Log.h"
#include <stdio.h>
#include <time.h>

int g_iLogLevel = dfLOG_LEVEL_WARNING;//dfLOG_LEVEL_DEBUG;//dfLOG_LEVEL_WARNING;
WCHAR g_szLogBuff[1024];

void Log(WCHAR* szString, int iLogLevel)
{
	wprintf(L"%s \n", szString);
	FILE* pFile;
	fopen_s(&pFile, "log.txt", "at");
	if (!pFile)
		return;
	
	fwprintf(pFile, L"%ls\n", szString);
	fclose(pFile);
}
