#ifndef MVV_PARSER_UTILS_H_
# define MVV_PARSER_UTILS_H_

# include "ast-files.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Returns the member function of a class
    */
   static std::vector<AstDeclFun*> getFunctionsFromClass( AstDeclClass& c, const mvv::Symbol& s )
   {
      std::vector<AstDeclFun*> res;
      for ( AstDecls::Decls::iterator it = c.getDeclarations().getDecls().begin();
            it != c.getDeclarations().getDecls().end();
            ++it )
      {
         AstDeclFun* fn = dynamic_cast<AstDeclFun*>( *it );
         if ( (*it)->getName() == s && fn )
         {
            res.push_back( fn );
         }
      }
      return res;
   }

   namespace impl
   {
      inline mvv::Symbol toSymbol( AstOpBin::Op op )
      {
         switch ( op )
         {
         case AstOpBin::GE:
            return mvv::Symbol::create( "operator>=" );
         case AstOpBin::LE:
            return mvv::Symbol::create( "operator<=" );
         case AstOpBin::NE:
            return mvv::Symbol::create( "operator!=" );
         case AstOpBin::MINUS:
            return mvv::Symbol::create( "operator-" );
         case AstOpBin::PLUS:
            return mvv::Symbol::create( "operator+" );
         case AstOpBin::TIMES:
            return mvv::Symbol::create( "operator*" );
         case AstOpBin::DIVIDE:
            return mvv::Symbol::create( "operator/" );
         case AstOpBin::EQ:
            return mvv::Symbol::create( "operator==" );
         case AstOpBin::GT:
            return mvv::Symbol::create( "operator>" );
         case AstOpBin::LT:
            return mvv::Symbol::create( "operator<" );
         case AstOpBin::AND:
            return mvv::Symbol::create( "operator&&" );
         case AstOpBin::OR:
            return mvv::Symbol::create( "operator||" );
         case AstOpBin::STREAMO:
            return mvv::Symbol::create( "operator<<" );
         case AstOpBin::STREAMI:
            return mvv::Symbol::create( "operator>>" );
         };
         ensure( 0, "unreachable, or missing a case..." );
      }

      inline void reportAlreadyDeclaredType( const YYLTYPE& previous, const YYLTYPE& current, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << current << msg <<" (see " << previous << ")" << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::BIND;
      }

      inline void reportError( const YYLTYPE& current, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << current << msg << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::BIND;
      }

      inline void reportTypeError( const YYLTYPE& current, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << current << msg << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::TYPE;
      }

      inline void reportTypeMultipleCallableFunction( const std::vector<AstDeclFun*>& fns, mvv::parser::ParserContext& context )
      {
         std::stringstream ss;
         ss << "ambiguous function call. See possible functions:" << std::endl;

         for ( size_t n = 0; n < fns.size(); ++n )
         {
            ss << fns[ n ]->getLocation();
            if ( ( n + 1 ) !=  fns.size() )
            {
               ss << ", ";
            }
         }
         context.getError() << ss.str() << mvv::parser::Error::TYPE;
      }
   }

   inline bool areTypesEqual( const AstTypeT* t1, const AstTypeT* t2 )
   {
      ensure( t1 && t2, "can be null" );

      if ( t1 == t2 )
         return true;
      if ( t1->isArray() != t2->isArray() )
         return false;
      if ( t1->isArray() && t1->getSize()->size() != t2->getSize()->size() )
         return false;
      const AstTypeField* t1field = dynamic_cast<const AstTypeField*>( t1 );
      const AstTypeField* t2field = dynamic_cast<const AstTypeField*>( t2 );
      if ( t1field && !t2field || !t2field && t1field )
         return false;
      if ( t1field )
      {
         if ( t1field->getName() != t2field->getName() )
            return false;
         return areTypesEqual( &t1field->getField(), &t2field->getField() );
      }

      const AstType* t1type = dynamic_cast<const AstType*>( t1 );
      const AstType* t2type = dynamic_cast<const AstType*>( t2 );
      if ( t1type && !t2type || !t2type && t1type )
         return false;
      ensure( t1type, "only possible case, else new types have not been referenced" );
      if ( t1type->getType() != t2type->getType() ||
           t1type->getSymbol() != t2type->getSymbol() )
      {
         return false;
      }

      return true;
   }

   inline bool areDeclVarsEqual( const AstDeclVars* t1, const AstDeclVars* t2 )
   {
      ensure( t1 && t2, "can be null" );

      if ( t1->getVars().size() != t2->getVars().size() )
         return false;
      for ( AstDeclVars::Decls::const_iterator it1 = t1->getVars().begin(),
                                               it2 = t2->getVars().begin();
            it1 != t1->getVars().end(); ++it1, ++it2 )
      {
         if ( !areTypesEqual( &(*it1)->getType(), &(*it2)->getType() ) )
            return false;
      }

      return true;
   }   

   inline Type* createTypeFromFunction( AstDeclFun* f, TypeNamed* classDecl = 0 )
   {
      std::vector<Type*> types( f->getVars().getVars().size() );
      for ( ui32 n = 0; n < types.size(); ++n )
         types[ n ] = f->getVars().getVars()[ n ]->getNodeType();
      return new TypeFunctionPointer( false, f->getNodeType(), types, classDecl );
   }
}
}

#endif
