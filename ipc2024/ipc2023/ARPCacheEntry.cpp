// ARPCacheEntry.cpp
#include "ARPCacheEntry.h"
#include "pch.h"

ARPCacheEntry::ARPCacheEntry(const unsigned char* ipAddress, const unsigned char* macAddress, const bool& entryState, TimeoutObserver* obs, const bool& isPermanent)
    : m_observer(obs), m_timerId(0) {
    setEntry(ipAddress, macAddress, entryState);
    if (isPermanent) {
        // MFC 20분 타이머 설정
        m_timerId = SetTimer(0, 20 * 60 * 1000, nullptr); // 20분(1200초) 타이머 설정, ID 자동 생성
    }
    else {
        // MFC 3분 타이머 설정 (임시 엔트리 생성 시)
        m_timerId = SetTimer(0, 3 * 60 * 1000, nullptr); // 3분(180초) 타이머 설정, ID 자동 생성 ***디버깅중 여기서 오류 발생***
    }
}

ARPCacheEntry::~ARPCacheEntry() {
    if (m_timerId != 0) {
        KillTimer(m_timerId); // 타이머 해제
        m_timerId = 0;
    }
}

void ARPCacheEntry::setEntry(const unsigned char* ipAddress, const unsigned char* macAddress, const bool& entryState) {
    memcpy(m_ip, ipAddress, 4);
    memcpy(m_mac, macAddress, 6);
    m_state = entryState;
}

void ARPCacheEntry::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == m_timerId) {
        // 타이머 만료 시, 엔트리가 응답을 받지 않으면 제거
        if (m_observer && m_state == false) {
            // incomplete 상태
            m_observer->onEntryTimeout(m_ip); // 타임아웃 시 옵저버에게 알림
            KillTimer(m_timerId); // 3분 타이머 해제
            m_timerId = 0;
        }
        else if (m_observer) {
            // complete 상태
            m_observer->onEntryTimeout(m_ip); // 타임아웃 시 옵저버에게 알림
            KillTimer(m_timerId); // 타이머 해제
            m_timerId = 0;
        }
    }
}

void ARPCacheEntry::onArpReplyReceived() {
    if (m_timerId != 0) {
        KillTimer(m_timerId); // 기존 타이머 해제 (3분 타이머)
        m_timerId = 0;
    }
    // 상태를 complete로 변경하고 20분 타이머 설정
    m_state = true;
    m_timerId = SetTimer(0, 20 * 60 * 1000, nullptr); // 20분(1200초) 타이머 설정, ID 자동 생성
}

std::string ARPCacheEntry::getIP() {
    return std::to_string(m_ip[0]) + "." +
        std::to_string(m_ip[1]) + "." +
        std::to_string(m_ip[2]) + "." +
        std::to_string(m_ip[3]);
}
std::string ARPCacheEntry::getMAC() {
    return std::to_string(m_mac[0]) + "." +
        std::to_string(m_mac[1]) + "." +
        std::to_string(m_mac[2]) + "." +
        std::to_string(m_mac[3]) + "." +
        std::to_string(m_mac[4]) + "." +
        std::to_string(m_mac[5]);
}
std::string ARPCacheEntry::getState() {
    if (m_state == true) return "complete";
    else return "incomplete";
}

BEGIN_MESSAGE_MAP(ARPCacheEntry, CWnd)
    ON_WM_TIMER()
END_MESSAGE_MAP()