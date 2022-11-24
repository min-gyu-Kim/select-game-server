#include "framework.h"
#include "Util/Packet.h"
#include "Util/RingBuffer.h"
#include "Network/Protocol.h"
#include "Network/Network.h"
#include "Contents.h"
#include "Util/Log.h"
#include "Util/Define.h"
#include "Network/PacketProc.h"
#include "Util/Profile.h"

std::unordered_map<Identity, Character*> g_umapCharacters;

static DWORD g_secondCounter;
static BYTE	g_frameCount;
static DWORD g_logicCount;

//DEBUG
extern DWORD g_TPS;
extern DWORD g_sendCnt;

#ifdef __WINDBG__
HDC g_memDC;
HBITMAP g_bitmap;
HPEN	g_whitePen;
#endif

//set변경 고려해보기
std::list<Character*> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

Character* FindCharacter(const Identity& sessionIdentity)
{
	const auto& iter = g_umapCharacters.find(sessionIdentity);
	if (iter == g_umapCharacters.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}

Character* CreateCharacter(Session* pSession)
{
	Character* pCharacter = new Character;
	pCharacter->_action = dfACTION_STAND;
	pCharacter->_direction = dfACTION_MOVE_RR;
	pCharacter->_hp = 100;
	pCharacter->_id = pSession->_id;
	pCharacter->_moveDirection = dfACTION_MOVE_RR;
	pCharacter->_pSession = pSession;
	pCharacter->_x = rand() % (dfRANGE_MOVE_RIGHT);
	pCharacter->_y = rand() % (dfRANGE_MOVE_BOTTOM);
	//pCharacter->_x = 300;
	//pCharacter->_y = 6300;

	pCharacter->_curSectorPos.iX = pCharacter->_x / dfSECTOR_SIZE_X;
	pCharacter->_curSectorPos.iY = pCharacter->_y / dfSECTOR_SIZE_Y;
	pCharacter->_oldSectorPos.iX = pCharacter->_curSectorPos.iX;
	pCharacter->_oldSectorPos.iY = pCharacter->_curSectorPos.iY;

	g_umapCharacters.emplace(pCharacter->_id, pCharacter);
	SectorAddCharacter(pCharacter);

	Packet packet;
	SendCreateMyCharacter(&packet, pCharacter);
	
	//주변 섹터들에게 나 들어왔음을 보냄
	MakePacketCreateOtherCharacter(&packet, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y, pCharacter->_hp);

	SectorAround sectorAround;
	int iCnt;
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &sectorAround);
	for (iCnt = 0; iCnt < sectorAround.iCount; iCnt++)
	{
		SendPacket_SectorOne(sectorAround.Around[iCnt].iX, sectorAround.Around[iCnt].iY, &packet, pCharacter->_pSession);
	}

	//나에게 다른 플레이어 정보들 보내기
	std::list<Character*>::iterator iter;
	for (iCnt = 0; iCnt < sectorAround.iCount; iCnt++)
	{
		std::list<Character*>& sectorList = g_sector[sectorAround.Around[iCnt].iY][sectorAround.Around[iCnt].iX];
		iter = sectorList.begin();
		for (; iter != sectorList.end(); ++iter)
		{
			if (*iter != pCharacter)
			{
				Character* pCharac = *iter;
				MakePacketCreateOtherCharacter(&packet, pCharac->_id, pCharac->_direction, pCharac->_x, pCharac->_y, pCharac->_hp);
				SendUnicast(pCharacter->_pSession, &packet);

				if (pCharac->_action >= dfACTION_MOVE_LL && pCharac->_action <= dfACTION_MOVE_LD)
				{
					MakePacketMoveStart(&packet, pCharac->_id, pCharac->_action, pCharac->_x, pCharac->_y);
					SendUnicast(pCharacter->_pSession, &packet);
				}
			}
		}
	}

	return pCharacter;
}

