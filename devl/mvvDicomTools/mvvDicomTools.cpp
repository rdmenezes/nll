// mvvDicomTools.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "mvvDicomTools.h"
#include "utils.h"
#include "read-dicom.h"

/*
#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif
*/

using namespace mvv::parser;
using namespace mvv;

void importFunctions( CompilerFrontEnd& e, mvv::platform::Context& context )
{
   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomAttributs" ) ) ) );
      ensure( ty, "can't find 'DicomAttributs' in srouce" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readDicomVolume" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), ty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadDicomVolume( fn, context ) ) );
   }
}