#include "stdafx.h"
#include "pch.h"
#include "ARPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CARPLayer::CARPLayer(char* pName)
    : CBaseLayer(pName)
{
    ResetHeader();
}

CARPLayer::~CARPLayer()
{

}


void CARPLayer::ResetHeader()
{
    // 이더넷 목적지 주소, 나의 주소, 타입, Data를 초기화함
    arpHeader.hard_type = 1;
    arpHeader.prot_type = 0x0800;
    arpHeader.mac_len = 6;
    arpHeader.ip_len = 4;
    arpHeader.op_code = 0;
    memset(arpHeader.source_mac, 6, 0);
    memset(arpHeader.source_ip, 4, 0);
    memset(arpHeader.target_mac, 6, 0);
    memset(arpHeader.target_ip, 4, 0);
}

void CARPLayer::SetSenderInfo(const unsigned char* macAddress, const unsigned char* ipAddress) {
    memcpy(sender_mac, macAddress, 6);    // MAC 주소 설정
    memcpy(sender_ip, ipAddress, 4);      // 나의 IP 주소 설정
}

void CARPLayer::SetTargetInfo(const unsigned char* targetIp) {
    memcpy(target_ip, targetIp, 4);       // 타겟 IP 주소 설정
}

BOOL CARPLayer::SetEthernetDest(unsigned char* target_mac) {
    const unsigned char broadcast_mac[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    if (arpHeader.op_code == 1) {  // ARP request
        ((CEthernetLayer*)GetUnderLayer())->SetDestinAddress((unsigned char*)broadcast_mac);
    }
    else if (arpHeader.op_code == 2) {  // ARP reply
        ((CEthernetLayer*)GetUnderLayer())->SetDestinAddress(target_mac);
    }
    return true;
}

void CARPLayer::createRequestPacket() {
    // 선택된 IP주소에 해당하는 mac주소가 있으면 전송 X
    // 없으면 브로드캐스트로 전송
    // mac주소가 0이면 없을수 밖에 없기때문에 의미없다
    unsigned char defaultMac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    if (addOrPresent(target_ip, defaultMac, false, false)) {
        ResetHeader();
        memcpy(arpHeader.source_mac, sender_mac, 6);
        memcpy(arpHeader.source_ip, sender_ip, 4);
        memcpy(arpHeader.target_ip, target_ip, 4);

        createPacket(1);
    }
}

void CARPLayer::createReplyPacket(unsigned char* payload_data) {
    // 송신측의 맥주소와 IP주소를 타겟으로 바꾸고 송신측엔 나의 정보로 채움
    // target ip 주소가 나인지 확인
    // 내가 맞을 경우 source 필드를 target 필드로 설정하고
    // source 필드에 나의 ip주소 mac주소 설정
    // op code 2로 send에 전달
    PARP_HEADER data = (PARP_HEADER)payload_data;
    ResetHeader();

    // Target 필드 설정: Source 필드를 Target 필드로 설정
    memcpy(arpHeader.target_mac, data->source_mac, data->mac_len);  // source_mac -> target_mac
    memcpy(arpHeader.target_ip, data->source_ip, data->ip_len);   // source_ip -> target_ip

    // Source 필드 설정: 
    memcpy(arpHeader.source_mac, sender_mac, data->mac_len);
    memcpy(arpHeader.source_ip, sender_ip, data->ip_len);

    createPacket(2);
};

void CARPLayer::createPacket(unsigned short op_code) {
    arpHeader.op_code = op_code;
    SetEthernetDest(arpHeader.target_mac);
    Send((unsigned char*)&arpHeader, ARP_HEADER_SIZE);
};

BOOL CARPLayer::Send(unsigned char* ppayload, int nlength)
{
    BOOL success = ((CEthernetLayer*)(this->GetUnderLayer()))->Send(ppayload, ARP_HEADER_SIZE, ARP_LAYER_IDENTIFIER);  // ARP 패킷 타입 0x0806

    if (success) {
        // target_ip, target_mac, incomplete으로 테이블에 추가
        //addOrPresent(arpHeader.target_ip, arpHeader.target_mac, false, false);
        addOrPresent(arpHeader.target_ip, 0, false, false);
    }
    else {
        AfxMessageBox(_T("패킷 전송 실패 - ARP Send"));
    }

    return success;
}


BOOL CARPLayer::Receive(unsigned char* payload_data)
{
    PARP_HEADER data = (PARP_HEADER)payload_data;

    //받은 ARP OP code가 1 - ARP 응답 패킷 생성 함수 호출
    if (data->op_code == 1) {
        // sender의 mac주소와 ip주소 전달
        if (data->target_ip == sender_ip)
            addOrPresent(data->source_ip, data->source_mac, true, true); // 질문 중복이면?
            createReplyPacket(payload_data);
    }
    //받은 ARP OP code가 2 - ARP cashe table 업데이트 
    else if (data->op_code == 2) {
        // 캐시 테이블 업데이트
        handleArpReply(data->source_ip);
    };

    return true;
}

void CARPLayer::onEntryTimeout(const unsigned char* ip) {
    std::string strIP = binaryToString(ip);
    //std::cout << "Entry with IP " << strIP << " has timed out. Removing from cache." << std::endl;
    removeEntry(ip);
    //dlg에 엔트리 제거 후 테이블 업데이트
}