void DisconnectCharacter(const Identity& sessionIdentity)
{
	Character* pCharacter = FindCharacter(sessionIdentity);
	SectorRemoveCharacter(pCharacter);
	g_umapCharacters.erase(sessionIdentity);
	delete pCharacter;
}

bool CheckMoveCharacter(short x, short y)
{
	if (x < dfRANGE_MOVE_LEFT || x >= dfRANGE_MOVE_RIGHT)
	{
		return false;
	}

	if (y < dfRANGE_MOVE_TOP || y >= dfRANGE_MOVE_BOTTOM)
	{
		return false;
	}
	return true;
}

Character* CheckCollisionAttack1(Character* pCharacter)
{
	short rangeX1, rangeX2;
	short rangeY1, rangeY2;

	if (pCharacter->_direction == dfACTION_MOVE_LL)
	{
		rangeX1 = pCharacter->_x - dfATTACK1_RANGE_X;
		rangeX2 = pCharacter->_x;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK1_RANGE_Y;
	}
	else if (pCharacter->_direction == dfACTION_MOVE_RR)
	{
		rangeX1 = pCharacter->_x;
		rangeX2 = pCharacter->_x + dfATTACK1_RANGE_X;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK1_RANGE_Y;
	}

	SectorAround sectorAround;
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &sectorAround);
	for (int secCnt = 0; secCnt < sectorAround.iCount; secCnt++)
	{
		std::list<Character*>& secList = g_sector[sectorAround.Around[secCnt].iY][sectorAround.Around[secCnt].iX];
		std::list<Character*>::iterator iter = secList.begin();
		for (; iter != secList.end(); ++iter)
		{
			Character* checkCharacter = *iter;
			if (checkCharacter == pCharacter)
				continue;

			short x1;
			short x2;
			short y1 = checkCharacter->_y - 80;
			short y2 = checkCharacter->_y - 60;
			if (checkCharacter->_direction == dfACTION_MOVE_LL)
			{
				x1 = checkCharacter->_x - 20;
				x2 = checkCharacter->_x + 10;
			}
			else if (checkCharacter->_direction == dfACTION_MOVE_RR)
			{
				x1 = checkCharacter->_x - 10;
				x2 = checkCharacter->_x + 20;
			}

			if (x2 < rangeX1 || x1 > rangeX2) continue;
			if (y2 < rangeY1 || y1 > rangeY2) continue;

			return checkCharacter;
		}
	}

	return nullptr;
	/*
	std::unordered_map<Identity, Character*>::iterator iter = g_umapCharacters.begin();
	for (; iter != g_umapCharacters.end(); ++iter)
	{
		Character* checkCharacter = iter->second;
		if (checkCharacter == pCharacter)
			continue;

		short x1;
		short x2;
		short y1 = checkCharacter->_y - 90;
		short y2 = checkCharacter->_y - 90 + dfATTACK1_RANGE_Y;
		if (checkCharacter->_direction == dfACTION_MOVE_LL)
		{
			x1 = checkCharacter->_x - 71;
			x2 = checkCharacter->_x;
		}
		else if (checkCharacter->_direction == dfACTION_MOVE_RR)
		{
			x1 = checkCharacter->_x;
			x2 = checkCharacter->_x + 71;
		}

		if (x2 < rangeX1 || x1 > rangeX2) continue;
		if (y2 < rangeY1 || y1 > rangeY2) continue;

		return checkCharacter;
	}
	return nullptr;*/
}

