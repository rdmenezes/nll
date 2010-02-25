#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvScript/parser-context.h>
#include <mvvScript/visitor-print.h>
#include <mvvScript/visitor-default.h>
#include <mvvScript/visitor-bind.h>
#include <mvvScript/visitor-register-declarations.h>
#include <mvvScript/visitor-type.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"

struct TestBasic
{
   /**
    @brief basic input to test the lexer && parser only
    */
   void testBinding1()
   {
      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;\n" 
                                    "  int n = 1;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test { class Test2{}}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test { class Test{}}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int Test; class Test { class Test2{ class Test{} }}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int Test3; class Test { class Test2{} } class Test2{}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int Test(); int Test();" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int Test(); int Test( int a ); int Test2;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int Test(); class Test{}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{} int Test( int a );" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( " int Test( int a ); class Test{}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( " class Aha{ class Test{}} int Test( int a );" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }
   }

   void testDummy2()
   {
       ParserContext context;
       Ast* exp = 0;
      
      exp = context.parseString( "if (a) { print( a ); print( a ); } else { print( a3 ); }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "if (1 && 1 + 1) { return 2.5; } else { return 3; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "if (1 * atest1.val[a[8]+3] ) { print(2.5); return 5; } else { print(3); }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class test { var a; int n; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "call(4, 3); call(); call(4);" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int call( int n ){return 5;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int haha( test a){return 5;} }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int test = 0; int main(){ dsf haha; int test; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int aa; int haha( test a){ return 5;} }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int aa; int haha( test a){ return 5;} float test2; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int main(){test.print(\"asf\"); return 0;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "{var list[] = {5, 4, 3};}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "{int a = 3 + test;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int n = 1 * 2; int main( float a, int test = 2 * 3 ){return 0;} class A{ void init( int n = 0 ){} int n = 0; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "a[1].a[5] = 2*3+1;" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int n[] = {2 * 3};" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "import \"test1.v\" include \"test2.v\" class Test{TTest test; int getVal( int a = 2 * 5, float b ); int getVal(){ int a; int b; string strings[5]; strings[ 0 ] = \"test2\"; return a + b;} string str = \"test\"; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "getVal(5);" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int n[ 5 ][ 4 ][ 3 ]; print( n[ 0 ][ 1 ][ 2 ] );" );
      TESTER_ASSERT( exp );

      exp = context.parseString( "class Test{ int a[] = {1}; int a[2][2][3];}" );
      TESTER_ASSERT( exp );

      std::cout << "msg=" << context.getError();
   }

   void testFull1()
   {
      ParserContext context;
      Ast* exp = 0;
      
      exp = context.parseFile( TEST_PATH "test1.txt" );
      std::cout << "msg=" << context.getError();
      TESTER_ASSERT( exp );

      VisitorPrint p( std::cout );
      p( *exp );
   }

   void testSymbolTableDisctionary()
   {
      YYLTYPE loc;
      AstDecls decls( loc );
      AstDeclClass c1( loc, mvv::Symbol::create("C1"), &decls );
      AstDeclClass c2( loc, mvv::Symbol::create("C2"), &decls );
      AstDeclClass c3( loc, mvv::Symbol::create("C3"), &decls );
      AstDeclClass c4( loc, mvv::Symbol::create("C4"), &decls );

      SymbolTableDictionary dictionary;
      dictionary.begin_scope( c1.getName(), &c1 );
      
      dictionary.begin_scope( c2.getName(), &c2 );
      dictionary.end_scope();
      dictionary.begin_scope( c3.getName(), &c3 );
      dictionary.end_scope();
      dictionary.end_scope();
      dictionary.begin_scope( c4.getName(), &c4 );
      dictionary.end_scope();

      const AstDeclClass* tc1 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1") ) );
      const AstDeclClass* tc2 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1"), mvv::Symbol::create("C2") ) );
      const AstDeclClass* tc3 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1"), mvv::Symbol::create("C3") ) );
      const AstDeclClass* tc4 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C4") ) );
      TESTER_ASSERT( tc1 == &c1 );
      TESTER_ASSERT( tc2 == &c2 );
      TESTER_ASSERT( tc3 == &c3 );
      TESTER_ASSERT( tc4 == &c4 );
   }

   void testBinding2()
   {
      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "  int n;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "  int nn = n * 3;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "  int nn = n2 * 3;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n = 0;" 
                                    "{"
                                    "  int n = 1;"
                                    "  int nn = n2[15] * 3;"
                                    "}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "Test test = 0;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int fn( Test t );" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{} int fn( Test t );" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test clone(); }" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ class Test2{} class Test3{ Test2 clone(); } }" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ class Test2{} int empty( Test3 c ); class Test3{ } }" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ class Test2{ class Test3{ Test4 haha(); } } Test2::Test3 create(){} Test::Test2::Test3 create2(); class Test4{ } }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "void print( int n ){} class Test{ int operator()( int n ){ return 0; } }  Test array[5]; print( array[0](0) );" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "{return 5;}" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }


      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n; void test( int n, float f ) { return n * f;}" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n; void test( float f ) { return n * f;}" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n; void test( float f ) { float f; return f;}" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ int n; int get(){ return n; } }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n; class Test{ int n; class Test2{ int get(){ return n; } } }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

       {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "{ int test(){ return 0;} }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

       {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "{ class Test{} }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import \"test\"" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "include \"test\"" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "{import \"test\"}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "{include \"test\"}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n){} } int Test(){return 0;}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n){} } Test test = Test(3);" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n){} } { Test Test = Test(3); }" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ Test(int n){} } Test Test = Test(3);" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{} } Test Test2 = Test(3);" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{} } Test::Test2 Test2 = typename Test::Test2(3);" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{} } int Test2 = typename Test::Test2::Test3(3);" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{ class Test3{}} } int Test2 = typename Test::Test2::Test3(3);" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{ class Test3{ class Test4{ Test2::Test3 haha; }}} }" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{ Test3::Test4 haha; class Test3{ class Test4{ }}} }" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{ class Test3{ class Test4{ Test2 haha; }}} }" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "class Test{ class Test2{} } int Test2 = typename Test::Test2::Test(3);" );
         TESTER_ASSERT( exp );

         VisitorPrint p( std::cout );
         p( *exp );

         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }
   }

   void testType1()
   {
      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n; int testint = 0; testint = n + testint;" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{} Test test; int test2; test2 = test + test2;" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ test(){} }" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int func( int n ){ return n; } int sum = func( 10 );" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
      }
      /*
      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ int n;} Test test; int testint; testint = test.n" );
         VisitorPrint p( std::cout );
         p( *exp );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      }
      */
   }
};

TESTER_TEST_SUITE(TestBasic);

TESTER_TEST(testBinding2);
TESTER_TEST(testBinding1);
TESTER_TEST(testDummy2);
TESTER_TEST(testFull1);
TESTER_TEST(testSymbolTableDisctionary);
//TESTER_TEST(testType1);
TESTER_TEST_SUITE_END();