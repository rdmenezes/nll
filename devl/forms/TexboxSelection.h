#pragma once

#include "resource"
#include "afxwin.h"

// TexboxSelection dialog

class TexboxSelection : public CDialog
{
	DECLARE_DYNAMIC(TexboxSelection)

public:
	TexboxSelection(CWnd* pParent = NULL);   // standard constructor
	virtual ~TexboxSelection();
   int DoModal(const std::string& title, const std::vector<std::string>& texts);
   BOOL OnInitDialog();

// Dialog Data
	enum { IDD = TEXTBOX_SELECTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
   void OnOK();

private:
   CString     _title;
   CListBox    _listbox;
   std::vector<std::string>   _textToDisplay;
};