Character* CheckCollisionAttack2(Character* pCharacter)
{
	short rangeX1, rangeX2;
	short rangeY1, rangeY2;

	if (pCharacter->_direction == dfACTION_MOVE_LL)
	{
		rangeX1 = pCharacter->_x - dfATTACK2_RANGE_X;
		rangeX2 = pCharacter->_x;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK2_RANGE_Y;
	}
	else if (pCharacter->_direction == dfACTION_MOVE_RR)
	{
		rangeX1 = pCharacter->_x;
		rangeX2 = pCharacter->_x + dfATTACK2_RANGE_X;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK2_RANGE_Y;
	}

	SectorAround sectorAround;
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &sectorAround);
	for (int secCnt = 0; secCnt < sectorAround.iCount; secCnt++)
	{
		std::list<Character*>& secList = g_sector[sectorAround.Around[secCnt].iY][sectorAround.Around[secCnt].iX];
		std::list<Character*>::iterator iter = secList.begin();
		for (; iter != secList.end(); ++iter)
		{
			Character* checkCharacter = *iter;
			if (checkCharacter == pCharacter)
				continue;

			short x1;
			short x2;
			short y1 = checkCharacter->_y - 80;
			short y2 = checkCharacter->_y - 60;
			if (checkCharacter->_direction == dfACTION_MOVE_LL)
			{
				x1 = checkCharacter->_x - 20;
				x2 = checkCharacter->_x + 10;
			}
			else if (checkCharacter->_direction == dfACTION_MOVE_RR)
			{
				x1 = checkCharacter->_x - 10;
				x2 = checkCharacter->_x + 20;
			}

			if (x2 < rangeX1 || x1 > rangeX2) continue;
			if (y2 < rangeY1 || y1 > rangeY2) continue;

			return checkCharacter;
		}
	}

	return nullptr;
	/*
	std::unordered_map<Identity, Character*>::iterator iter = g_umapCharacters.begin();
	for (; iter != g_umapCharacters.end(); ++iter)
	{
		Character* checkCharacter = iter->second;
		if (checkCharacter == pCharacter)
			continue;

		short x1;
		short x2;
		short y1 = checkCharacter->_y - 90;
		short y2 = checkCharacter->_y - 90 + dfATTACK2_RANGE_Y;
		if (checkCharacter->_direction == dfACTION_MOVE_LL)
		{
			x1 = checkCharacter->_x - 71;
			x2 = checkCharacter->_x;
		}
		else if (checkCharacter->_direction == dfACTION_MOVE_RR)
		{
			x1 = checkCharacter->_x;
			x2 = checkCharacter->_x + 71;
		}

		if (x2 < rangeX1 || x1 > rangeX2) continue;
		if (y2 < rangeY1 || y1 > rangeY2) continue;

		return checkCharacter;
	}
	return nullptr;*/
}

Character* CheckCollisionAttack3(Character* pCharacter)
{
	short rangeX1, rangeX2;
	short rangeY1, rangeY2;

	if (pCharacter->_direction == dfACTION_MOVE_LL)
	{
		rangeX1 = pCharacter->_x - dfATTACK3_RANGE_X;
		rangeX2 = pCharacter->_x;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK3_RANGE_Y;
	}
	else if (pCharacter->_direction == dfACTION_MOVE_RR)
	{
		rangeX1 = pCharacter->_x;
		rangeX2 = pCharacter->_x + dfATTACK3_RANGE_X;
		rangeY1 = pCharacter->_y - 70;
		rangeY2 = pCharacter->_y - 70 + dfATTACK3_RANGE_Y;
	}

	SectorAround sectorAround;
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &sectorAround);
	for (int secCnt = 0; secCnt < sectorAround.iCount; secCnt++)
	{
		std::list<Character*>& secList = g_sector[sectorAround.Around[secCnt].iY][sectorAround.Around[secCnt].iX];
		std::list<Character*>::iterator iter = secList.begin();
		for (; iter != secList.end(); ++iter)
		{
			Character* checkCharacter = *iter;
			if (checkCharacter == pCharacter)
				continue;

			short x1;
			short x2;
			short y1 = checkCharacter->_y - 80;
			short y2 = checkCharacter->_y - 60;
			if (checkCharacter->_direction == dfACTION_MOVE_LL)
			{
				x1 = checkCharacter->_x - 20;
				x2 = checkCharacter->_x + 10;
			}
			else if (checkCharacter->_direction == dfACTION_MOVE_RR)
			{
				x1 = checkCharacter->_x - 10;
				x2 = checkCharacter->_x + 20;
			}

			if (x2 < rangeX1 || x1 > rangeX2) continue;
			if (y2 < rangeY1 || y1 > rangeY2) continue;

			return checkCharacter;
		}
	}

	return nullptr;
	/*
	std::unordered_map<Identity, Character*>::iterator iter = g_umapCharacters.begin();
	for (; iter != g_umapCharacters.end(); ++iter)
	{
		Character* checkCharacter = iter->second;
		if (checkCharacter == pCharacter)
			continue;

		short x1;
		short x2;
		short y1 = checkCharacter->_y - 90;
		short y2 = checkCharacter->_y - 90 + 10;
		if (checkCharacter->_direction == dfACTION_MOVE_LL)
		{
			x1 = checkCharacter->_x - 71;
			x2 = checkCharacter->_x;
		}
		else if (checkCharacter->_direction == dfACTION_MOVE_RR)
		{
			x1 = checkCharacter->_x;
			x2 = checkCharacter->_x + 71;
		}

		if (x2 < rangeX1 || x1 > rangeX2) continue;
		if (y2 < rangeY1 || y1 > rangeY2) continue;

		return checkCharacter;
	}
	return nullptr;*/
}

