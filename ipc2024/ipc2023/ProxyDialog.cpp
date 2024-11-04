#include "pch.h"
#include "ipc2023.h"
#include "afxdialogex.h"
#include "ProxyDialog.h"
#include "ipc2023Dlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// ProxyDialog 대화 상자

IMPLEMENT_DYNAMIC(ProxyDialog, CDialogEx)

ProxyDialog::ProxyDialog(CWnd* pParent /*=nullptr*/, pcap_if_t** adapterList /*=nullptr*/)
    : CDialogEx(IDD_DIALOG1, pParent), m_index(0)
{
    m_pAdapterList = adapterList;

}

BOOL ProxyDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    UpdateData(TRUE); // OnInitDialog에서는 필요하지 않음
    CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBOP);
    if (pComboBox)
    {
        for (int i = 0; i < NI_COUNT_NIC; ++i) {
            pcap_if_t* tempAdapter = m_pAdapterList[i];
            pComboBox->AddString(tempAdapter->description);
            pComboBox->SetCurSel(0); // 첫 번째 항목 선택
            if (!tempAdapter->next) break;
        }
    }
    UpdateData(FALSE);
    return TRUE; // 대화 상자가 포커스를 설정하지 않으면 TRUE를 반환합니다.
}


ProxyDialog::~ProxyDialog()
{
}

void ProxyDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBOP, m_combop); // IDC_COMBOP과 m_combop 연결
}

BEGIN_MESSAGE_MAP(ProxyDialog, CDialogEx)
    ON_BN_CLICKED(IDOK, &ProxyDialog::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &ProxyDialog::OnBnClickedCancel)
    ON_CBN_SELCHANGE(IDC_COMBOP, &ProxyDialog::OnCbnSelchangeCombop)
    ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESSP, &ProxyDialog::OnIpnFieldchangedIpaddressp)
    ON_EN_CHANGE(IDC_EDITP, &ProxyDialog::OnEnChangeEditp)
END_MESSAGE_MAP()

// ProxyDialog 메시지 처리기
//void Cipc2023Dlg::SetDlgState(int state)
//{
//   UpdateData(TRUE);
//
//   CComboBox* pCombop = (CComboBox*)GetDlgItem(IDC_COMBOP);
//
//   switch (state)
//   {
//   case IPC_INITIALIZING:
//      //pSendButton->EnableWindow(FALSE);
//      //pMsgEdit->EnableWindow(FALSE);
//      //m_ListChat.EnableWindow(FALSE);
//      break;
//   case IPC_READYTOSEND:
//      //pSendButton->EnableWindow(TRUE);
//      //pMsgEdit->EnableWindow(TRUE);
//      //m_ListChat.EnableWindow(TRUE);
//      break;
//   case IPC_WAITFORACK:   break;
//   case IPC_ERROR:      break;
//   case IPC_COMBO_SET:
//      for (int i = 0; i < NI_COUNT_NIC; ++i) {
//         pcap_if_t* tempAdater = m_NI->GetAdapterObject(i);
//         if (!tempAdater) continue;
//         pCombop->AddString(tempAdater->description);
//         pCombop->SetCurSel(0);
//      }
//   }
//
//   UpdateData(FALSE);
//}


void ProxyDialog::OnBnClickedOk()
{
    CDialogEx::OnOK();
}

void ProxyDialog::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}

void ProxyDialog::OnCbnSelchangeCombop()
{
    
}

void ProxyDialog::OnIpnFieldchangedIpaddressp(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *pResult = 0;
}

void ProxyDialog::OnEnChangeEditp()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
