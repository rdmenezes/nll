// mvvDicomTools.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "mvvDicomTools.h"
#include "utils.h"
#include "read-dicom.h"
#include "dicom-slice.h"

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
   std::cout << "mvvDicomTools.dll import functions..." << std::endl;

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
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomSlice" ) ) ) );
      TypeArray* arrayty = new TypeArray( 0, *ty, false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSizeDicomSlice( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "DicomSlice" ), platform::Symbol::create( "~DicomSlice" ) ), std::vector<const Type*>() );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionDicomSliceDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readDicomSlice" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadDicomSlice( fn, e ) ) );
   }

   {
      TypeArray* arrayty = new TypeArray( 0, *new TypeInt( false ), false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readDicomSlices" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), arrayty ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadDicomSlices( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "DicomSlice" ), platform::Symbol::create( "setTag" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionDicomSliceSetTag( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "DicomSlice" ), platform::Symbol::create( "getTag" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionDicomSliceGetTag( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomSlice" ) ) ) );
      ensure( ty, "can't find 'DicomSlice' in source" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeDicomSlice" ) ), nll::core::make_vector<const Type*>( ty, new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteDicomSlice( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomSlice" ) ) ) );
      Type* ty2 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "DicomSliceNamingFunc" ) ) ) );
      ensure( ty && ty2, "can't find 'DicomSlice' or 'DicomSliceNamingFunc' in srouce" );
      TypeArray* arrayty = new TypeArray( 0, *ty, false );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeDicomSlices" ) ), nll::core::make_vector<const Type*>( arrayty, new TypeString( false ), ty2 ) );
      ensure( fn, "can't find the function declaration in mvvDicomTools.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteDicomSlices( fn, e ) ) );
   }

   std::cout << "mvvDicomTools.dll import functions done" << std::endl;
}