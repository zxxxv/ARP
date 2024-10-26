// ipc2023Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ipc2023.h"
#include "ipc2023Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX); // 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cipc2023Dlg 대화 상자



Cipc2023Dlg::Cipc2023Dlg(CWnd* pParent /*=nullptr*/) // Cipc2023Dlg의 생성자 구현
	: CDialogEx(IDD_IPC2023_DIALOG, pParent)
	, CBaseLayer("ChatDlg") // CBaseLayer의 생성자를 호출하여 ChatDlg라는 레이어를 생성한다.
	, m_bSendReady(FALSE)
	, m_nAckReady(-1)
	, m_stMessage(_T(""))
	, m_index(0)
{
	//대화상자 멤버 변수 초기화
	//  m_unDstAddr = 0;
	//  unSrcAddr = 0;
	//  m_stMessage = _T("");
	//대화 상자 멤버 초기화 완료

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_unSrcAddr = _T("");
	m_unDstAddr = _T("");
	m_stFilePath = _T("");

	//Protocol Layer Setting
	m_LayerMgr.AddLayer(new CChatAppLayer("ChatApp"));
	m_LayerMgr.AddLayer(new CEthernetLayer("Ethernet"));
	m_LayerMgr.AddLayer(new CNILayer("NI"));
	m_LayerMgr.AddLayer(new CFileAppLayer("FileApp"));
	m_LayerMgr.AddLayer(this);

	// 레이어를 연결한다. (레이어 생성)
	m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *ChatApp ( *ChatDlg ) *FileApp (*ChatDlg) ) ) )");

	m_ChatApp = (CChatAppLayer*)m_LayerMgr.GetLayer("ChatApp");
	m_Eth = (CEthernetLayer*)m_LayerMgr.GetLayer("Ethernet");
	m_NI = (CNILayer*)m_LayerMgr.GetLayer("NI");
	m_File = (CFileAppLayer*)m_LayerMgr.GetLayer("FileApp");
	//Protocol Layer Setting
}

void Cipc2023Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO4, m_comboBox);
	DDX_Text(pDX, IDC_EDIT_SRC, m_unSrcAddr); // IDC_EDIT_SRC와 IDC_EDIT1는 같다.
	DDX_Text(pDX, IDC_EDIT_DST, m_unDstAddr); // IDC_EDIT_DST와 IDC_EDIT2는 같다.
	DDX_Text(pDX, IDC_EDIT_MSG, m_stMessage); // IDC_EDIT_MSG와 IDC_EDIT3는 같다. 메시지 입력창
	DDX_Text(pDX, IDC_EDIT_FILE, m_stFilePath); // 파일 경로 띄우기
	DDX_Control(pDX, IDC_LIST_CHAT, m_ListChat); // IDC_LIST_CHAT와 IDC_LIST1는 같다. 메시지 올라오는 창
	DDX_Control(pDX, IDC_PROGRESS, m_progressCtrl);
}

// 레지스트리에 등록하기 위한 변수
UINT nRegSendMsg;
UINT nRegAckMsg;
// 레지스트리에 등록하기 위한 변수


