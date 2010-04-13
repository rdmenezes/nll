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
         
         exp = context.parseString( "import int Test(); import int Test();" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int Test(); import int Test( int a ); int Test2;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int Test(); class Test{}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{} import int Test( int a );" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( " import int Test( int a ); class Test{}" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( " class Aha{ class Test{}} import int Test( int a );" );
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
      exp = context.parseString( "import \"test1.v\" include \"test2.v\" class Test{TTest test; import int getVal( int a = 2 * 5, float b ); int getVal(){ int a; int b; string strings[5]; strings[ 0 ] = \"test2\"; return a + b;} string str = \"test\"; }" );
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

   void testFull2()
   {
      ParserContext context;
      Ast* exp = 0;
      
      exp = context.parseFile( TEST_PATH "std.txt" );
      std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      TESTER_ASSERT( exp );
      VisitorRegisterDeclarations visitor( context );
      visitor( *exp );
      std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      TESTER_ASSERT( !context.getError().getStatus() );

      VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
      visitorBind( *exp );
      std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      TESTER_ASSERT( !context.getError().getStatus() );

      VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
      visitorType( *exp );

      VisitorPrint p( std::cout );
      p( *exp );
      std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
      TESTER_ASSERT( !context.getError().getStatus() );
      delete exp;
   }

   void testSymbolTableDisctionary()
   {
      YYLTYPE loc;
      AstDeclClass* c1 = new AstDeclClass( loc, mvv::Symbol::create("C1"), new AstDecls( loc ) );
      AstDeclClass* c2 = new AstDeclClass( loc, mvv::Symbol::create("C2"), new AstDecls( loc ) );
      AstDeclClass* c3 = new AstDeclClass( loc, mvv::Symbol::create("C3"), new AstDecls( loc ) );
      AstDeclClass* c4 = new AstDeclClass( loc, mvv::Symbol::create("C4"), new AstDecls( loc ) );

      SymbolTableDictionary dictionary;
      dictionary.begin_scope( c1->getName(), c1 );
      
      dictionary.begin_scope( c2->getName(), c2 );
      dictionary.end_scope();
      dictionary.begin_scope( c3->getName(), c3 );
      dictionary.end_scope();
      dictionary.end_scope();
      dictionary.begin_scope( c4->getName(), c4 );
      dictionary.end_scope();

      const AstDeclClass* tc1 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1") ) );
      const AstDeclClass* tc2 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1"), mvv::Symbol::create("C2") ) );
      const AstDeclClass* tc3 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C1"), mvv::Symbol::create("C3") ) );
      const AstDeclClass* tc4 = dictionary.find( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("C4") ) );
      TESTER_ASSERT( tc1 == c1 );
      TESTER_ASSERT( tc2 == c2 );
      TESTER_ASSERT( tc3 == c3 );
      TESTER_ASSERT( tc4 == c4 );
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
         
         exp = context.parseString( "import int fn( Test t );" );
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
         
         exp = context.parseString( "class Test{} import int fn( Test t );" );
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
         
         exp = context.parseString( "class Test{ import Test clone(); }" );
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
         
         exp = context.parseString( "class Test{ class Test2{} class Test3{ import Test2 clone(); } }" );
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
         
         exp = context.parseString( "class Test{ class Test2{} import int empty( Test3 c ); class Test3{ } }" );
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
         
         exp = context.parseString( "class Test{ class Test2{ class Test3{ import Test4 haha(); } } Test2::Test3 create(){} import Test::Test2::Test3 create2(); class Test4{ } }" );
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

      {
         ParserContext context;
         Ast* exp = 0;
         
         // var can't have the same name than a class
         exp = context.parseString( "int fn1(){ int fn2(){} }" );
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
         exp = context.parseString( "class Test { class Test2{} Test(){}} Test test; test = Test();" );
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
         exp = context.parseString( "class Test { class Test2{} Test(){}} Test::Test2 test; test = typename Test::Test2();" );
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
   }

   /*
   static AstDeclFun* createFunctionPrototype( mvv::Symbol& name, AstTypeT* returnType, const std::vector<AstType*>& args )
   {
      YYLTYPE loc;
      AstDeclVars* vars = new AstDeclVars( loc );
      for ( ui32 n = 0; n < args.size(); ++n )
      {
         vars->insert( new AstDeclVar( loc, args[ n ], mvv::Symbol::create("unnamed"), 0 ) );
      }
      AstDeclFun* fn = new AstDeclFun( loc, returnType, name, vars, 0 );
      fn->setNodeType( returnType->getNodeType() );
      return fn;
   }
   */

   void testType1()
   {
      {
         ParserContext context;
         Ast* exp = 0;

         exp = context.parseString( "class Test{Test(){}} Test test; int test2; test2 = test + test2;" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+(int a, int b); int testint = testint + testint;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         SymbolTableVars vars = visitorBind.getVars();
         SymbolTableFuncs funcs = visitorBind.getFuncs();
         SymbolTableClasses classes = visitorBind.getClasses();
         VisitorType visitorType( context, vars, funcs, classes );
         visitorType( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+(int a, int b); int n; int testint = 0; testint = n + testint;" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         SymbolTableVars vars = visitorBind.getVars();
         SymbolTableFuncs funcs = visitorBind.getFuncs();
         SymbolTableClasses classes = visitorBind.getClasses();
         VisitorType visitorType( context, vars, funcs, classes );
         visitorType( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} }" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ test(){} }" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int func( int n ){ return; }" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int func( int n ){ return 1; return \"test\"; }" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "void func( int n ){ return void; }" );
         TESTER_ASSERT( !exp );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{int test(){}}" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{void test(){ void test2(){} }}" );;
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import float operator+( int n, float nn); int n = 3; float f = 2.5; int nn = f + n;" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} } Test n; float f = 2.5; int nn = f + n;" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[ 2 ][ 2 ]; int nn[ 2 ]; nn = n[ 0 ]; n[ 0 ] = nn;" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[ 2 ][ 2 ]; int nn[ 2 ]; nn = n[ 0 ]; n[ 0 ] = nn;" );
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[ 3.5 ];" );
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[ 3 ]; n[ 0.5 ] = 5;" );
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
         delete exp;
      }


      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} Test test1; Test test2; } Test aa; aa = aa.test1.test2.test1;" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} Test test1; Test test2; } Test aa; aa = aa.test1.test3.test1;" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} Test test1; Test test2; } Test aa; aa = aa1.test1.test2.test1;" );
         
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} Test test1; Test test2; } Test aa[ 5 ]; aa[ 0 ].test1 = aa[ 0 ];" );
         
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
         delete exp;
      }

       {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test test1; Test test2; } Test aa[ 5 ][ 5 ]; aa[ 0 ][ 0 ].test1 = aa[ 0 ];" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} Test test1; Test test2; } Test aa[ 5 ][ 5 ]; aa[ 0 ][ 0 ].test1 = aa[ 0 ][ 0 ];" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test test1; Test test2; } Test aa;" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int fn(){return 0;} } Test test; test.fn();" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int n;} Test test; int testint; testint = test.n;" );
         
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int operator[]( int n ){return n;} void print(int n){}} Test n; n.print( n[3] );" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int operator()( int n ){return n;} void print(int n){}} Test n; n.print( n(3) );" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int operator()( int n ){return n;} void print(int n){}} Test n; n.print2( n(3) );" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int operator()( int n ){return n;} void print(int n){}} Test n; n2.print( n(3) );" );
         
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }


      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int vala; float valb; Test( int a, float b ){ \nvala = a; valb = b; } } Test a = Test( 0, 1.5);");
         
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
         delete exp;
      }


      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "float n( float v ){ return v; } float tmp = n( 5.0 );" );
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int vala; float valb; Test( int a, float b = 3.5 ){ \nvala = a; valb = b; } } Test a = Test( 0 );");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int vala; float valb; Test( int a = 3.5, float b ){ \nvala = a; valb = b; } } Test a = Test( 0 );");
         
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
         
         exp = context.parseString( "class Test{ Test(){} int vala; float valb; Test( int a, float b ){ vala = a; valb = b; } } Test a = Test( 0, 1.5 );");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} class Test2{ Test2(){} int vala; float valb; Test( int a, float b = 3.5 ){ \nvala = a; valb = b; } } } Test::Test2 a = typename Test::Test2( 0 );");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} class Test2{ Test2(){} int vala; float valb; Test2( int a, float b = 3.5 ){ \nvala = a; valb = b; } } } Test::Test2 a = typename Test::Test2( 0 );");
         
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
         delete exp;
      }



      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ int vala; float valb; Test( int a, float b = 3.5 ){ \nvala = a; valb = b; } } Test a = Test();");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator>( float n, int nn); if ( 1.0 > 5){}");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import float operator>( float n, int nn); if ( 1.0 > 5){}");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[] = {1, 2};");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int n[] = {1, \"df\"};");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{Test(){}} Test n[] = {Test(), Test()};");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import string operator+( string n, string nn); void print( string s ){} string s = \"tralala\"; print(\"hahaha\" + s ); ");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test {Test(){} } Test t = NULL; ");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test {Test(){} } Test t = NULL + NULL; ");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int t = NULL; ");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n ){} } Test tests[ 10 ]; ");
         
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+(int n, int nn); class Test{ int n; void test(){ this.n = this.n + 1;}}");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+(int n, int nn); class Test{ int n; void test(){ this.n = this.this.n + 1;}}");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test( string str2 ){ string str;} Test( int n ){} Test( Test t ){} } Test str = Test(\"\");");
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
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(){} int tralala(){return 0;} float tralala(){return 0.0;} } Test t = Test(); t.tralala(); ");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+(int n, int nn); class Test{ int n; int this; void test(){ this.n = this.this.n + 1;}}");
         TESTER_ASSERT( !exp );
         delete exp;
      }


      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "NULL n = 0; ");
         
         TESTER_ASSERT( !exp );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{Test(){}} Test test;");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{} Test test;");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n){}} Test test;");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test(int n){}} Test test(5);");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );
         visitorType( *exp );

         VisitorPrint p( std::cout );

         p( *exp );
         std::cout << "exp=" << context.getError().getMessage().str() << std::endl;
         TESTER_ASSERT( !context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{int n; Test(){} int& ref(){ return n; } }");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int& ref;");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+( int n, int n2 ); int n; int n2; int& ref = n + n2;");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int operator+( int n, int n2 ); int n; int n2; int& ref = n; ref = n2;");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import int& operator+( int n, int n2 ); int n; int n2; int& ref = n+n2;");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import void test( int& a[3] );");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "import void test( int& a[3] ); int a[4]; test( a );");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test { class Test2{ Test2(){} int val; int& tt(){return val;} }} Test::Test2 test; int& t = test.tt();");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int& fun(int& a){return a;}");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int& fun(int& a){return 0;}");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int test( int n = 0, int b, float nn = 0 ){ return 0; }");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "int test( string& s ){ return \"haha\"; }");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "string test( string& s ){ string s; return s; }");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "string test( string& s ){ return s; } test(\"sdfsd\");");
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
         delete exp;
      }

      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "string tt; string test( string& s = tt ){ return s; } ");
         TESTER_ASSERT( exp );
         VisitorRegisterDeclarations visitor( context );
         visitor( *exp );
         TESTER_ASSERT( !context.getError().getStatus() );

         VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );
         visitorBind( *exp );
         TESTER_ASSERT( context.getError().getStatus() );
         delete exp;
      }


/*
      {
         ParserContext context;
         Ast* exp = 0;
         
         exp = context.parseString( "class Test{ Test( int a, float b ){ \nvala = a; valb = b; } int vala; float valb; } Test a;");
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
      }*/
      std::cout << "done" << std::endl;
   }
};

TESTER_TEST_SUITE(TestBasic);
/*
TESTER_TEST(testBinding2);
TESTER_TEST(testBinding1);
TESTER_TEST(testDummy2);
TESTER_TEST(testFull1);
TESTER_TEST(testSymbolTableDisctionary);

TESTER_TEST(testType1);

TESTER_TEST(testFull2);
*/
TESTER_TEST_SUITE_END();