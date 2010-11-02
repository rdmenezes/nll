#ifndef CORE_MVV_GEOMETRY_H_
# define CORE_MVV_GEOMETRY_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionPlaneConstructor : public FunctionRunnable
{
public:
   typedef nll::core::GeometryPlane Pointee;

public:
   FunctionPlaneConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );

      nll::core::vector3f position;
      nll::core::vector3f axis1;
      nll::core::vector3f axis2;
      getVector3fValues( v2, position );
      getVector3fValues( v3, axis1 );
      getVector3fValues( v4, axis2 );

      // construct the type
      Pointee* p = new Pointee( position, axis1, axis2 );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( p ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionPlaneDestructor : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete lut;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionPlaneOrthogonalProjection : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneOrthogonalProjection( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      nll::core::vector3f pp;
      getVector3fValues( v2, pp );
      nll::core::vector3f projection = p->getOrthogonalProjection( pp );

      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, projection[ 0 ], projection[ 1 ], projection[ 2 ] );
      return rt;
   }
};

class FunctionPlaneWorldToPlaneCoordinate : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneWorldToPlaneCoordinate( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      nll::core::vector3f pp;
      getVector3fValues( v2, pp );
      nll::core::vector2f projection = p->worldToPlaneCoordinate( pp );

      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector2f( rt, projection[ 0 ], projection[ 1 ] );
      return rt;
   }
};

class FunctionPlanePlaneToWorldCoordinate : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlanePlaneToWorldCoordinate( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      nll::core::vector2f pp;
      getVector2fValues( v2, pp );
      nll::core::vector3f projection = p->planeToWorldCoordinate( pp );

      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, projection[ 0 ], projection[ 1 ], projection[ 2 ] );
      return rt;
   }
};

class FunctionPlaneGetIntersection : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneGetIntersection( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      nll::core::vector3f px;
      getVector3fValues( v2, px );
      nll::core::vector3f dir;
      getVector3fValues( v3, dir );

      nll::core::vector3f proj;
      bool res = p->getIntersection( px, dir, proj );
      if ( !res )
         throw std::runtime_error( "no intersection between plane and line!" );

      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, proj[ 0 ], proj[ 1 ], proj[ 2 ] );
      return rt;
   }
};

class FunctionPlaneGetAxisX : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneGetAxisX( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, p->getAxisX()[ 0 ], p->getAxisX()[ 1 ], p->getAxisX()[ 2 ] );
      return rt;
   }
};

class FunctionPlaneGetAxisY : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneGetAxisY( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, p->getAxisY()[ 0 ], p->getAxisY()[ 1 ], p->getAxisY()[ 2 ] );
      return rt;
   }
};

class FunctionPlaneGetNormal : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneGetNormal( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, p->getNormal()[ 0 ], p->getNormal()[ 1 ], p->getNormal()[ 2 ] );
      return rt;
   }
};

class FunctionPlaneGetOrigin : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneGetOrigin( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, p->getOrigin()[ 0 ], p->getOrigin()[ 1 ], p->getOrigin()[ 2 ] );
      return rt;
   }
};

class FunctionPlaneContains : public FunctionRunnable
{
   typedef FunctionPlaneConstructor::Pointee Pointee;

public:
   FunctionPlaneContains( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      nll::core::vector3f px;
      getVector3fValues( v2, px );

      bool res = p->contains( px );
      
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = res ? 1 : 0;
      return rt;
   }
};


#endif