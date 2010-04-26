#ifndef MVV_PARSER_AST_ARGS_H_
# define MVV_PARSER_AST_ARGS_H_

# include "ast-args.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstArgs : public Ast
   {
   public:
      typedef std::deque<AstExp*>   Args;

      AstArgs( const YYLTYPE& location, bool toDeallocate = true ) : Ast( location ), _toDeallocate( toDeallocate )
      {
      }

      ~AstArgs()
      {
         if ( _toDeallocate )
         {
            for ( Args::iterator i = _args.begin(); i != _args.end(); ++i )
               delete *i;
         }
      }

      void insert( AstExp* ast )
      {
         ensure( ast, "can't be null" );
         _args.push_front( ast );   // we usually push_front because in the scanner the list is recursively build from last to first, so invert it!
      }

      void insert_back( AstExp* ast )
      {
         ensure( ast, "can't be null" );
         _args.push_back( ast );
      }

      const Args& getArgs() const
      {
         return _args;
      }

      Args& getArgs()
      {
         return _args;
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
      Args    _args;
      bool    _toDeallocate;
   };
}
}

#endif