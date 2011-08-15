#pragma once

#include "resource"

// TextDialog dialog

class TextDialog : public CDialog
{
	DECLARE_DYNAMIC(TextDialog)

public:
	TextDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedOk();
   CString _textBox;
};
