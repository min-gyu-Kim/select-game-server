#pragma once

BOOL DomainToIP(LPCWSTR szDomain, IN_ADDR* pAddr, BOOL bFlushDnsCashe = FALSE);

BOOL GetDomainToIPList(LPCWSTR szDomain, List<IN_ADDR>& addr_list);