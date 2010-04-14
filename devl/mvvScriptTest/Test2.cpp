#include <tester/register.h>

#include <mvvScript/compiler.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"

#define EVAL(str)\
   {\
      ParserContext       context;\
      SymbolTableVars     vars;\
      SymbolTableFuncs    funcs;\
      SymbolTableClasses  classes;\
      Ast* exp = 0;\
      exp = context.parseString( str );\
      TESTER_ASSERT( exp );\
      VisitorRegisterDeclarations visitor( context );\
      visitor( *exp );\
      TESTER_ASSERT( !context.getError().getStatus() );\
      VisitorBind visitorBind( context, visitor.getVars(), visitor.getFuncs(), visitor.getClasses() );\
      visitorBind( *exp );\
      TESTER_ASSERT( !context.getError().getStatus() );\
      VisitorType visitorType( context, visitorBind.getVars(), visitorBind.getFuncs(), visitorBind.getClasses() );\
      visitorType( *exp );\
      vars = visitorType.getVars();\
      funcs = visitorType.getFuncs();\
      classes = visitorType.getClasses();\
      TESTER_ASSERT( !context.getError().getStatus() );\
      VisitorEvaluate visitorEval( context, vars, funcs, classes );\
      visitorEval( *exp );

#define EVAL_END  \
       delete exp;\
   }


struct TestEval
{
   void eval1()
   {
      /*
      EVAL( "int n = 5;" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::INT );
      TESTER_ASSERT( val->getRuntimeValue().intval == 5 );
      EVAL_END

      EVAL( "float n = 5.5;" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::FLOAT );
      TESTER_ASSERT( val->getRuntimeValue().floatval == 5.5 );
      EVAL_END

      EVAL( "string n = \"123456\";" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::STRING );
      TESTER_ASSERT( val->getRuntimeValue().stringval == "123456" );
      EVAL_END;
      
      EVAL( "int test( int n ){return n;} int n = test(5);" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::INT );
      TESTER_ASSERT( val->getRuntimeValue().intval == 5 );
      EVAL_END;

      EVAL( "int test( int n, int nn = 11 ){return nn;} int n = test(5);" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::INT );
      TESTER_ASSERT( val->getRuntimeValue().intval == 11 );
      EVAL_END;
   
      EVAL( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 3 ];" );
      TESTER_ASSERT( !context.getError().getStatus() );
      AstDeclVar* val = vars.find( mvv::Symbol::create("n" ) );
      TESTER_ASSERT( val );
      TESTER_ASSERT( val->getRuntimeValue().type == RuntimeValue::ARRAY );

      AstDeclVar* val2 = vars.find( mvv::Symbol::create("nn" ) );
      TESTER_ASSERT( val2 );
      TESTER_ASSERT( val2->getRuntimeValue().type == RuntimeValue::INT );
      TESTER_ASSERT( val2->getRuntimeValue().intval == 7 );
      EVAL_END;

      try
      {
         EVAL( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 13 ];" );
         EVAL_END;
         TESTER_ASSERT( 0 );
      } catch ( RuntimeException e )
      {
         TESTER_ASSERT( 1 );
      }

      */

      {
         CompilerFrontEnd fe;
         try
         {
            fe.run( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 13 ];" );
            TESTER_ASSERT( 0 );
         } catch( RuntimeException e )
         {
         }
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int nn1 = 8; int nn2 = nn1; int n; n = nn2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( " int test = 5;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "test" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }


      /*
      // TODO activate
      try
      {
         EVAL( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ -13 ];" );
         EVAL_END;
         TESTER_ASSERT( 0 );
      } catch ( RuntimeException e )
      {
      }
      */
   }


};

TESTER_TEST_SUITE(TestEval);
TESTER_TEST(eval1);
TESTER_TEST_SUITE_END();