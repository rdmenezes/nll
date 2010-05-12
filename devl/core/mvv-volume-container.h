#ifndef CORE_MVV_VOLUME_CONTAINER_H_
# define CORE_MVV_VOLUME_CONTAINER_H_

# include "core.h"
# include <mvvLauncher/init.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include "mvv-lut.h"

using namespace mvv::parser;
using namespace mvv;

namespace impl
{
   struct VolumeContainer
   {
      VolumeContainer( ResourceStorageVolumes volumeStorage ) : volumes( volumeStorage )
      {}

      std::vector<RuntimeValue>  volumeIdList;
      std::vector<RuntimeValue>  lutList;

      ResourceMapTransferFunction luts;
      ResourceFloats              intensities;
      ResourceVolumes             volumes;
   };
}

class FunctionVolumeContainerConstructor : public FunctionRunnable
{
   typedef ::impl::VolumeContainer Pointee;

public:
   FunctionVolumeContainerConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      ContextVolumes* volumes = _context.get<ContextVolumes>();
      if ( !volumes )
      {
         throw RuntimeException( "ContextVolumes context has not been loaded" );
      }

      // construct the type
      Pointee* lut = new Pointee( volumes->volumes );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( lut ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context&     _context;
};

class FunctionVolumeContainerDestructor : public FunctionRunnable
{
   typedef ::impl::VolumeContainer Pointee;

public:
   FunctionVolumeContainerDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete pointee;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionVolumeContainerAdd : public FunctionRunnable
{
   typedef ::impl::VolumeContainer Pointee;

public:
   FunctionVolumeContainerAdd( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // preconditions
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::STRING );
      assert( (*v3.vals)[ 0 ].type == RuntimeValue::PTR );
      assert( v4.type == RuntimeValue::FLOAT );

      pointee->volumeIdList.push_back( v2 ); // keep a reference on the VolumeID
      pointee->lutList.push_back( v3 ); // keep a reference on the lut
      pointee->lutList.rbegin()->stringval = (*v2.vals)[ 0 ].stringval; // we save the associated VolumeID so we can remove it later on

      mvv::SymbolVolume volume = mvv::SymbolVolume::create( (*v2.vals)[ 0 ].stringval );
      pointee->luts.insert( volume, *reinterpret_cast<FunctionLutConstructor::Pointee*>( (*v3.vals)[ 0 ].ref ) );
      pointee->volumes.insert( volume );
      pointee->intensities.insert( volume, v4.floatval );

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionVolumeContainerErase : public FunctionRunnable
{
   typedef ::impl::VolumeContainer Pointee;

public:
   FunctionVolumeContainerErase( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // preconditions
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::STRING );

      mvv::SymbolVolume volume = mvv::SymbolVolume::create( (*v2.vals)[ 0 ].stringval );

      // first remove from the volume ID
      std::vector<RuntimeValue>::iterator it = pointee->volumeIdList.begin();
      for ( ; it != pointee->volumeIdList.end(); )
      {
         std::vector<RuntimeValue>::iterator cur = it++;
         
         mvv::SymbolVolume vol = mvv::SymbolVolume::create( (*cur->vals)[ 0 ].stringval );
         if ( vol == volume )
         {
            it = pointee->volumeIdList.erase( cur );
         }
      }

      // remove from the volumes
      it = pointee->lutList.begin();
      for ( ; it != pointee->lutList.end(); )
      {
         std::vector<RuntimeValue>::iterator cur = it++;
         
         mvv::SymbolVolume vol = mvv::SymbolVolume::create( (*cur->vals)[ 0 ].stringval );
         if ( vol == volume )
         {
            it = pointee->lutList.erase( cur );
         }
      }

      // remove the values potentially used by a segment
      pointee->luts.erase( volume );
      pointee->volumes.erase( volume );

      

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};


#endif