// mvvForms.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "mvvForms.h"
#include "windows.h"
#include "utils.h"
#include <iostream>

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

namespace mvv
{
   std::vector<std::string> openFiles( const std::string& root )
   {
      std::cout << "OPENED STARTED" << std::endl;
      OPENFILENAME ofn;
      const unsigned maxSize = 1024 * 10;
      wchar_t szFileName[ maxSize ] = L"";

      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
      ofn.hwndOwner = NULL;
      ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = szFileName;
      ofn.nMaxFile = maxSize;
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
      ofn.lpstrDefExt = L"txt";

      std::vector<std::string> files;
      if(GetOpenFileName(&ofn))
      {
         std::cout << "OPENED" << std::endl;
      // Do something usefull with the filename stored in szFileName 
      }

      std::cout << "OPENED DONE" << std::endl;


      return files;
   }

   void createMessageBoxError( const std::string& title, const std::string& msg )
   {
      std::wstring titlew = getWideString( title );
      std::wstring msgw = getWideString( msg );
      MessageBox( 0, titlew.c_str(), msgw.c_str(), MB_ICONERROR | MB_OK);
   }
}