// ProxyDialog.cpp: 구현 파일
//

#include "pch.h"
#include "ipc2023.h"
#include "afxdialogex.h"
#include "ProxyDialog.h"


// ProxyDialog 대화 상자

IMPLEMENT_DYNAMIC(ProxyDialog, CDialogEx)

ProxyDialog::ProxyDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

ProxyDialog::~ProxyDialog()
{
}

void ProxyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ProxyDialog, CDialogEx)

	ON_BN_CLICKED(IDOK, &ProxyDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &ProxyDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// ProxyDialog 메시지 처리기


void ProxyDialog::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void ProxyDialog::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