void Update()
{
	g_logicCount++;
	static DWORD s_prevTime = timeGetTime();
	static DWORD s_total = 0;

	//DEBUG
	static DWORD s_frameAvr = 0;
	static DWORD s_frameMax = 0;

	DWORD curTime = timeGetTime();
	DWORD elapseTime = curTime - s_prevTime;

	s_prevTime = curTime;
	s_total += elapseTime;
	g_secondCounter += elapseTime;
	if (s_total < 40)
	{
		return;
	}
	g_frameCount++;
	PROFILE();

	s_frameAvr += s_total;
	if (s_frameMax < s_total)
	{
		s_frameMax = s_total;		
	}

	//DEBUG
	static DWORD s_maxSend;
	if (s_maxSend < g_sendCnt)
	{
		s_maxSend = g_sendCnt;
	}
	g_sendCnt = 0;

	if (g_secondCounter >= 1000)
	{
		g_secondCounter -= 1000;
		if (g_frameCount != 25)
		{			
			time_t t = time(NULL);
			struct tm tm;
			localtime_s(&tm, &t);
			_LOG(dfLOG_LEVEL_WARNING, L"[%02d/%02d/%02d %02d:%02d:%02d] Frame: %d, Loop: %d Avr: %dms Max: %dms tps: %d send max: %d", tm.tm_year - 100, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, g_frameCount, g_logicCount, s_frameAvr / g_frameCount, s_frameMax, g_TPS, s_maxSend);
		}

		//SAVE_PROFILE_DATA(L"Profile");
		s_frameAvr = 0;
		s_frameMax = 0;
		g_TPS = 0;
		g_logicCount = 0;
		g_frameCount = 0;

		//DEBUG
		s_maxSend = 0;
	}

	Character* pCharacter = nullptr;

#ifdef __WINDBG__
	SelectObject(g_memDC, GetStockObject(NULL_PEN));
	Rectangle(g_memDC, 0, 0, 1920, 1080);

	SelectObject(g_memDC, g_whitePen);
	Rectangle(g_memDC, 0, 0, 640, 640);
#endif
	{
		s_total -= 40;
		std::unordered_map<Identity, Character*>::iterator iter = g_umapCharacters.begin();
		for (; iter != g_umapCharacters.end();)
		{
			pCharacter = iter->second;
			++iter;

#ifndef _DUMMY_
			if (pCharacter->_hp <= 0)
			{
				_LOG(dfLOG_LEVEL_DEBUG, L"HP = 0!! SessionID: %d", pCharacter->_id);
				DisconnectClient(pCharacter->_pSession->_socket);
				continue;
			}
#endif

			switch (pCharacter->_action)
			{
			case dfPACKET_MOVE_DIR_LL:
				if (CheckMoveCharacter(pCharacter->_x - dfSPEED_PLAYER_X, pCharacter->_y))
				{
					pCharacter->_x -= dfSPEED_PLAYER_X;
				}
				break;
			case dfPACKET_MOVE_DIR_LU:
				if (CheckMoveCharacter(pCharacter->_x - dfSPEED_PLAYER_X, pCharacter->_y - dfSPEED_PLAYER_Y ))
				{
					pCharacter->_x -= dfSPEED_PLAYER_X;
					pCharacter->_y -= dfSPEED_PLAYER_Y;
				}
				break;
			case dfPACKET_MOVE_DIR_UU:
				if (CheckMoveCharacter(pCharacter->_x, pCharacter->_y - dfSPEED_PLAYER_Y ))
				{
					pCharacter->_y -= dfSPEED_PLAYER_Y;
				}
				break;
			case dfPACKET_MOVE_DIR_RU:
				if (CheckMoveCharacter(pCharacter->_x + dfSPEED_PLAYER_X, pCharacter->_y - dfSPEED_PLAYER_Y))
				{
					pCharacter->_x += dfSPEED_PLAYER_X;
					pCharacter->_y -= dfSPEED_PLAYER_Y;
				}
				break;
			case dfPACKET_MOVE_DIR_RR:
				if (CheckMoveCharacter(pCharacter->_x + dfSPEED_PLAYER_X, pCharacter->_y))
				{
					pCharacter->_x += dfSPEED_PLAYER_X;
				}
				break;
			case dfPACKET_MOVE_DIR_RD:
				if (CheckMoveCharacter(pCharacter->_x + dfSPEED_PLAYER_X, pCharacter->_y + dfSPEED_PLAYER_Y))
				{
					pCharacter->_x += dfSPEED_PLAYER_X;
					pCharacter->_y += dfSPEED_PLAYER_Y;
				}
				break;
			case dfPACKET_MOVE_DIR_DD:
				if (CheckMoveCharacter(pCharacter->_x, pCharacter->_y + dfSPEED_PLAYER_Y))
				{
					pCharacter->_y += dfSPEED_PLAYER_Y;
				}
				break;
			case dfPACKET_MOVE_DIR_LD:
				if (CheckMoveCharacter(pCharacter->_x - dfSPEED_PLAYER_X, pCharacter->_y + dfSPEED_PLAYER_Y))
				{
					pCharacter->_x -= dfSPEED_PLAYER_X;
					pCharacter->_y += dfSPEED_PLAYER_Y;
				}
				break;
			default:
				continue;
			}

			if (SectorUpdateCharacter(pCharacter))
			{
				CharacterSectorUpdatePacket(pCharacter);
			}
#ifdef __WINDBG__
			SetPixel(g_memDC, pCharacter->_x / 10, pCharacter->_y / 10, RGB(255, 0, 0));
#endif
		}
	}
}

