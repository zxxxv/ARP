#include "stdafx.h"
#include "pch.h"
#include "ChatAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), Head(nullptr), totalLength(0)
{}

CChatAppLayer::~CChatAppLayer()
{}

struct ThreadParam {
	CChatAppLayer* pLayer;
	unsigned char* payload_data;
	int payload_data_len;
};


void CChatAppLayer::Build_frame(unsigned char* payload_data, unsigned short payload_data_len, unsigned char frame_type, unsigned char seq_id) {
	unsigned short length = payload_data_len;

	// 마지막 프레임이거나 단일 프레임인 경우 NULL 문자 추가를 고려
	if (frame_type == 0x00 || frame_type == 0x03) length++;

	m_appHeader.app_type = frame_type;
	m_appHeader.app_length = payload_data_len;  // 전송할 데이터의 실제 길이 설정
	m_appHeader.app_seq = (unsigned char)seq_id;

	// 정확한 데이터 길이를 복사
	unsigned short copy_len = length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : payload_data_len;
	memcpy(m_appHeader.app_data, payload_data + (seq_id * CHAR_DATA_MAX_SIZE), copy_len);

	((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&m_appHeader, CHAT_HEADER_SIZE + copy_len, CHAT_LAYER_IDENTIFIER);
}

BOOL CChatAppLayer::Send(unsigned char* payload_data, int payload_data_len)
{

	// 메시지 길이가 1496bytes 미만인 경우
	if (payload_data_len < CHAR_DATA_MAX_SIZE) {
		// 단편화 x 프레임 -> 0x00 으로 표시 (단편화를 하지 않으므로 프레임 순서도 0으로 표시)
		Build_frame(payload_data, payload_data_len, 0x00, 0);
		return TRUE;
	}
	// 메시지 길이가 1496bytes를 초과한 경우
	else {
		int length = payload_data_len - CHAR_DATA_MAX_SIZE;
		int i = 0;

		// 첫 단편화 프레임 (문장 첫 부분) -> 0x01 로 표시
		Build_frame(payload_data, payload_data_len, 0x01, i);
		i++;

		for (; length > CHAR_DATA_MAX_SIZE; i++, length -= CHAR_DATA_MAX_SIZE) {
			// 중간 단편화 프레임 (문장 가운데 부분) -> 0x02 로 표시
			Build_frame(payload_data, CHAR_DATA_MAX_SIZE, 0x02, i);
		}
		// 마지막 단편화 프레임 (문장 마지막 부분) -> 0x03 으로 표시
		Build_frame(payload_data, length, 0x03, i);
		return TRUE;
	}
	return FALSE;
}

UINT CChatAppLayer::ChatSendThread(LPVOID pParam) {
	/* send 함수를 쓰레드로 돌리기 위해 존재하는 함수이다.
	   # parameter : ThreadParam 구조체를 사용
	*/
	ThreadParam* param = (ThreadParam*)pParam;
	if (param == nullptr || param->pLayer == nullptr) {
		return 1; // 파라미터가 null인 경우 오류 반환
	}

	try {
		param->pLayer->Send(param->payload_data, param->payload_data_len);
	}
	catch (const std::exception& e) {
		std::cerr << "Error in ChatSendThread: " << e.what() << std::endl;
	}

	// 메모리 해제
	delete[] param->payload_data;
	delete param;

	return 0;
}

BOOL CChatAppLayer::StartChatSendThread(unsigned char* payloadData, int payloadDataLength) {
	ThreadParam* param = new ThreadParam;
	param->pLayer = this;
	param->payload_data = new unsigned char[payloadDataLength];
	memcpy(param->payload_data, payloadData, payloadDataLength);
	param->payload_data_len = payloadDataLength;

	// 스레드를 생성하고, 성공 여부 반환
	return (AfxBeginThread(ChatSendThread, param) != nullptr) ? TRUE : FALSE;
}

void CChatAppLayer::add_frame(unsigned char* frame_data, unsigned char seq_id) {
	Frame_Seq* p = Head;
	Frame_Seq* q = nullptr;

	Frame_Seq* tmp = new Frame_Seq;
	tmp->frame_data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);

	if (tmp->frame_data == nullptr) {
		delete tmp;
		return;
	}

	memcpy(tmp->frame_data, frame_data, CHAR_DATA_MAX_SIZE);
	tmp->seq_id = seq_id;

	while (p != nullptr && (p->seq_id < seq_id)) {
		q = p;
		p = p->next_frame;
	}
	// 처음 들어온 값 앞에 추가, 순서 앞이면 앞에 추가
	if (q == nullptr) {
		tmp->next_frame = Head;
		Head = tmp;
	}
	// 다음 들어온값 순서 뒤일때 뒤에 추가
	else {
		tmp->next_frame = q->next_frame;
		q->next_frame = tmp;
	}
}


BOOL CChatAppLayer::Receive(unsigned char* payload_data) {

	PCHAT_APP_HEADER received_header = (PCHAT_APP_HEADER)payload_data;

	// 단편화되지 않은 경우 (0x00)
	if (received_header->app_type == 0x00) {
		unsigned short size = received_header->app_length;
		unsigned char* GetBuff = new unsigned char[size + 1];  // +1 to ensure null termination
		memcpy(GetBuff, received_header->app_data, size);
		GetBuff[size] = '\0';  // 정확한 위치에 null termination
		this->mp_aUpperLayer[0]->Receive(GetBuff);
		delete[] GetBuff;  // 메모리 해제
		return TRUE;
	}
	else {
		if (received_header->app_type == 0x01) {
			add_frame(received_header->app_data, received_header->app_seq);
			totalLength = received_header->app_length;
			return TRUE;
		}
		else if (received_header->app_type == 0x02) {
			add_frame(received_header->app_data, received_header->app_seq);
			return TRUE;
		}
		else {
			unsigned char* GetBuff = new unsigned char[totalLength + 1];
			int i = 0;
			for (Frame_Seq* _head = Head; _head != nullptr; i++, _head = _head->next_frame) {
				memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), _head->frame_data, CHAR_DATA_MAX_SIZE);
			}

			memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), received_header->app_data, received_header->app_length);
			GetBuff[totalLength] = '\0';  // 마지막에 null termination 
			
			CString msg(GetBuff);  // unsigned char* -> CString 변환
			AfxMessageBox(msg);  // 메세지 박스로 내용 출력
			
			mp_aUpperLayer[0]->Receive(GetBuff);
			deleteList();
			delete[] GetBuff;  // 메모리 해제
			return TRUE;
		}
	}
	return FALSE;
}

void CChatAppLayer::deleteList() {
	Frame_Seq* tmp = Head;
	while (tmp != nullptr) {
		Head = Head->next_frame;
		delete tmp;
		tmp = Head;
	}
	Head = nullptr;
}
