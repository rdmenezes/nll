#ifndef MVV_SCRIPT_VISITOR_H_
# define MVV_SCRIPT_VISITOR_H_

# include <nll/nll.h>

namespace mvv
{
namespace parser
{
   class Ast;
   class AstInt;
   class AstFloat;
   class AstString;
   class AstOpBin;
   class AstIf;
   class AstStatements;
   class AstExp;
   class AstExpAssign;
   class AstExpCall;
   class AstVar;
   class AstVarSimple;
   class AstVarArray;
   class AstVarField;
   class AstType;
   class AstDecl;
   class AstDeclVar;
   class AstDecls;
   class AstDeclVars;
   class AstDeclClass;
   class AstDeclFun;
   class AstArgs;
   class AstReturn;
   class AstImport;
   class AstInclude;
   //class AstDeclarationList;

   /**
    @brief root of all the visitors
    */
   template < template <class> class Const> 
   class GenVisitor
   {
   public:
      virtual ~GenVisitor()
      {
      }

      virtual void operator()( typename Const<Ast>::type& e )
      {
         e.accept( *this );
      }

      virtual void operator()( typename Const<AstInt>::type& e ) = 0;
      virtual void operator()( typename Const<AstFloat>::type& e ) = 0;
      virtual void operator()( typename Const<AstString>::type& e ) = 0;
      virtual void operator()( typename Const<AstOpBin>::type& e ) = 0;
      virtual void operator()( typename Const<AstIf>::type& e ) = 0;
      virtual void operator()( typename Const<AstStatements>::type& e ) = 0;
      virtual void operator()( typename Const<AstExpAssign>::type& e ) = 0;
      virtual void operator()( typename Const<AstVarSimple>::type& e ) = 0;
      virtual void operator()( typename Const<AstVarArray>::type& e ) = 0;
      virtual void operator()( typename Const<AstVarField>::type& e ) = 0;
      virtual void operator()( typename Const<AstType>::type& e ) = 0;
      virtual void operator()( typename Const<AstDeclVar>::type& e ) = 0;
      virtual void operator()( typename Const<AstDecls>::type& e ) = 0;
      virtual void operator()( typename Const<AstDeclVars>::type& e ) = 0;
      virtual void operator()( typename Const<AstDeclClass>::type& e ) = 0;
      virtual void operator()( typename Const<AstDeclFun>::type& e ) = 0;
      virtual void operator()( typename Const<AstArgs>::type& e ) = 0;
      virtual void operator()( typename Const<AstReturn>::type& e ) = 0;
      virtual void operator()( typename Const<AstImport>::type& e ) = 0;
      virtual void operator()( typename Const<AstInclude>::type& e ) = 0;
      virtual void operator()( typename Const<AstExpCall>::type& e ) = 0;
   };

   /// defines a visitor that is constant
   typedef GenVisitor<nll::core::Constify> ConstVisitor;

   /// defines a non const visitor
   typedef GenVisitor<nll::core::Unconstify> Visitor;
}
}

#endif