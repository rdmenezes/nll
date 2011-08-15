// TexboxSelection.cpp : implementation file
//

#include "stdafx.h"
#include "forms.h"
#include "TexboxSelection.h"
#include "utils.h"
#include <iostream>
#include <afxtempl.h>

// TexboxSelection dialog

IMPLEMENT_DYNAMIC(TexboxSelection, CDialog)

TexboxSelection::TexboxSelection(CWnd* pParent /*=NULL*/)
	: CDialog(TexboxSelection::IDD, pParent)
{

}

TexboxSelection::~TexboxSelection()
{
}

void TexboxSelection::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_LIST1, _listbox);
}

int TexboxSelection::DoModal(const std::string& title, const std::vector<std::string>& texts)
{
  // Initialization
  _title = mvv::getWideString( title ).c_str();
  _textToDisplay = texts;
  return CDialog::DoModal();
}

BOOL TexboxSelection::OnInitDialog()
{
   CDialog::OnInitDialog();
   SetWindowText(_title);

   for ( size_t n = 0; n < _textToDisplay.size(); ++n )
   {
      _listbox.AddString( mvv::getWideString( _textToDisplay[ n ] ).c_str() );
   }
   return TRUE;
}

void TexboxSelection::OnOK()
{
   int nCount = _listbox.GetSelCount();
   CArray<int,int> aryListBoxSel;
   
   aryListBoxSel.SetSize(nCount);
   _listbox.GetSelItems(nCount, aryListBoxSel.GetData()); 
   

   _selection.clear();
   for ( size_t n = 0; n < aryListBoxSel.GetSize(); ++n )
   {
      _selection.push_back( aryListBoxSel.GetAt(n ) );
   }

   CDialog::OnOK();
}


BEGIN_MESSAGE_MAP(TexboxSelection, CDialog)
END_MESSAGE_MAP()


// TexboxSelection message handlers
