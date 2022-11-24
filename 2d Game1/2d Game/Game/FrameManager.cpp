#include "frameWork.h"
#include "FrameManager.h"

FrameManager FrameManager::s_instance;

bool FrameManager::Initialize()
{
	_prev_time = timeGetTime();
	_timer = 0;
	_insufficient_time = 0;

	_update_count = 0;
	_render_count = 0;
	_update_frame = 0;
	_render_frame = 0;

	return true;
}

bool FrameManager::Update()
{
	DWORD cur_time = timeGetTime();
	int elapse_time = cur_time - _prev_time;
	_prev_time = cur_time;
	
	_timer += elapse_time;
	_insufficient_time += elapse_time - FRAME_TIME;

	int sleep_time = 0;
	if (_insufficient_time < 0)
	{
		sleep_time = FRAME_TIME + _insufficient_time;
		if (sleep_time <= 0)
		{
			sleep_time = 19;
		}
		Sleep(sleep_time);
	}

	//printf("%d %d\n", elapse_time, sleep_time);
	
	if (_timer >= 1000)
	{
		_update_frame = _update_count;
		_render_frame = _render_count;

		_update_count = 0;
		_render_count = 0;

		_timer -= 1000;
	}

	if (_insufficient_time >= (int)FRAME_TIME)
	{
		return false;
	}
	else
	{
		return true;
	}	
}

FrameManager::FrameManager()
{
}

FrameManager::~FrameManager()
{
}
