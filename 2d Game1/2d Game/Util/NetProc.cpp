#include "../framework.h"

//#include "PacketDefine.h"
#include "Protocol.h"
#include "PacketType.h"
#include "../Game/Object/BaseObject.h"
#include "../Game/Object/Effect.h"
#include "../Game/Object/Player.h"
#include "List.h"
#include "NetProc.h"
#include "../Camera.h"

void NetProcCreateMyCharacter(const char* buffer)
{
	stPACKET_SC_CREATE_MY_CHARACTER* packet = (stPACKET_SC_CREATE_MY_CHARACTER*)buffer;

	unsigned char dir;
	if (packet->byDirection == dfPACKET_MOVE_DIR_LL)
	{
		dir = dfDIRECTION_LEFT;
	}
	else if (packet->byDirection == dfPACKET_MOVE_DIR_RR)
	{
		dir = dfDIRECTION_RIGHT;
	}
	Player* player = new Player(true, dir);
	player->SetHP(packet->byHP);
	player->SetPosition(POINT{ packet->shX, packet->shY });
	player->SetID(packet->dwID);

	g_myPlayer = player;
	Camera::GetInstance()->Initialize(player);

	g_playerList.push_back(player);
}

void NetProcCreateOtherCharacter(const char* buffer)
{
	stPACKET_SC_CREATE_OTHER_CHARACTER* packet = (stPACKET_SC_CREATE_OTHER_CHARACTER*)buffer;

	unsigned char dir;
	if (packet->byDirection == dfPACKET_MOVE_DIR_LL)
	{
		dir = dfDIRECTION_LEFT;
	}
	else if (packet->byDirection == dfPACKET_MOVE_DIR_RR)
	{
		dir = dfDIRECTION_RIGHT;
	}
	Player* player = new Player(false, dir);
	player->SetHP(packet->byHP);
	player->SetPosition(POINT{ packet->shX, packet->shY });
	player->SetID(packet->dwID);

	g_playerList.push_back(player);
}

void NetProcDeleteCharacter(const char* buffer)
{
	stPACKET_SC_DELETE_CHARACTER* packet = (stPACKET_SC_DELETE_CHARACTER*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end;)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			delete (*iter);
			g_playerList.erase(iter);
			break;
		}
		else
		{
			++iter;
		}
	}
}

void NetProcMoveStart(const char* buffer)
{
	static int count = 0;
	count++;
	stPACKET_SC_MOVE_START* packet = (stPACKET_SC_MOVE_START*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			(*iter)->SetPosition(POINT{ packet->shX, packet->shY });
			(*iter)->ActionInput(packet->byDirection);

			break;
		}
	}
}

void NetProcMoveStop(const char* buffer)
{
	stPACKET_SC_MOVE_STOP* packet = (stPACKET_SC_MOVE_STOP*)buffer;

	unsigned char dir;
	if (packet->byDirection == dfPACKET_MOVE_DIR_LL)
	{
		dir = dfDIRECTION_LEFT;
	}
	else if (packet->byDirection == dfPACKET_MOVE_DIR_RR)
	{
		dir = dfDIRECTION_RIGHT;
	}

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			(*iter)->SetPosition(POINT{ packet->shX, packet->shY });
			(*iter)->SetDirection(dir);
			(*iter)->ActionInput(-1);

			break;
		}
	}
}

void NetProcAttack1(const char* buffer)
{
	stPACKET_SC_ATTACK1* packet = (stPACKET_SC_ATTACK1*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			(*iter)->SetPosition(POINT{ packet->shX, packet->shY });
			(*iter)->ActionInput(dfACTION_ATTACK1);

			break;
		}
	}
}

void NetProcAttack2(const char* buffer)
{
	stPACKET_SC_ATTACK2* packet = (stPACKET_SC_ATTACK2*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			(*iter)->SetPosition(POINT{ packet->shX, packet->shY });
			(*iter)->ActionInput(dfACTION_ATTACK2);

			break;
		}
	}
}

