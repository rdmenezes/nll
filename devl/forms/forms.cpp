// forms.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "forms.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#include "utils.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#include <Shlobj.h>
#include <algorithm>
#include <sstream>
#include "TextDialog.h"
#include "TexboxSelection.h"

#ifdef WIN32
#  include <direct.h>
#  define getcwd  _getcwd
#  define chdir   _chdir
#else
#  include <unistd.h>
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#pragma comment(lib, "comctl32.lib")

namespace mvv
{
   HINSTANCE hinstDLL;

   namespace impl
   {
      #define ILNext(pidl)           ILSkip(pidl, (pidl)->mkid.cb)
      #define ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))

      static HRESULT SHGetUIObjectFromFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
      {
         LPCITEMIDLIST pidlChild;
         IShellFolder* psf;
         *ppv = NULL;
         HRESULT hr = SHBindToParent(pidl, IID_IShellFolder, (LPVOID*)&psf, &pidlChild);
         if (SUCCEEDED(hr))
         {
            hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
            psf->Release();
         }
      return hr;
      }

      static HRESULT SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
      {
         DWORD cbTotal = 0;
         if (pidl)
         {
            LPCITEMIDLIST pidl_temp = pidl;
            cbTotal += pidl_temp->mkid.cb;
            while (pidl_temp->mkid.cb) 
            {
               cbTotal += pidl_temp->mkid.cb;
               pidl_temp = ILNext(pidl_temp);
            }
         }

         *ppidl = (LPITEMIDLIST)CoTaskMemAlloc(cbTotal);

         if (*ppidl)
         CopyMemory(*ppidl, pidl, cbTotal);

         return  *ppidl ? S_OK: E_OUTOFMEMORY;
      }

