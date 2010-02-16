#ifndef MVV_PARSE_DEFAULT_VISITOR_H_
# define MVV_PARSE_DEFAULT_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    */
   class VisitorDefault : public Visitor
   {
   public:
      virtual void operator()( AstInt& )
      {
      }

      virtual void operator()( AstFloat& )
      {
      }

      virtual void operator()( AstString& )
      {
      }

      virtual void operator()( AstOpBin& e )
      {
         operator()( e.getLeft() );
         operator()( e.getRight() );
      }

      virtual void operator()( AstIf& e )
      {
         operator()( e.getCondition() );
         operator()( e.getThen() );

         if ( e.getElse() )
         {
            operator()( *e.getElse() );
         }
      }

      virtual void operator()( AstStatements& e )
      {
         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstExpAssign& e )
      {
         operator()( e.getLValue() );
         operator()( e.getValue() );
      }

      virtual void operator()( AstVarSimple& )
      {
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getName() );
         operator()( e.getIndex() );
      }

      virtual void operator()( AstVarField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstType& )
      {
      }

      virtual void operator()( AstDeclVar& e )
      {
         operator()( e.getType() );
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
         }
      }

      virtual void operator()( AstDecls& e )
      {
         for ( AstDecls::Decls::const_iterator it = e.getDecls().begin(); it != e.getDecls().end(); ++it)
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclVars& e )
      {
         ui32 n = 0;
         for ( AstDeclVars::Decls::const_iterator it = e.getVars().begin(); it != e.getVars().end(); ++it, ++n )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         operator()( e.getDeclarations() );
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         operator()( e.getType() );
         if ( e.getBody() )
         {
            operator()( *e.getBody() );
         }
      }

      virtual void operator()( AstArgs& e )
      {
         for ( AstArgs::Args::const_iterator it = e.getArgs().begin(); it != e.getArgs().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstReturn& e )
      {
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
         }
      }

      virtual void operator()( AstImport& )
      {
      }

      virtual void operator()( AstInclude& )
      {
      }

      virtual void operator()( AstExpCall& e )
      {
         operator()( e.getName() );
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }
   };
}
}

#endif