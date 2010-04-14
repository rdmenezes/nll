#ifndef MVV_PARSE_EVALUATE_VISITOR_H_
# define MVV_PARSE_EVALUATE_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"

namespace mvv
{
namespace parser
{
   class RuntimeException : std::exception
   {
   public:
      RuntimeException( const char* c ) : std::exception( c )
      {}
   };

   namespace impl
   {
      inline void runtimeError( const YYLTYPE& loc, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << loc << msg << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::RUNTIME;

         throw RuntimeException( ss.str().c_str() );
      }
   }

   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    */
   class VisitorEvaluate : public Visitor
   {
   public:
      VisitorEvaluate( ParserContext& context,
                       SymbolTableVars& vars,
                       SymbolTableFuncs& funcs,
                       SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
      }

      virtual void operator()( AstInt& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::INT, e.getNodeType() );
         val.intval = e.getValue();
      }

      virtual void operator()( AstThis& )
      {
      }

      virtual void operator()( AstNil& )
      {
      }

      virtual void operator()( AstFloat& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::FLOAT, e.getNodeType() );
         val.floatval = e.getValue();
      }

      virtual void operator()( AstString& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::STRING, e.getNodeType() );
         val.stringval = e.getValue();
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
         operator()( e.getValue() );
         operator()( e.getLValue() );


      }

      virtual void operator()( AstVarSimple& e )
      {
         ensure( e.getReference(), "compiler error: can't find variable declaration" );
         e.getRuntimeValue() = e.getReference()->getRuntimeValue();
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getIndex() );
         operator()( e.getName() );

         ensure( e.getName().getRuntimeValue().type == RuntimeValue::ARRAY, "compiler error: must be an array" );
         int index = e.getIndex().getRuntimeValue().intval;

         if ( index < 0 || static_cast<unsigned int>( index ) >= (*e.getName().getRuntimeValue().vals).size() )
         {
            std::stringstream msg;
            msg << "out of bound index: array size=" << (*e.getName().getRuntimeValue().vals).size() << " index=" << index;
            impl::runtimeError( e.getIndex().getLocation(), _context, msg.str() );
         }

         e.getRuntimeValue() = (*e.getName().getRuntimeValue().vals)[ index ];
      }

      virtual void operator()( AstVarField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstType& )
      {
      }

      virtual void operator()( AstDecls& e )
      {
         for ( AstDecls::Decls::const_iterator it = e.getDecls().begin(); it != e.getDecls().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclVars& e )
      {
         for ( AstDeclVars::Decls::const_iterator it = e.getVars().begin(); it != e.getVars().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         operator()( e.getDeclarations() );
      }

      virtual void operator()( AstDeclFun& ) 
      {
         // the definition has been hadnled before...
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
            e.getFunction()->getRuntimeValue() = e.getReturnValue()->getRuntimeValue();
         }
      }

      virtual void operator()( AstImport& )
      {
         // nothing to do
      }

      virtual void operator()( AstInclude& )
      {
         // nothing to do
      }

      virtual void operator()( AstExpCall& e )
      {
         // first, evaluate the arguments
         operator()( e.getArgs() );

         
         // if the function is actually a member function, fetch the object
         ensure( e.getFunctionCall(), "compiler error: function call not set" );
         if ( e.getConstructed() )
         {
            // TODO e.getFunctionCall()->getRuntimeObjectSource() = sourceObject
         } else if ( e.getFunctionCall() )
         {
            // TODO
         } else {
            // TODO: only in debug mode
            e.getFunctionCall()->getRuntimeObjectSource().setType( RuntimeValue::EMPTY, 0 );
         }

         // fetch the arguments
         AstDeclVars::Decls::iterator itVar = e.getFunctionCall()->getVars().getVars().begin();
         AstArgs::Args::iterator itArg = e.getArgs().getArgs().begin();
         for ( ; itArg != e.getArgs().getArgs().end(); ++itVar, ++itArg )
         {
            (*itVar)->getRuntimeValue() = (*itArg)->getRuntimeValue();
         }
         while ( itVar != e.getFunctionCall()->getVars().getVars().end() )
         {
            // if we are not done, we must first evaluate the default value and fetch it to the argument
            ensure( (*itVar)->getInit(), "compiler error: there must be a default value!" );
            operator()( *(*itVar)->getInit() );
            (*itVar)->getRuntimeValue() = (*itVar)->getInit()->getRuntimeValue();

            ++itVar;
         }

         // call the function
         ensure( e.getFunctionCall()->getBody(), "compiler error: body function undeclared" );
         operator()( *e.getFunctionCall()->getBody() );

         // fetch the value
         e.getRuntimeValue() = e.getFunctionCall()->getRuntimeValue();
      }

      virtual void operator()( AstDeclVar& e )
      {
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            e.getRuntimeValue() = e.getInit()->getRuntimeValue();
            return;
         } else if ( e.getDeclarationList() )
         {
            operator()( *e.getDeclarationList() );

            // check we have an array
            ensure( e.getType().isArray(), "compiler error: current implementation: it must be an array" );

            // copy the value from the initialization list
            e.getRuntimeValue().setType( RuntimeValue::ARRAY, e.getNodeType() );
            e.getRuntimeValue().vals = platform::RefcountedTyped< RuntimeValues >( new RuntimeValues( e.getDeclarationList()->getArgs().size() ) );

            ui32 n = 0;
            for ( AstArgs::Args::iterator itArg = e.getDeclarationList()->getArgs().begin(); itArg != e.getDeclarationList()->getArgs().end(); ++itArg, ++n )
            {
               (*e.getRuntimeValue().vals)[ n ] = (*itArg)->getRuntimeValue();
            }
         }

         if ( e.getType().isArray() )
         {
            // TODO
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );
               }
            } 
         }

         if ( e.getObjectInitialization() )
         {
            // TODO
            operator()( *e.getObjectInitialization() );
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
      }

      virtual void operator()( AstTypeField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            operator()( e.getArgs() );
         } 
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

   private:
      // disabled copy
      VisitorEvaluate( const VisitorEvaluate& );
      VisitorEvaluate& operator=( const VisitorEvaluate& );


   private:
      ParserContext&       _context;
      SymbolTableVars&     _vars;
      SymbolTableFuncs&    _funcs;
      SymbolTableClasses&  _classes;
   };
}
}

#endif