BEGIN_MESSAGE_MAP(Cipc2023Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADDR, &Cipc2023Dlg::OnBnClickedButtonAddr)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &Cipc2023Dlg::OnBnClickedButtonSend)
	ON_WM_TIMER()

	ON_REGISTERED_MESSAGE(nRegSendMsg, OnRegSendMsg)
	//////////////////////// fill the blank ///////////////////////////////
		// Ack 레지스터 등록
	ON_REGISTERED_MESSAGE(nRegAckMsg, OnRegAckMsg)
	///////////////////////////////////////////////////////////////////////


	ON_BN_CLICKED(IDC_CHECK_TOALL, &Cipc2023Dlg::OnBnClickedCheckToall)
	ON_CBN_SELCHANGE(IDC_COMBO4, &Cipc2023Dlg::OnCbnSelchangeCombo4)
	ON_BN_CLICKED(IDC_BUTTON2, &Cipc2023Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &Cipc2023Dlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// Cipc2023Dlg 메시지 처리기

BOOL Cipc2023Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	UpdateData(FALSE);
	SetRegstryMessage();
	SetDlgState(IPC_INITIALIZING);
	SetDlgState(IPC_COMBO_SET);

	m_File->SetProgressBar(&m_progressCtrl);

	// 파일 전송 버튼 비활성화
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void Cipc2023Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Cipc2023Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Cipc2023Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cipc2023Dlg::OnBnClickedButtonSend()
{
	UpdateData(TRUE); // 대화상자에 입력된 값들을 해당되는 변수에 저장한다.(IDC_EDIT_MSG에 입력된 내용을 m_stMessage에 저장)

	if (!m_stMessage.IsEmpty())
	{
		SetTimer(1, 2000, NULL); // 타임아웃 타이머를 2초로 설정한다.
		m_nAckReady = 0;

		SendData();
		m_stMessage = ""; // m_stMessage를 빈 문자열로 초기화한다.

		(CEdit*)GetDlgItem(IDC_EDIT3)->SetFocus(); // IDC_EDIT_MSG(=IDC_EDIT3)으로 setFocus

		//////////////////////// fill the blank ///////////////////////////////
				// Send 신호를 브로드캐스트로 알림
		//::SendMessage(HWND_BROADCAST, nRegSendMsg, 0, 0);
		///////////////////////////////////////////////////////////////////////
	}

	UpdateData(FALSE); // 변수들에 있는 값을 연결된 대화상자의 요소에 대입한다.(m_stMessage의 빈 문자열을 IDC_EDIT_MSG에 대입)
}

void Cipc2023Dlg::SetRegstryMessage()
{
	nRegSendMsg = RegisterWindowMessage(_T("Send IPC Message"));
	//////////////////////// fill the blank ///////////////////////////////
		// Ack 레지스트리의 메시지를 설정
	nRegAckMsg = RegisterWindowMessage(_T("Ack IPC Message"));
	///////////////////////////////////////////////////////////////////////
}

void Cipc2023Dlg::SendData()
{
	CString MsgHeader;
	if (m_unDstAddr == (unsigned int)0xff) // Destination 주소가 Broadcast인 경우
		MsgHeader.Format(_T("[%d:BROADCAST] "), m_unSrcAddr); // [SrcAdd:BROADCAST]를 MsgHeader에 대입
	else // Destination 주소가 Broadcast가 아닌 경우
		MsgHeader.Format(_T("[%s | %s] "), (LPCTSTR)m_unSrcAddr, (LPCTSTR)m_unDstAddr); // [SrcAdd:DstAdd]를 MsgHeader에 대입 /@@@/

	m_ListChat.AddString(MsgHeader + m_stMessage);
	// 위에서 저장한 MsgHeader와 OnBnClickedButtonSend() 함수에서 받은 m_stMessage 값을 합쳐서
	// m_ListChat에 String 형태로 추가한다.

	// 입력한 메시지를 파일로 저장
	int nlength = m_stMessage.GetLength(); // nlength에 m_stMessage의 길이, 즉, 입력된 메시지의 길이를 저장한다. 
	unsigned char* ppayload = new unsigned char[nlength + 1]; // nlenghth + 1의 크기를 가진 ppayload 문자열 생성.
	memcpy(ppayload, (unsigned char*)(LPCTSTR)m_stMessage, nlength); // m_stMessage->ppayload로 데이터 복사
	ppayload[nlength] = '\0'; // ppayload의 맨 마지막 index에 \0(null 문자)를 넣어준다.(그래서 nlenghth + 1 해준 것)


	// 보낼 data와 메시지 길이를 Send함수로 넘겨준다.
	//m_ChatApp->Send(ppayload, nlength);
	m_ChatApp->StartChatSendThread(ppayload, nlength);
	// ChatApp 레이어에 메시지를 넘겨준다.
}

BOOL Cipc2023Dlg::Receive(unsigned char* ppayload)
{
	CString message = _T("");
	message.Format(_T("[%s | %s] %s"), (LPCTSTR)m_unDstAddr, (LPCTSTR)m_unSrcAddr, ppayload);

	m_ListChat.AddString((LPCTSTR)message);
	return TRUE;
}

BOOL Cipc2023Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			if (::GetDlgCtrlID(::GetFocus()) == IDC_EDIT3)
				OnBnClickedButtonSend();
			return FALSE;
		case VK_ESCAPE: return FALSE;
		}
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void Cipc2023Dlg::SetDlgState(int state)
{
	UpdateData(TRUE);

	CButton* pChkButton = (CButton*)GetDlgItem(IDC_CHECK1);

	CButton* pSendButton = (CButton*)GetDlgItem(bt_send);
	CButton* pSetAddrButton = (CButton*)GetDlgItem(bt_setting);
	CEdit* pMsgEdit = (CEdit*)GetDlgItem(IDC_EDIT3);
	CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_EDIT1);
	CEdit* pDstEdit = (CEdit*)GetDlgItem(IDC_EDIT2);
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO4);

	switch (state)
	{
	case IPC_INITIALIZING:
		pSendButton->EnableWindow(FALSE);
		pMsgEdit->EnableWindow(FALSE);
		m_ListChat.EnableWindow(FALSE);
		break;
	case IPC_READYTOSEND:
		pSendButton->EnableWindow(TRUE);
		pMsgEdit->EnableWindow(TRUE);
		m_ListChat.EnableWindow(TRUE);
		break;
	case IPC_WAITFORACK:	break;
	case IPC_ERROR:		break;
		/*case IPC_UNICASTMODE:
			m_unDstAddr = 0x0;
			pDstEdit->EnableWindow(TRUE);
			break;
		case IPC_BROADCASTMODE:
			m_unDstAddr = 0xff;
			pDstEdit->EnableWindow(FALSE);
			break;*/
	case IPC_ADDR_SET:
		pSetAddrButton->SetWindowText(_T("재설정(&R)"));
		pSrcEdit->EnableWindow(FALSE);
		pDstEdit->EnableWindow(FALSE);
		pChkButton->EnableWindow(FALSE);
		break;
	case IPC_ADDR_RESET:
		pSetAddrButton->SetWindowText(_T("설정(&O)"));
		pSrcEdit->EnableWindow(TRUE);
		if (!pChkButton->GetCheck())
			pDstEdit->EnableWindow(TRUE);
		pChkButton->EnableWindow(TRUE);
		break;
	case IPC_COMBO_SET:
		for (int i = 0; i < NI_COUNT_NIC; ++i) {
			pcap_if_t* tempAdater = m_NI->GetAdapterObject(i);
			if (!tempAdater) continue;
			pComboBox->AddString(tempAdater->description);
			pComboBox->SetCurSel(0);
		}
	}

	UpdateData(FALSE);
}