void SectorAddCharacter(Character* pCharacter)
{
	int iX = pCharacter->_x / dfSECTOR_SIZE_X;
	int iY = pCharacter->_y / dfSECTOR_SIZE_Y;

	g_sector[iY][iX].emplace_back(pCharacter);
}

void SectorRemoveCharacter(Character* pCharacter)
{
	int iX = pCharacter->_x / dfSECTOR_SIZE_X;
	int iY = pCharacter->_y / dfSECTOR_SIZE_Y;

	std::list<Character*>::iterator iter = g_sector[iY][iX].begin();
	for (; iter != g_sector[iY][iX].end(); ++iter)
	{
		if (*iter == pCharacter)
		{
			g_sector[iY][iX].erase(iter);
			return;
		}
	}
}

void SectorGetAround(int iSectorX, int iSectorY, SectorAround* pSectorAround)
{
	PROFILE();
	int iX, iY;

	iSectorX--;
	iSectorY--;

	pSectorAround->iCount = 0;
	for (iY = 0; iY < 3; iY++)
	{
		if (iY + iSectorY < 0 || iY + iSectorY >= dfSECTOR_MAX_Y)
			continue;

		for (iX = 0; iX < 3; iX++)
		{
			if (iX + iSectorX < 0 || iX + iSectorX >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iY;
			pSectorAround->iCount++;
		}
	}
}

void SectorGetUpdateAround(Character* pCharacter, SectorAround* pRemoveSector, SectorAround* pAddSector)
{
	int cntOld;
	int cntCur;
	bool bFind;
	SectorAround oldSectorAround, curSectorAround;

	oldSectorAround.iCount = 0;
	curSectorAround.iCount = 0;
	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	SectorGetAround(pCharacter->_oldSectorPos.iX, pCharacter->_oldSectorPos.iY, &oldSectorAround);
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &curSectorAround);

	for (cntOld = 0; cntOld < oldSectorAround.iCount; cntOld++)
	{
		bFind = false;
		for (cntCur = 0; cntCur < curSectorAround.iCount; cntCur++)
		{
			if (oldSectorAround.Around[cntOld].iX == curSectorAround.Around[cntCur].iX &&
				oldSectorAround.Around[cntOld].iY == curSectorAround.Around[cntCur].iY)
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			pRemoveSector->Around[pRemoveSector->iCount] = oldSectorAround.Around[cntOld];
			pRemoveSector->iCount++;
		}
	}

	for (cntCur = 0; cntCur < curSectorAround.iCount; cntCur++)
	{
		bFind = false;
		for (cntOld = 0; cntOld < oldSectorAround.iCount; cntOld++)
		{
			if (oldSectorAround.Around[cntOld].iX == curSectorAround.Around[cntCur].iX &&
				oldSectorAround.Around[cntOld].iY == curSectorAround.Around[cntCur].iY)
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			pAddSector->Around[pAddSector->iCount] = curSectorAround.Around[cntCur];
			pAddSector->iCount++;
		}
	}
}

