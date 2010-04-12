#ifndef MVV_PARSER_TYPE_RUNTIME_H_
# define MVV_PARSER_TYPE_RUNTIME_H_

namespace mvv
{
namespace parser
{
   class RuntimeType
   {
   public:
      RuntimeType( TypeEnum t, Type* v ) : type( t ), val( v )
      {
      }

      enum TypeEnum
      {
         INT,
         FLOAT,
         STRING,
         TYPE,
         ARRAY
      };

      TypeEnum    type;
      Type*       val;
   };
}
}

#endif