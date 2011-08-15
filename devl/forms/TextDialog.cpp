// TextDialog.cpp : implementation file
//

#include "stdafx.h"
#include "forms.h"
#include "TextDialog.h"
#include <iostream>

// TextDialog dialog

IMPLEMENT_DYNAMIC(TextDialog, CDialog)

TextDialog::TextDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TextDialog::IDD, pParent)
   , m_strFirstName(_T(""))
   , m_strTitle(_T(""))
   , m_strFullName(_T(""))
{

}

TextDialog::~TextDialog()
{
}

void TextDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, FirstName, m_strFirstName);
   DDX_CBString(pDX, IDC_COMBO1, m_strTitle);
   DDX_Control(pDX, IDC_LIST1, m_NameList);
   DDX_LBString(pDX, IDC_LIST1, m_strFullName);
}


BEGIN_MESSAGE_MAP(TextDialog, CDialog)
   ON_EN_CHANGE(FirstName, &TextDialog::OnEnChangeFirstname)
   ON_LBN_SELCHANGE(IDC_LIST1, &TextDialog::OnLbnSelchangeList1)
   ON_BN_CLICKED(IDOK, &TextDialog::OnBnClickedOk)
   ON_BN_CLICKED(IDCANCEL, &TextDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// TextDialog message handlers

void TextDialog::OnEnChangeFirstname()
{
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CDialog::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.

   // TODO:  Add your control notification handler code here
}

void TextDialog::OnLbnSelchangeList1()
{
   // TODO: Add your control notification handler code here
}

void TextDialog::OnBnClickedOk()
{
   //exit(1);
   // TODO: Add your control notification handler code here
   std::cout << "OK" << std::endl;
   //OnOK();

   CString strTitle; 
   int nIndex; 
   UpdateData();

   nIndex = GetDlgItemText(IDC_TITLE, strTitle); 
   m_strFullName = strTitle + " " + m_strFirstName; 
   m_NameList.AddString(m_strFullName); 
   UpdateData(FALSE);
}

void TextDialog::OnBnClickedCancel()
{
  // exit(1);
   // TODO: Add your control notification handler code here
   //OnCancel();
}