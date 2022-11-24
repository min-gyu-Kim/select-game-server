#include "../../framework.h"
#include "BaseObject.h"
#include "Player.h"
#include "Effect.h"

#include "../SpriteDib.h"

Effect::Effect() : BaseObject()
{
	_player = nullptr;
	_attacker = nullptr;
	_object_type = eObjectType::EFFECT;
	SetSprite(65, 68, dfDELAY_EFFECT);
}

Effect::~Effect()
{
}

void Effect::Update()
{
	if (_attacker == nullptr)
		return;

	_position.x = _player->GetPosition().x;
	_position.y = _player->GetPosition().y - 50;

	if (_attacker->GetCurAction() == dfACTION_ATTACK1 &&
		_player->GetCurFrame() >= 0)
	{		
		_isRun = true;
	}
	else if (_attacker->GetCurAction() == dfACTION_ATTACK2 &&
		_attacker->GetCurFrame() >= 0)
	{
		_isRun = true;
	}
	else if (_attacker->GetCurAction() == dfACTION_ATTACK3 &&
		_attacker->GetCurFrame() >= 2)
	{
		_isRun = true;
	}

	if (_isRun)
	{
		NextFrame();

		if (_end_frame == true)
		{
			_end_frame = false;
			_isRun = false;
			_player = nullptr;
			_attacker = nullptr;
		}
	}
}

void Effect::Render(BYTE* screen, size_t width, size_t height, size_t pitch)
{
	if(_isRun)
		SpriteDib::GetInstance()->DrawSprite_redtone(_sprite_now, _position, screen, width, height, pitch);
}
