#ifndef MVV_PARSER_TYPABLE_H_
# define MVV_PARSER_TYPABLE_H_

# include "type.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API Typable
   {
   public:
      Typable() : _type( 0 )
      {
      }

      virtual ~Typable()
      {
         delete _type;
      }

      void setNodeType( const Type* t )
      {
         ensure( t, "compiler error: can't evaluate the type of a tree node" );
         _type = t;
      }

      const Type* getNodeType() const
      {
         return _type;
      }

   protected:
      const Type*    _type;
   };
}
}

#endif