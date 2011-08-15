// TextDialog.cpp : implementation file
//

#include "stdafx.h"
#include "forms.h"
#include "TextDialog.h"


// TextDialog dialog

IMPLEMENT_DYNAMIC(TextDialog, CDialog)

TextDialog::TextDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TextDialog::IDD, pParent)
   , _textBox(_T(""))
{

}

TextDialog::~TextDialog()
{
}

void TextDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_EDIT1, _textBox);
}


BEGIN_MESSAGE_MAP(TextDialog, CDialog)
   ON_BN_CLICKED(IDOK, &TextDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// TextDialog message handlers

void TextDialog::OnBnClickedOk()
{
   // TODO: Add your control notification handler code here
   OnOK();
}
