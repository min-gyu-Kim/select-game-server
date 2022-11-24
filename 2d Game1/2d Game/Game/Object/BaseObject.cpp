#include "framework.h"
#include "BaseObject.h"

BaseObject::BaseObject() :
	_end_frame(false),
	_sprite_start(-1),
	_sprite_end(0),
	_sprite_now(0)
{
}

BaseObject::~BaseObject()
{
}

void BaseObject::SetSprite(int sprite_start, int sprite_end, int frame_delay)
{
	_sprite_start = sprite_start;
	_sprite_end = sprite_end;
	_sprite_now = sprite_start;

	_frame_delay = frame_delay;
	_delay_count = 0;
	_end_frame = false;
}

void BaseObject::NextFrame()
{
	if (_sprite_start < 0)
	{
		return;
	}

	_delay_count++;

	if (_delay_count >= _frame_delay)
	{
		_delay_count = 0;
		_sprite_now++;

		if (_sprite_now > _sprite_end)
		{
			_sprite_now = _sprite_start;
			_end_frame = true;
		}
	}
}

void BaseObject::ActionInput(DWORD action_type)
{
	_action_input = action_type;
}