void NetProcAttack3(const char* buffer)
{
	stPACKET_SC_ATTACK3* packet = (stPACKET_SC_ATTACK3*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwID == (*iter)->GetID())
		{
			(*iter)->SetPosition(POINT{ packet->shX, packet->shY });
			(*iter)->ActionInput(dfACTION_ATTACK3);

			break;
		}
	}
}

void NetProcDamage(const char* buffer)
{
	stPACKET_SC_DAMAGE* packet = (stPACKET_SC_DAMAGE*)buffer;

	List<Player*>::iterator iter = g_playerList.begin();
	List<Player*>::iterator iter_end = g_playerList.end();

	Player* attacker = nullptr;

	int dir;
	POINT position;
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwAttackerID == (*iter)->GetID())
		{
			dir = (*iter)->GetDirection();
			position = (*iter)->GetPosition();
			attacker = (*iter);

			break;
		}
	}

	iter = g_playerList.begin();
	for (; iter != iter_end; ++iter)
	{
		if (packet->dwVictimID == (*iter)->GetID())
		{
			(*iter)->SetHP(packet->byVictimHP);
			if (attacker != nullptr)
			{
				SetEffect(*iter, attacker);
			}

			break;
		}
	}
}

void SetEffect(Player* victimPlayer, Player* attacker)
{

	List<Effect*>::iterator effect_iter = g_effectList.begin();
	List<Effect*>::iterator effect_iter_end = g_effectList.end();

	for (; effect_iter != effect_iter_end; ++effect_iter)
	{
		if ((*effect_iter)->IsRun() == false)
		{
			(*effect_iter)->SetPlayer(victimPlayer, attacker);
			break;
		}
	}	
}

void NetSetMoveStart(PacketHeader* outHeader, stPACKET_CS_MOVE_START* outPayload, unsigned char byDirection, unsigned short x, unsigned short y)
{
	outHeader->byCode = 0x89;
	outHeader->bySize = sizeof(stPACKET_CS_MOVE_START);
	outHeader->byType = dfPACKET_CS_MOVE_START;

	outPayload->byDirection = byDirection;
	outPayload->shX = x;
	outPayload->shY = y;
}

void NetSetMoveStop(PacketHeader* outHeader, stPACKET_CS_MOVE_STOP* outPayload, unsigned byDirection, unsigned short x, unsigned short y)
{
	outHeader->byCode = 0x89;
	outHeader->bySize = sizeof(stPACKET_CS_MOVE_STOP);
	outHeader->byType = dfPACKET_CS_MOVE_STOP;

	outPayload->byDirection = byDirection;
	outPayload->shX = x;
	outPayload->shY = y;
}

void NetSetAttack1(PacketHeader* outHeader, stPACKET_CS_ATTACK1* outPayload, unsigned byDirection, unsigned short x, unsigned short y)
{
	outHeader->byCode = 0x89;
	outHeader->bySize = sizeof(stPACKET_CS_ATTACK1);
	outHeader->byType = dfPACKET_CS_ATTACK1;

	outPayload->byDirection = byDirection;
	outPayload->shX = x;
	outPayload->shY = y;
}

void NetSetAttack2(PacketHeader* outHeader, stPACKET_CS_ATTACK2* outPayload, unsigned byDirection, unsigned short x, unsigned short y)
{
	outHeader->byCode = 0x89;
	outHeader->bySize = sizeof(stPACKET_CS_ATTACK2);
	outHeader->byType = dfPACKET_CS_ATTACK2;

	outPayload->byDirection = byDirection;
	outPayload->shX = x;
	outPayload->shY = y;
}

void NetSetAttack3(PacketHeader* outHeader, stPACKET_CS_ATTACK3* outPayload, unsigned byDirection, unsigned short x, unsigned short y)
{
	outHeader->byCode = 0x89;
	outHeader->bySize = sizeof(stPACKET_CS_ATTACK3);
	outHeader->byType = dfPACKET_CS_ATTACK3;

	outPayload->byDirection = byDirection;
	outPayload->shX = x;
	outPayload->shY = y;
}