bool SectorUpdateCharacter(Character* pCharacter)
{
	PROFILE();
	pCharacter->_oldSectorPos.iX = pCharacter->_curSectorPos.iX;
	pCharacter->_oldSectorPos.iY = pCharacter->_curSectorPos.iY;

	std::list<Character*>& sectorList = g_sector[pCharacter->_curSectorPos.iY][pCharacter->_curSectorPos.iX];
	std::list<Character*>::iterator iter = sectorList.begin();
	std::list<Character*>::iterator iterEnd = sectorList.end();
	for (;iter != iterEnd; ++iter)
	{
		if (*iter == pCharacter)
		{
			sectorList.erase(iter);
			break;
		}
	}

	pCharacter->_curSectorPos.iX = pCharacter->_x / dfSECTOR_SIZE_X;
	pCharacter->_curSectorPos.iY = pCharacter->_y / dfSECTOR_SIZE_Y;

	g_sector[pCharacter->_curSectorPos.iY][pCharacter->_curSectorPos.iX].emplace_back(pCharacter);

	if (pCharacter->_oldSectorPos.iX == pCharacter->_curSectorPos.iX &&
		pCharacter->_oldSectorPos.iY == pCharacter->_curSectorPos.iY)
		return false;
	else
		return true;
}

void CharacterSectorUpdatePacket(Character* pCharacter)
{
	SectorAround removeSector, addSector;
	Character* pExitCharacter;

	std::list<Character*>* pSectorList;
	std::list<Character*>::iterator iter;
	Packet packet;
	int iCnt;

	SectorGetUpdateAround(pCharacter, &removeSector, &addSector);

	MakePacketDeleteCharacter(&packet, pCharacter->_id);
	for (iCnt = 0; iCnt < removeSector.iCount; iCnt++)
	{
		SendPacket_SectorOne(removeSector.Around[iCnt].iX, removeSector.Around[iCnt].iY, &packet, nullptr);
	}

	for (iCnt = 0; iCnt < removeSector.iCount; iCnt++)
	{
		pSectorList = &g_sector[removeSector.Around[iCnt].iY][removeSector.Around[iCnt].iX];

		iter = pSectorList->begin();
		for (; iter != pSectorList->end(); ++iter)
		{
			MakePacketDeleteCharacter(&packet, (*iter)->_id);
			SendUnicast(pCharacter->_pSession, &packet);
		}
	}

	MakePacketCreateOtherCharacter(&packet, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y, pCharacter->_hp);
	for (iCnt = 0; iCnt < addSector.iCount; iCnt++)
	{
		SendPacket_SectorOne(addSector.Around[iCnt].iX, addSector.Around[iCnt].iY, &packet, nullptr);
	}

	MakePacketMoveStart(&packet, pCharacter->_id, pCharacter->_moveDirection, pCharacter->_x, pCharacter->_y);
	for (iCnt = 0; iCnt < addSector.iCount; iCnt++)
	{
		SendPacket_SectorOne(addSector.Around[iCnt].iX, addSector.Around[iCnt].iY, &packet, nullptr);
	}

	for (iCnt = 0; iCnt < addSector.iCount; iCnt++)
	{
		pSectorList = &g_sector[addSector.Around[iCnt].iY][addSector.Around[iCnt].iX];

		iter = pSectorList->begin();
		for (; iter != pSectorList->end(); ++iter)
		{
			pExitCharacter = *iter;
			if (pExitCharacter != pCharacter)
			{
				MakePacketCreateOtherCharacter(&packet, pExitCharacter->_id, pExitCharacter->_direction, pExitCharacter->_x, pExitCharacter->_y, pExitCharacter->_hp);

				SendUnicast(pCharacter->_pSession, &packet);

				if (pExitCharacter->_action >= dfACTION_MOVE_LL &&
					pExitCharacter->_action <= dfACTION_MOVE_LD)
				{
					MakePacketMoveStart(&packet, pExitCharacter->_id, pExitCharacter->_moveDirection, pExitCharacter->_x, pExitCharacter->_y);
					SendUnicast(pCharacter->_pSession, &packet);
				}
			}
		}
	}
}

