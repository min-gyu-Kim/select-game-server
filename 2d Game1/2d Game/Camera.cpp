#include "framework.h"
#include "Camera.h"
#include "Game/Object/BaseObject.h"
#include "Game/Object/Player.h"
#include "Types.h"

Camera Camera::s_instance;

Camera::Camera()
{
	_player = nullptr;
}

Camera::~Camera()
{
}

void Camera::Initialize(Player* player)
{
	_player = player;
}

void Camera::Update()
{
	if (!_player)
	{
		return;
	}
	POINT playerPos = _player->GetPosition();

	_position.x = playerPos.x - 320;
	_position.y = playerPos.y - 240 + 25;

	if (_position.x < 0)
	{
		_position.x = 0;
	}
	if (_position.y < 0)
	{
		_position.y = 0;
	}
	if (_position.x > dfCAMERA_END_X)
	{
		_position.x = dfCAMERA_END_X;
	}
	if (_position.y > dfCAMERA_END_Y)
	{
		_position.y = dfCAMERA_END_Y;
	}
}
