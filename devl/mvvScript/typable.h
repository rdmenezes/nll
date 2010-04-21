#ifndef MVV_PARSER_TYPABLE_H_
# define MVV_PARSER_TYPABLE_H_

# include "type.h"
# include "type-runtime.h"

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

      void setNodeType( Type* t )
      {
         ensure( t, "compiler error: can't evaluate the type of a tree node" );
         _type = t;
      }

      const Type* getNodeType() const
      {
         return _type;
      }

      Type* getNodeType()
      {
         return _type;
      }

   protected:
      Type*          _type;
   };
}
}

#endif