void Cipc2023Dlg::EndofProcess()
{
	m_LayerMgr.DeAllocLayer();
}

// Send메시지 레지스트리가 켜졌을 때
LRESULT Cipc2023Dlg::OnRegSendMsg(WPARAM wParam, LPARAM lParam)
{
	//////////////////////// fill the blank ///////////////////////////////
	if (m_nAckReady) {
		// send하지 않았으면 TRUE인 상태(OnBnClickedButtonSend 함수 내 if문이 수행되었다면 m_nAckReady = 0이 수행되었다.)
		// File 레이어에서 상대방이 전송한 메시지가 담긴 파일을 가져옴
		if (m_LayerMgr.GetLayer("File")->Receive())
		{
			// 메시지를 받았다면 Ack 신호를 브로드캐스트로 날린다.
			::SendMessage(HWND_BROADCAST, nRegAckMsg, 0, 0);
		}
	}
	///////////////////////////////////////////////////////////////////////
	return 0;
}

LRESULT Cipc2023Dlg::OnRegAckMsg(WPARAM wParam, LPARAM lParam)
{
	if (!m_nAckReady) { // Ack 신호를 받으면 타이머를 멈춘다.
		m_nAckReady = -1;
		KillTimer(1);
	}

	return 0;
}

//void Cipc2023Dlg::OnTimer(UINT nIDEvent)
//{
//	// TODO: Add your message handler code here and/or call default
//	m_ListChat.AddString(_T(">> The last message was time-out.."));
//	m_nAckReady = -1;
//	KillTimer(1);
//
//	CDialog::OnTimer(nIDEvent);
//}

