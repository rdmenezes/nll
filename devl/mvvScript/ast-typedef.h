#ifndef MVV_PARSER_AST_TYPEDEF_H_
# define MVV_PARSER_AST_TYPEDEF_H_

# include "typable.h"
# include "ast.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstTypedef : public AstDecl
   {
   public:
      AstTypedef( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name ) : AstDecl( location, name ), _type( type )
      {
         _visited = false;
      }

      void setVisited()
      {
         _visited = true;
      }

      bool getVisited() const
      {
         return _visited;
      }

      virtual ~AstTypedef()
      {
         delete _type;
      }


      AstTypeT& getType()
      {
         return *_type;
      }

      const AstTypeT& getType() const
      {
         return *_type;
      }

      /// Accept a const visitor \a v.
      virtual void accept( ConstVisitor& v ) const
      {
         v( *this );
      }

      /// Accept a non-const visitor \a v.
      virtual void accept( Visitor& v )
      {
         v( *this );
      }

   private:
      AstTypedef& operator=( const AstTypedef& );
      AstTypedef( const AstTypedef& );

   protected:
      AstTypeT*            _type;
      bool                 _visited;   // used by the type visitor and recursive typedef...
   };
}
}

#endif