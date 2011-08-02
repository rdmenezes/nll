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

namespace mvv
{
   ui32 dicomVolumeId = 1;
}

void importFunctions( CompilerFrontEnd& e, mvv::platform::Context& context )
{
   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomAttributs" ) ) ) );
      ensure( ty, "can't find 'DicomAttributs' in srouce" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readDicomVolume" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), ty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadDicomVolume( fn, context, e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomAttributs" ) ) ) );
      TypeArray* arrayty = new TypeArray( 0, *ty, false );
      TypeArray* arrayint = new TypeArray( 0, *new TypeInt( false ), false );
      ensure( ty, "can't find 'DicomAttributs' in srouce" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readDicomVolumes" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), arrayty, arrayint ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadDicomVolumes( fn, context, e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      TypeArray* arrayty = new TypeArray( 0, *ty, false );
      ensure( ty, "can't find 'DicomAttributs' in srouce" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSizeDicomVolumes( fn ) ) );
   }

   {
      TypeArray* arrayty = new TypeArray( 0, *new TypeInt( false ), false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSizeInt( fn ) ) );
   }
}