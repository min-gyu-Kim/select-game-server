#define _PROFILE

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "Profile.h"

static PROFILE_INFO profileInfos[50];
static int count = 0;

//TODO: 호출횟수가 2이하인 경우 처리
//		- 개선

void ProfileBegin(const wchar_t* tagName)
{
	for (int idx = 0; idx < count; idx++)
	{
		if (wcscmp(tagName, profileInfos[idx].tagName) == 0)
		{
			profileInfos[idx].callCount++;
			QueryPerformanceCounter(&profileInfos[idx].startTime);
			return;
		}
	}

	wcscpy_s(profileInfos[count].tagName, tagName);
	profileInfos[count].callCount = 1;
	profileInfos[count].minTime = MAXINT64;
	profileInfos[count].maxTime = MININT64;
	QueryPerformanceCounter(&profileInfos[count++].startTime);
}

void ProfileEnd(const wchar_t* tagName)
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	for (int idx = 0; idx < count; idx++)
	{
		if (wcscmp(tagName, profileInfos[idx].tagName) == 0)
		{
			__int64 elapse = endTime.QuadPart - profileInfos[idx].startTime.QuadPart;

			profileInfos[idx].totalTime += elapse;

			if (elapse < profileInfos[idx].minTime)
			{
				profileInfos[idx].minTime = elapse;
			}
			
			if (elapse > profileInfos[idx].maxTime)
			{
				profileInfos[idx].maxTime = elapse;
			}

			return;
		}
	}
}

void ProfileDataOutText(const wchar_t* fileName)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	wchar_t name[256];
	wcscpy_s(name, fileName);
	wcscat_s(name, L".txt");

	FILE* profileFile = nullptr;
	errno_t result = _wfopen_s(&profileFile, name, L"w");
	if (result == 0)
	{
		fwprintf_s(profileFile, L"%43s|%15s|%15s|%15s|%15s\n", L"Name", L"Average", L"Min", L"Max", L"Call");
		fwprintf_s(profileFile, L"%55s\n", L"------------------------------------------------------------------------------------------------------------------------");

		for (int idx = 0; idx < count; idx++)
		{
			double avg;
			if (profileInfos[idx].callCount <= 2)
			{
				avg = (profileInfos[idx].totalTime / profileInfos[idx].callCount) / (double)freq.QuadPart * 1000.0 * 1000.0;
			}
			else
			{
				avg = ((profileInfos[idx].totalTime - profileInfos[idx].minTime - profileInfos[idx].maxTime) / (profileInfos[idx].callCount - 2)) / (double)freq.QuadPart * 1000.0 * 1000.0;
			}

			fwprintf_s
			(
				profileFile, 
				L"%40s|%13.2lfus|%13.2lfus|%13.2lfus|%15ld\n", 
				profileInfos[idx].tagName, 
				avg, 
				profileInfos[idx].minTime / (double)freq.QuadPart * 1000.0 * 1000.0,
				profileInfos[idx].maxTime / (double)freq.QuadPart * 1000.0 * 1000.0,
				profileInfos[idx].callCount
			);
		}
		fclose(profileFile);
	}
}

void ProfileReset(void)
{
	for (int idx = 0; idx < 50; idx++)
	{
		profileInfos[idx].callCount = 0;
		profileInfos[idx].totalTime = 0;
	}
}

Profile::Profile(const wchar_t* tagName)
{
	wcscpy_s(this->tagName, tagName);
	PRO_BEGIN(this->tagName);
}

Profile::~Profile()
{
	PRO_END(this->tagName);
}
