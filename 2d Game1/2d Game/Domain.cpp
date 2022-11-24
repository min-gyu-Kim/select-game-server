#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Util/List.h"
#include "Domain.h"

BOOL DomainToIP(LPCWSTR szDomain, IN_ADDR* pAddr, BOOL bFlushDnsCashe)
{
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;

	if (bFlushDnsCashe == TRUE)
	{
		system("ipconfig /flushdns");
		system("cls");
	}

	if (GetAddrInfo(szDomain, L"0", nullptr, &pAddrInfo) != 0)
	{
		return FALSE;
	}

	pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;
	FreeAddrInfoW(pAddrInfo);

	return TRUE;
}

BOOL GetDomainToIPList(LPCWSTR szDomain, List<IN_ADDR>& list_addr)
{
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;

	if (GetAddrInfo(szDomain, L"0", nullptr, &pAddrInfo) != 0)
	{
		return FALSE;
	}

	ADDRINFOW* pCurAddrInfo = pAddrInfo;
	while (pCurAddrInfo != nullptr)
	{
		pSockAddr = (SOCKADDR_IN*)pCurAddrInfo->ai_addr;
		list_addr.push_back(pSockAddr->sin_addr);

		pCurAddrInfo = pCurAddrInfo->ai_next;
	}

	FreeAddrInfoW(pAddrInfo);

	return TRUE;
}
