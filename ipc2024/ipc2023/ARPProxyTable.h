#pragma once

#include "pch.h"
#include <map>
#include <array>
#include <string>
#include <afxwin.h>
#include <sstream>

struct ProxyEntry {
    CString deviceName;
    unsigned char MACAddress[6]; // MAC 주소
};

class ARPProxyTable
{
public:
	ARPProxyTable();
	~ARPProxyTable();

    // 항목 추가
    void AddEntry(const CString& deviceName, const unsigned char ipAddress[4], const unsigned char macAddress[6]);

    // 모든 항목 삭제
    void ClearTable();

    // IP 주소로 항목 검색
    ProxyEntry* FindEntryByIP(const unsigned char* ipAddress);

    // 캐시 테이블의 모든 항목 반환
    const std::map<std::string, ProxyEntry>& GetAllEntries() const;

    // 전달한 ip 주소에 해당하는 항목 삭제
    void ARPProxyTable::RemoveEntryByIP(const unsigned char ipAddress[4]);

    // 모든 항목 메시지 박스로 띄움
    void DisplayAllEntries() const;

private:
    std::map<std::string, ProxyEntry> m_entries;  // Proxy ARP 테이블을 위한 맵

    // IP 주소를 문자열로 변환하는 함수
    std::string IpToString(const unsigned char ipAddress[4]) const;
};