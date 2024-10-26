
// ipc2023Dlg.h: 헤더 파일
//

#pragma once

#include "LayerManager.h"	// Added by ClassView
//#include "ChatAppLayer.h"	// Added by ClassView
#include "EthernetLayer.h"	// Added by ClassView
//#include "FileLayer.h"	// Added by ClassView
#include "NILayer.h"
#include "ARPLayer.h"
//#include "FileAppLayer.h"
// Cipc2023Dlg 대화 상자
class Cipc2023Dlg : public CDialogEx, public CBaseLayer
{
// 생성입니다.
public:
	Cipc2023Dlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.



// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPC2023_DIALOG };
#endif

	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
//	UINT m_unDstAddr;
//	UINT unSrcAddr;
//	CString m_stMessage;
//	CListBox m_ListChat;
	
	//afx_msg void OnTimer(UINT nIDEvent);


public:
	BOOL			Receive(unsigned char* ppayload);
	void UpdateListCtrlItem(int row, int col, const CString& newValue); // 캐시 테이블 변경

private:
	CLayerManager	m_LayerMgr;
	int				m_nAckReady;

	enum {
		IPC_INITIALIZING,
		IPC_READYTOSEND,
		IPC_WAITFORACK,
		IPC_ERROR,
		IPC_BROADCASTMODE,
		IPC_UNICASTMODE,
		IPC_ADDR_SET,
		IPC_ADDR_RESET,
		IPC_COMBO_SET
	};

	void			SetDlgState(int state);
	inline void		EndofProcess();
	void Str2UCHAR(CString& src, UCHAR* dst);
	void UCHAR2Str(UCHAR* src, CString& dst);

	BOOL			m_bSendReady;

	// Implementation
	UINT			m_wParam;
	DWORD			m_lParam;

	// Object Layer
	CNILayer* m_NI;
	CEthernetLayer* m_Eth;
	CARPLayer* m_ARP;

public:
	CComboBox m_comboBox;
	CString m_unSrcMac;
	CString m_unDstMac;
	UCHAR m_ucSrcAddrArray[6];
	UCHAR m_ucDstAddrArray[6];
	UCHAR m_unused[100];
	afx_msg void OnCbnSelchangeCombo(); // 어댑터 선택
	int m_index;
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonDeleteAll();
	afx_msg void OnBnClickedButtonIpSend(); 
	afx_msg void OnBnClickedButtonSelect();
	CListCtrl m_ListCtrl;		// ARP 캐시 테이블
	CIPAddressCtrl m_ipSource;	// 내 IP 주소
	CIPAddressCtrl m_ipTarget;	// 상대 IP 주소
};
