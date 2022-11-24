#pragma once

class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();

protected:
	POINT	_position;
	DWORD	_action_input;

	bool	_end_frame;
	int		_sprite_start;
	int		_sprite_end;
	int		_sprite_now;
	int		_delay_count;		//카운터
	int		_frame_delay;		//목표치

	int		_object_id;
	eObjectType _object_type;

public:
	void SetPosition(const POINT& position) { _position.x = position.x; _position.y = position.y; }
	POINT GetPosition() const { return _position; }

	void SetSprite(int sprite_start, int sprite_end, int frame_delay);
	void NextFrame();

	void ActionInput(DWORD action_type);

	virtual void Update() = 0;
	virtual void Render(BYTE* screen, size_t width, size_t height, size_t pitch) = 0;

	bool IsEndFrame() { return _end_frame; }
	int GetCurFrame() const { return _sprite_now - _sprite_start; }
	eObjectType GetType() const { return _object_type; }
	int GetID() const { return _object_id; }
	void SetID(DWORD id) { _object_id = id; }
};