void Cipc2023Dlg::Str2UCHAR(CString& src, UCHAR* dst)
{
	sscanf_s(src, "%02x:%02x:%02x:%02x:%02x:%02x",
		&dst[0], &dst[1], &dst[2],
		&dst[3], &dst[4], &dst[5]);
}

void Cipc2023Dlg::UCHAR2Str(UCHAR* src, CString& dst)
{
	dst.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"),
		src[0], src[1], src[2],
		src[3], src[4], src[5]);
}


void Cipc2023Dlg::OnBnClickedButtonAddr()
{
	UpdateData(TRUE);

	if (m_unDstAddr.IsEmpty() || // SrcAdd나 DstAdd 둘 중 하나라도 설정되어있지 않다면 오류 메시지를 띄운다.
		m_unSrcAddr.IsEmpty())
	{
		AfxMessageBox(_T("주소를 설정 오류발생",
			"경고"),
			MB_OK | MB_ICONSTOP);

		return;
	}

	if (m_bSendReady) { // 대화상자가 생성될 때, m_bSendReady는 FALSE로 초기화된다.
		SetDlgState(IPC_ADDR_RESET);
		SetDlgState(IPC_INITIALIZING);
		// 주소 재설정 시 파일 전송 버튼 비활성화
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	}
	else {
		Str2UCHAR(m_unSrcAddr, m_ucSrcAddrArray);
		Str2UCHAR(m_unDstAddr, m_ucDstAddrArray);
		m_Eth->SetSourceAddress(m_ucSrcAddrArray); // ChatApp 레이어의 헤더 정보에 SrcAdd 값을 저장
		m_Eth->SetDestinAddress(m_ucDstAddrArray); // ChatApp 레이어의 헤더 정보에 DstAdd 값을 저장

		m_NI->PacketStartDriver();

		SetDlgState(IPC_ADDR_SET);
		SetDlgState(IPC_READYTOSEND);
		// 주소 설정 완료 시 파일 전송 버튼 활성화
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	}

	m_bSendReady = !m_bSendReady; // 초기 m_bSendReady 값을 반전시킨다. 
	// 홀수번 작동할 때, Address 값 설정
	// 짝수번 작동할 때, Dlg 초기화(재설정)
}

void Cipc2023Dlg::OnBnClickedCheckToall()
{
	CButton* pChkButton = (CButton*)GetDlgItem(IDC_CHECK_TOALL);

	if (pChkButton->GetCheck()) {
		SetDlgState(IPC_BROADCASTMODE); // m_unDstAddr = 0xff로 설정
	}
	else {
		SetDlgState(IPC_UNICASTMODE); // m_unDstAddr = 0x0로 설정
	}
}

void Cipc2023Dlg::OnCbnSelchangeCombo4()
{
	UpdateData(TRUE);
	m_index = m_comboBox.GetCurSel();
	m_NI->SetAdapterIndex(m_index);
	pcap_if_t* selectedAdapter = m_NI->GetAdapterObject(m_index);
	CString selectedAdapterAdress = m_NI->GetNICardAddress(selectedAdapter->name);
	m_unSrcAddr = selectedAdapterAdress;
	CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_EDIT_SRC);
	pSrcEdit->SetWindowTextA(m_unSrcAddr);
	UpdateData(FALSE);
}

void Cipc2023Dlg::OnBnClickedButton2()
{
	CFileDialog fileDlg(TRUE);
	if (fileDlg.DoModal() == IDOK) {
		m_stFilePath = fileDlg.GetPathName();
		CEdit* pEditFile = (CEdit*)GetDlgItem(IDC_EDIT_FILE);
		pEditFile->SetWindowText(m_stFilePath);
		m_File->SetFilePath(m_stFilePath); // 파일경로 FileApp의 FilePath에 CSTRING 타입으로 저장

		// 파일 경로와 주소가 모두 설정되었는지 확인
		if (!m_stFilePath.IsEmpty() && !m_unSrcAddr.IsEmpty() && !m_unDstAddr.IsEmpty() && m_bSendReady)
		{
			GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		}
	}
}

// 파일 보내기 버튼
void Cipc2023Dlg::OnBnClickedButton1()
{
	m_File->StartSendThread();
}