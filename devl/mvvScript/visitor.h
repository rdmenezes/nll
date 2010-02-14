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
   class AstBinOp;
   class AstIf;
   class AstStatements;

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
   };

   /// defines a visitor that is constant
   typedef GenVisitor<nll::core::Constify> ConstVisitor;

   /// defines a non const visitor
   typedef GenVisitor<nll::core::Unconstify> Visitor;
}
}

#endif