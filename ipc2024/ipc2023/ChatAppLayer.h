#pragma once
// ChatAppLayer.h: interface for the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)
#define AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "pch.h"
#include <iostream>

class CChatAppLayer
	: public CBaseLayer
{
private:
	//inline void		ResetHeader();
	CObject* mp_Dlg;
	unsigned short totalLength;

	struct Frame_Seq {
		unsigned char* frame_data;
		unsigned char seq_id;
		Frame_Seq* next_frame;
	};
	Frame_Seq* Head;

	void	add_frame(unsigned char* frame_data, unsigned char seq_id);
	void	Build_frame(unsigned char* payload_data, unsigned short payload_data_len, unsigned char type, unsigned char seq_id);
	void	deleteList();

public:
	BOOL			Receive(unsigned char* payload_data);
	BOOL			Send(unsigned char* payload_data, int payload_data_len);
	// unsigned int	GetDestinAddress();
	// unsigned int	GetSourceAddress();
	// void			SetDestinAddress(unsigned int dst_addr);
	// void			SetSourceAddress(unsigned int src_addr);

	BOOL			StartChatSendThread(unsigned char* payload_data, int payload_data_len);
	static UINT ChatSendThread(LPVOID pParam);

	CChatAppLayer(char* pName);
	virtual ~CChatAppLayer();
	typedef struct CHAT_APP_HEADER {

		// unsigned int	app_dstaddr; // destination address of application layer
		// unsigned int	app_srcaddr; // source address of application layer
		unsigned short	app_length; // total length of the data
		unsigned char	app_type; // type of application data
		unsigned char   app_seq; // 단편화 시 프레임 순서
		unsigned char	app_data[CHAR_DATA_MAX_SIZE]; // application data

	} CHAT_APP_HEADER, * PCHAT_APP_HEADER;


protected:
	CHAT_APP_HEADER		m_appHeader;

	// enum {
	// 	DATA_TYPE_CONT = 0x01,
	// 	DATA_TYPE_END = 0x02
	// };
};

#endif // !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)