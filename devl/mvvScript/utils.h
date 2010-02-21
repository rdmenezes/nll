#ifndef MVV_PARSER_UTILS_H_
# define MVV_PARSER_UTILS_H_

# include "ast-files.h"

namespace mvv
{
namespace parser
{
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
            it1 != t1->getVars().end(); ++t1, ++t2 )
      {
         if ( !areTypesEqual( &(*it1)->getType(), &(*it2)->getType() ) )
            return false;
      }

      return true;
   }
}
}

#endif