void SendPacket_SectorOne(int iSectorX, int iSectorY, Packet* pPacket, Session* pSession)
{
	std::list<Character*>& sectorList = g_sector[iSectorY][iSectorX];
	std::list<Character*>::iterator iter = sectorList.begin();
	for (; iter != sectorList.end(); ++iter)
	{
		if((*iter)->_pSession != pSession)
			SendUnicast((*iter)->_pSession, pPacket);
	}
}

void SendPacket_SectorAround(Packet* pPacket, Session* pSession, bool sendMe)
{
	PROFILE();
	Character* pCharacter = g_umapCharacters.find(pSession->_id)->second;

	SectorAround sectorAround;
	SectorGetAround(pCharacter->_curSectorPos.iX, pCharacter->_curSectorPos.iY, &sectorAround);
	for (int secCnt = 0; secCnt < sectorAround.iCount; secCnt++)
	{
		std::list<Character*>& secList = g_sector[sectorAround.Around[secCnt].iY][sectorAround.Around[secCnt].iX];
		std::list<Character*>::iterator iter = secList.begin();
		for (; iter != secList.end(); ++iter)
		{
			if ((*iter)->_pSession != pSession)
			{
				SendUnicast((*iter)->_pSession, pPacket);
			}
			else if (sendMe)
			{
				SendUnicast(pSession, pPacket);
			}
		}
	}
}
