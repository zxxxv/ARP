#ifndef ARPCACHEENTRY_H
#define ARPCACHEENTRY_H

#include <afxwin.h> // MFC 헤더
#include <string>
#include "pch.h"
#include "TimeoutObserver.h"

class ARPCacheEntry : public CWnd {
private:
    unsigned char m_ip[4];
    unsigned char m_mac[6];
    bool m_state;
    UINT m_timerId; // 통합 타이머 ID
    TimeoutObserver* m_observer;

public:
    ARPCacheEntry(const unsigned char* ipAddress, const unsigned char* macAddress, const bool& entryState, TimeoutObserver* obs, const bool& isPermanent);
    /*
    ARPCacheEntry: ARP Entry의 생성자 함수
        # parameter
        - ipAddress: entry의 ip 주소
        - macAddress: entry의 mac주소
        - entryState: entry의 상태
            - true: complete
            - false: incomplete
        - obs: entry의 타임아웃 이벤트를 수신할 클래스(사용하는 클래스에서 this를 사용하면 됨)
        - isPermanent: entry 타이머의 시간을 결정하는 함수
            - true: 20분 설정
            - false: 3분 설정
    */

    ~ARPCacheEntry();

    void setEntry(const unsigned char* ipAddress, const unsigned char* macAddress, const bool& entryState);
    /*
    setEntry: entry에 값을 넣어주는 함수
    */

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    /*
    OnTimer: 타이머가 타임아웃되었을 때 트리거
    # parameter
        - ipAddress: entry의 ip 주소
        - macAddress: entry의 mac주소
        - entryState: entry의 상태
            - true: complete
            - false: incomplete
    */

    void onArpReplyReceived();
    /*
    onArpReplyReceived: ARP 응답 패킷 수신시 작동하는 함수(op code == 2)
    */

    // Getter 메서드들
    std::string getIP();
    std::string getMAC();
    std::string getState();

    DECLARE_MESSAGE_MAP()
};

#endif // ARPCACHEENTRY_H