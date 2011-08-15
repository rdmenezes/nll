// TextDialog.cpp : implementation file
//

#include "stdafx.h"
#include "forms.h"
#include "TextDialog.h"
#include "utils.h"
#include <iostream>


// TextDialog dialog

IMPLEMENT_DYNAMIC(TextDialog, CDialog)

TextDialog::TextDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TextDialog::IDD, pParent)
{

}

TextDialog::~TextDialog()
{
}

void TextDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_EDIT1, _editControl);
}


BEGIN_MESSAGE_MAP(TextDialog, CDialog)
   ON_BN_CLICKED(IDC_BUTTON1, &TextDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


void TextDialog::OnOK() 
{
	CString text;
   _editControl.GetWindowTextW( text );
   _msg = mvv::getString( std::wstring((wchar_t*)text.GetString()) );

	CDialog::OnOK();
}

// TextDialog message handlers
void TextDialog::OnBnClickedButton1()
{
   OnOK();
}

std::string TextDialog::getString() const
{
   return _msg;
}