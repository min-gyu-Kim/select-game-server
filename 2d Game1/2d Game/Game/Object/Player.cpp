#include "framework.h"
#include <WS2tcpip.h>
#include "../../Util/RingBuffer.h"
#include "BaseObject.h"
#include "Effect.h"
#include "Player.h"

#include "../SpriteDib.h"
#include "../../Util/List.h"
//#include "../../Util/PacketDefine.h"
#include "../../Util/Protocol.h"
#include "../../Util/PacketType.h"
#include "../../Util/NetProc.h"

Player::Player(bool is_player, unsigned char direction)
	: BaseObject()	
{
	_object_type = eObjectType::PLAYER;
	_is_player = is_player;

	_direction = direction;
	_action_input = -1;
	_cur_action = dfACTION_STAND;
	SetActionStand();
}

Player::~Player()
{
}

void Player::Update()
{
	NextFrame();
	ActionProc();
}

void Player::Render(BYTE* screen, size_t width, size_t height, size_t pitch)
{
	POINT position = _position;

	position.x -= 30;
	position.y += 9;

	//hp
	SpriteDib::GetInstance()->DrawSprite(29, position, screen, width, height, pitch, _hp);

	//그림자
	SpriteDib::GetInstance()->DrawSprite(54, _position, screen, width, height, pitch);

	if (_is_player == true)
	{
		SpriteDib::GetInstance()->DrawSprite_red(_sprite_now, _position, screen, width, height, pitch);
	}
	else
	{
		SpriteDib::GetInstance()->DrawSprite(_sprite_now, _position, screen, width, height, pitch);
	}
}

void Player::ActionProc()
{
	if (_is_player)
	{
		switch (_cur_action)
		{
		case dfACTION_ATTACK1:
		case dfACTION_ATTACK2:
		case dfACTION_ATTACK3:
		{
			if (IsEndFrame() == true)
			{
				SetActionStand();
				_action_input = dfACTION_STAND;
			}
			break;
		}
		default:
			SetAction();
			break;
		}
	}
	else
	{
		switch (_cur_action)
		{
		case dfACTION_ATTACK1:
		case dfACTION_ATTACK2:
		case dfACTION_ATTACK3:
		{
			if (IsEndFrame() == true)
			{
				SetActionStand();
				_action_input = dfACTION_STAND;
			}
			break;
		}
		}
		SetAction();
	}
}

void Player::SetAction()
{
	POINT pt = _position;

	if (_action_input == dfACTION_MOVE_DD ||
		_action_input == dfACTION_MOVE_LD ||
		_action_input == dfACTION_MOVE_RD)
	{
		_position.y += 2;
	}

	if (_action_input == dfACTION_MOVE_UU ||
		_action_input == dfACTION_MOVE_LU ||
		_action_input == dfACTION_MOVE_RU)
	{
		_position.y -= 2;
	}

	if (_action_input == dfACTION_MOVE_LL ||
		_action_input == dfACTION_MOVE_LU ||
		_action_input == dfACTION_MOVE_LD)
	{
		_direction = dfDIRECTION_LEFT;
		_position.x -= 3;
	}

	if (_action_input == dfACTION_MOVE_RR ||
		_action_input == dfACTION_MOVE_RU ||
		_action_input == dfACTION_MOVE_RD)
	{
		_direction = dfDIRECTION_RIGHT;
		_position.x += 3;
	}

	if (_position.x <= dfRANGE_MOVE_LEFT || _position.x >= dfRANGE_MOVE_RIGHT ||
		_position.y <= dfRANGE_MOVE_TOP || _position.y >= dfRANGE_MOVE_BOTTOM)
	{
		_position = pt;
	}

	if (_action_input == -1)
	{
		_action_input = 8;
	}

	if (_action_input == _cur_action)
	{
		return;
	}
	
	if (_cur_action >= 0 && _cur_action <= 7 && (_action_input == dfACTION_STAND || _action_input == dfACTION_ATTACK1 || _action_input == dfACTION_ATTACK2 || _action_input == dfACTION_ATTACK3))
	{
		SendStop();
	}

	_prev_action = _cur_action;
	_cur_action = _action_input;
		
	if (_action_input >= 0 && _action_input <= 7)
	{
		SetActionMove();
	}
	else if (_cur_action == 8)
	{
		SetActionStand();
	}
	else if(_cur_action == dfACTION_ATTACK1)
	{
		if (_direction == dfDIRECTION_LEFT)
		{
			SendAttack1(dfPACKET_MOVE_DIR_LL);
			SetSprite(1, 4, dfDELAY_ATTACK1);
		}
		else
		{
			SendAttack1(dfPACKET_MOVE_DIR_RR);
			SetSprite(5, 8, dfDELAY_ATTACK1);
		}
	}
	else if (_cur_action == dfACTION_ATTACK2)
	{
		if (_direction == dfDIRECTION_LEFT)
		{
			SendAttack2(dfPACKET_MOVE_DIR_LL);
			SetSprite(9, 12, dfDELAY_ATTACK2);
		}
		else
		{
			SendAttack2(dfPACKET_MOVE_DIR_RR);
			SetSprite(13, 16, dfDELAY_ATTACK2);
		}
	}
	else if (_cur_action == dfACTION_ATTACK3)
	{
		if (_direction == dfDIRECTION_LEFT)
		{
			SendAttack3(dfPACKET_MOVE_DIR_LL);
			SetSprite(17, 22, dfDELAY_ATTACK3);
		}
		else
		{
			SendAttack3(dfPACKET_MOVE_DIR_RR);
			SetSprite(23, 28, dfDELAY_ATTACK3);
		}
	}
}