      // Get the target PIDL for a folder PIDL. This also deals with cases of a folder  
      // shortcut or an alias to a real folder.
      static HRESULT SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl)
      {
         IShellLink *psl;

         *ppidl = NULL;

         HRESULT hr = SHGetUIObjectFromFullPIDL(pidlFolder, NULL, IID_IShellLink, (LPVOID*)&psl);

         if (SUCCEEDED(hr))
         {
            hr = psl->GetIDList(ppidl);
            psl->Release();
         }

         // It's not a folder shortcut so get the PIDL normally.
         if (FAILED(hr))
            hr = SHILClone(pidlFolder, ppidl);

         return hr;
      }
   }

   std::string getWorkingDirectory()
   {
      char cCurrentPath[FILENAME_MAX];
      if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
      {
         throw std::runtime_error( "error" );
      }
      cCurrentPath[sizeof(cCurrentPath) - 1] = '/0';
      return std::string( cCurrentPath );
   }

   void setWorkingDirectory( const std::string& s )
   {
      int err = chdir( s.c_str() );
      if ( err )
      {
         throw std::runtime_error( "error" );
      }
   }

   void unixStylePath( std::string& s )
   {
      std::replace( s.begin(), s.end(), '\\', '/' );
   }

   std::vector<std::string> openFiles( const std::string& title )
   {
      std::wstring titlew = getWideString( title );

      // save the working directory
      const std::string workingDir = getWorkingDirectory();

      OPENFILENAME ofn;
      const unsigned maxSize = 1024 * 10;
      wchar_t szFileName[ maxSize + 1 ] = L"";
      szFileName[ maxSize ] = 0;

      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
      ofn.hwndOwner = NULL;
      ofn.lpstrFilter = L"MF2 Files (*.mf2)\0*.mf2\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = szFileName;
      ofn.lpstrTitle = titlew.c_str();
      ofn.nMaxFile = maxSize;
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
      ofn.lpstrDefExt = L"txt";

      std::vector<std::string> files;
      std::string dir;
      if(GetOpenFileName(&ofn))
      {
         unsigned  start = 0;
         while ( start < maxSize )
         {
            if ( szFileName[ start ] == 0 )
               break;
            unsigned end = start;
            while ( szFileName[ end ] != 0 && end < maxSize )
               ++end;
            if ( start != end )
            {
               std::string str( getString( std::wstring( szFileName + start ) ) );
               if ( start == 0 )
               {
                  dir = str;
                  unixStylePath( dir );
                  dir = dir + "/";
               } else {
                  files.push_back( dir + str );
               }
            }
            start = end + 1; // skip the terminal 0
         }
      }

      setWorkingDirectory( workingDir );
      return files;
   }

   std::string openFolder( const std::string title )
   {
      BROWSEINFO bi = { 0 };
      TCHAR path[MAX_PATH];
      std::wstring titlew = getWideString( title );
      bi.lpszTitle = titlew.c_str();
      bi.pszDisplayName = path;
      LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
      if ( pidl != 0 )
      {
         LPITEMIDLIST pidlTarget;
	
         LPWSTR pszPath = path;
         *pszPath = 0;
         HRESULT hr = impl::SHGetTargetFolderIDList(pidl, &pidlTarget);

         if (SUCCEEDED(hr))
         {
            SHGetPathFromIDListW(pidlTarget, pszPath);   // Make sure it is a path
            CoTaskMemFree(pidlTarget);
         } else {
            throw std::runtime_error( "cannot open folder" );
         }

         // free memory used
         IMalloc * imalloc = 0;
         if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
         {
            imalloc->Free ( pidl );
            imalloc->Release ( );
         }

         std::string p = getString( path );
         unixStylePath( p );
         return p;
      }
      return "";
   }

   void createMessageBoxError( const std::string& title, const std::string& msg )
   {
      std::wstring titlew = getWideString( title );
      std::wstring msgw = getWideString( msg );
      const int val = MessageBox( 0, msgw.c_str(), titlew.c_str(), MB_ICONERROR | MB_OK);
      if ( val == 0 )
      {
         throw std::runtime_error( "can't create message box" );
      }
   }

   bool createMessageBoxQuestion( const std::string& title, const std::string& msg )
   {
      std::wstring titlew = getWideString( msg );
      std::wstring msgw = getWideString( title );
      const int val = MessageBox( 0, titlew.c_str(), msgw.c_str(), MB_ICONQUESTION | MB_YESNO);
      if ( val == IDNO )
         return false;
      if ( val == IDYES )
         return true;

      // error
      throw std::runtime_error( "can't create message box question" );
   }

   template <class T>
   std::string val2str( T val )
   {
      std::stringstream ss;
      ss << val;
      std::string n;
      ss >> n;
      return n;
   }

   std::string createMessageBoxText( const std::string& title )
   {
      TextDialog dialog;
      INT_PTR res = dialog.DoModal( title );
      if ( res == -1 )
      {
         throw std::runtime_error( "cannot create modal dialog, GetLastError()=" + val2str( GetLastError() ) );
      }

      return dialog.getString();
   }

   std::vector<unsigned> createMessageBoxTextSelection( const std::string& title, const std::vector<std::string>& texts )
   {
      std::vector<unsigned> selection;

      TexboxSelection box;
      INT_PTR res = box.DoModal( title, texts );
      if ( res == -1 )
      {
         throw std::runtime_error( "cannot create modal dialog, GetLastError()=" + val2str( GetLastError() ) );
      }
      return box.getSelection();
   }
}

CWinApp theApp;   // the application

// here we need to get the handle of the DLL, so that the dialog resources are looked in the
// correct assembly
BOOL WINAPI DllMain(HINSTANCE hinst,DWORD reason,LPVOID)
{
   if ( reason == DLL_PROCESS_ATTACH )
   {
      std::cout << "forms.dll attached!" << std::endl;
      mvv::hinstDLL = hinst;

      if (!AfxWinInit(mvv::hinstDLL, NULL, ::GetCommandLine(), 0))
      {
         throw std::runtime_error( "failed to initialize MFC..." );
      }
   }
   return true;
}