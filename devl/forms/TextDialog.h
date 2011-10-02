#pragma once

#include "resource"
#include "afxwin.h"

// TextDialog dialog

class TextDialog : public CDialog
{
	DECLARE_DYNAMIC(TextDialog)

public:
	TextDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextDialog();

   std::string getString() const;
   int DoModal(const std::string& title);
   BOOL OnInitDialog();

// Dialog Data
	enum { IDD = TEXTBOX1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
   CEdit _editControl;
   afx_msg void OnBnClickedButton1();
   void OnOK();

private:
   std::string _msg;
   CString     _title;
};