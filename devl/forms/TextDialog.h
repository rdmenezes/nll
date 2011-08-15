#pragma once
#include "resource.h"
#include "afxwin.h"


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
   afx_msg void OnEnChangeFirstname();
   CString m_strFirstName;
   CString m_strTitle;
   afx_msg void OnLbnSelchangeList1();
   CListBox m_NameList;
   afx_msg void OnBnClickedOk();
   CString m_strFullName;
   afx_msg void OnBnClickedCancel();
   afx_msg void OnBnClickedButton2();
};