void Player::SetActionStand()
{
	_prev_action = _cur_action;
	_cur_action = dfACTION_STAND;

	if (_direction == dfDIRECTION_LEFT)
	{
		SetSprite(55, 59, dfDELAY_STAND);
	}
	else
	{
		SetSprite(60, 64, dfDELAY_STAND);
	}
}

void Player::SetActionMove()
{
	SendMove();

	if (_direction == dfDIRECTION_LEFT)
	{
		SetSprite(30, 41, dfDELAY_MOVE);
	}
	else
	{
		SetSprite(42, 53, dfDELAY_MOVE);
	}
}

void Player::SendMove()
{
	if (!_is_player)
		return;

	PacketHeader header;
	stPACKET_CS_MOVE_START packet;

	NetSetMoveStart(&header, &packet, _cur_action, _position.x, _position.y);

	int sendRet = g_sendBuf->Enqueue((char*)&header, sizeof(header));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
	sendRet = g_sendBuf->Enqueue((char*)&packet, sizeof(packet));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		closesocket(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
}

void Player::SendStop()
{
	if (!_is_player)
		return;

	PacketHeader header;
	stPACKET_CS_MOVE_STOP packet;

	if(_direction == dfDIRECTION_LEFT)
		NetSetMoveStop(&header, &packet, dfPACKET_MOVE_DIR_LL, _position.x, _position.y);
	else if(_direction == dfDIRECTION_RIGHT)
		NetSetMoveStop(&header, &packet, dfPACKET_MOVE_DIR_RR, _position.x, _position.y);

	int sendRet = g_sendBuf->Enqueue((char*)&header, sizeof(header));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
	sendRet = g_sendBuf->Enqueue((char*)&packet, sizeof(packet));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		closesocket(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
}

void Player::SendAttack1(unsigned char byDir)
{
	if (!_is_player)
		return;

	PacketHeader header;
	stPACKET_CS_ATTACK1 packet;

	NetSetAttack1(&header, &packet, byDir, _position.x, _position.y);

	int sendRet = g_sendBuf->Enqueue((char*)&header, sizeof(header));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
	sendRet = g_sendBuf->Enqueue((char*)&packet, sizeof(packet));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		closesocket(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
}

void Player::SendAttack2(unsigned char byDir)
{
	if (!_is_player)
		return;

	PacketHeader header;
	stPACKET_CS_ATTACK2 packet;

	NetSetAttack2(&header, &packet, byDir, _position.x, _position.y);

	int sendRet = g_sendBuf->Enqueue((char*)&header, sizeof(header));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
	sendRet = g_sendBuf->Enqueue((char*)&packet, sizeof(packet));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		closesocket(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
}

void Player::SendAttack3(unsigned char byDir)
{
	if (!_is_player)
		return;

	PacketHeader header;
	stPACKET_CS_ATTACK3 packet;

	NetSetAttack3(&header, &packet, byDir, _position.x, _position.y);

	int sendRet = g_sendBuf->Enqueue((char*)&header, sizeof(header));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
	sendRet = g_sendBuf->Enqueue((char*)&packet, sizeof(packet));
	if (sendRet < sizeof(header))
	{
		g_isNet = false;
		closesocket(g_sock);
		delete g_recvBuf;
		delete g_sendBuf;

		PostQuitMessage(0);
		MessageBox(nullptr, L"send Queue overflow 연결 종료", L"알림", MB_OK);
	}
}
