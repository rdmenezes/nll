#ifndef MVV_DICOM_TOOLS_DICOM_SLICE_H_
# define MVV_DICOM_TOOLS_DICOM_SLICE_H_

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <dcmtk/dcmimage.h>
# include "read-dicom.h"
# include "dicom-attributs.h"
# include "utils.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

namespace mvv
{
   RuntimeValue createDicomSlice( DcmFileFormat* p, CompilerFrontEnd& e )
   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("DicomSlice") ) ) );
      RuntimeValue v( RuntimeValue::TYPE );
      v.vals = RuntimeValue::RefcountedValues( &e.getEvaluator(), ty, new RuntimeValues( 2 ) );
      DicomAttributs::exportTagsToRuntime( (*v.vals)[ 0 ], createDicomAttributs( *p->getDataset() ) );

      RuntimeValue ptr( RuntimeValue::PTR );
      ptr.ref = reinterpret_cast<RuntimeValue*>( p );

      (*v.vals)[ 1 ] = ptr;
      return v;
   }

   class FunctionDicomSliceDestructor : public FunctionRunnable
   {
   public:
      typedef DcmFileFormat Pointee;

   public:
      FunctionDicomSliceDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments" );
         }

         RuntimeValue& v1 = unref( *args[ 0 ] );

         // check we have the data
         ensure( (*v1.vals).size() == 2, "we are expecting [0] <mainAttributs>, [1] <source object pointer>" );
         ensure( (*v1.vals)[ 0 ].type == RuntimeValue::TYPE, "must be <mainAttributs> structure" ); // it must be 1 field, PTR type
         ensure( (*v1.vals)[ 1 ].type == RuntimeValue::PTR, "must be a pointer on the source object" ); // it must be 1 field, PTR type
         Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 1 ].ref );

         // deallocate data
         delete p;
         (*v1.vals)[ 1 ].ref = 0;
         
         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }
   };


   class FunctionReadDicomSlice : public FunctionRunnable
   {

   public:
      FunctionReadDicomSlice( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ),  _e( e )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );

         DcmFileFormat* dcm = new DcmFileFormat();
         OFCondition cond = dcm->loadFile( v0.stringval.c_str() );
         if ( cond.good() )
         {
            return createDicomSlice( dcm, _e );
         } else {
            std::cerr << "invalid DICOM file:" << v0.stringval << std::endl;
            delete dcm;
            RuntimeValue rt( RuntimeValue::NIL );
            return rt;
         }
      }

   private:
      CompilerFrontEnd&          _e;
   };

   class FunctionReadDicomSlices : public FunctionRunnable
   {

   public:
      FunctionReadDicomSlices( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ),  _e( e )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );

         DicomDatasets datasets;
         datasets.loadDicomDirectory( v0.stringval );

         std::vector<DcmFileFormat*> files;

         for ( size_t series = 0; series < datasets.getSeriesUids().size(); ++series )
         {
            for ( size_t i = 0; i < datasets.getSeriesUids()[ series ].size(); ++i )
            {
               DcmFileFormat& f = datasets.getSeriesUids()[ series ][ i ];
               files.push_back( new DcmFileFormat( f ) );
            }
         }

         RuntimeValue rt( RuntimeValue::TYPE );
         rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), 0, new RuntimeValues( files.size() ) );

         for ( size_t n = 0; n < files.size(); ++n )
         {
            (*rt.vals)[ n ] = createDicomSlice( files[ n ], _e );
         }
         return rt;
      }

   private:
      CompilerFrontEnd& _e;
   };

   class FunctionWriteDicomSlice : public FunctionRunnable
   {
      typedef FunctionDicomSliceDestructor::Pointee   Pointee;

   public:
      FunctionWriteDicomSlice( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 2 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting (DicomSlice, string)" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );


         ensure( v0.type == RuntimeValue::TYPE, "must be a type" );
         ensure( (*v0.vals).size() == 2, "must contain (DicomAttributs, PTR)" );
         Pointee* p = reinterpret_cast<Pointee*>( (*v0.vals)[ 1 ].ref );
         p->saveFile( v1.stringval.c_str() );

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }
   };


   class FunctionWriteDicomSlices : public FunctionRunnable
   {
      typedef FunctionDicomSliceDestructor::Pointee   Pointee;

   public:
      FunctionWriteDicomSlices( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 3 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting (DicomSlice, string)" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );
         RuntimeValue& v2 = unref( *args[ 2 ] );


         if ( v0.type == RuntimeValue::TYPE )
         {
            for ( size_t n = 0; n < (*v0.vals).size(); ++n )
            {
               // get the name of the slice based on the header
               std::vector<RuntimeValue> param = nll::core::make_vector<RuntimeValue>( (*v0.vals)[ n ] );
               RuntimeValue val = _e.evaluateCallback( v2, param );

               // check the slice is valid
               RuntimeValue& slice = (*v0.vals)[ n ];
               if ( slice.type != RuntimeValue::TYPE || (*slice.vals).size() != 2 )
               {
                  throw std::runtime_error( "FunctionWriteDicomSlices::invalid arguments" );
               }

               // override the slice tags by the DicomAttributs tags
               RuntimeValue& sliceStruct = (*slice.vals)[ 0 ];
               Pointee* p = reinterpret_cast<Pointee*>( (*slice.vals)[ 1 ].ref );
               DicomAttributs attributs = createDicomAttributs( sliceStruct );

               DcmDataset* data = p->getDataset();
               DicomAttributs::exportTagsToDataset( attributs, *data );

               DicomWrapper wrapper( *p->getDataset() );
               DicomWrapper wrapper2( *data );
               wrapper2.setPatientName( "TESTESTTEST") ;
               std::cout << "nameFound=" << wrapper.getPatientName() << std::endl;
               std::cout << "nameFound=" << wrapper2.getPatientName() << std::endl;
               std::cout << "name=" << attributs.patientName << std::endl;

               DcmFileFormat f( data );
               std::cout << "nameFound2=" << DicomWrapper( *f.getDataset() ).getPatientName() << std::endl;
               std::cout << "nameFound3=" << DicomWrapper( *data ).getPatientName() << std::endl;

               // save the slice
               f.saveFile( ( v1.stringval + val.stringval ).c_str() );
            }
         }

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }

   private:
      CompilerFrontEnd&    _e;
   };
}

#endif
