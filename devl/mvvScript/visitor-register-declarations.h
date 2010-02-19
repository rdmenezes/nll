#ifndef MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_
# define MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_

# include "visitor-default.h"
# include "parser-context.h"
# include "symbol-table.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor that will reference all global definitions:
           _global_ variable/ _global_ function/ all class declaration.

           It will report any redefinition errors.
    
           We need this as we need to parse first other files that might declare global variables,
           functions. Also the class life's scope is different from functions and variables as they
           need to remain in the look up table.
    */
   class VisitorRegisterDeclarations : public VisitorDefault
   {
   public:
      typedef std::set<mvv::Symbol> Symbols;
      using VisitorDefault::operator();

   public:
      VisitorRegisterDeclarations( ParserContext& context ) : _context( context )
      {}

      const Symbols& getFilesToInclude() const
      {
         return _filesToInclude;
      }

      const Symbols& getFilesToImport() const
      {
         return _filesToImport;
      }

      virtual void operator()( AstDeclClass& e )
      {
         // TODO scope
         operator()( e.getDeclarations() );
      }

      virtual void operator()( AstIf& )
      {
         // TODO scope handling
      }

      virtual void operator()( AstArgs& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstReturn& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstExpCall& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstExpTypename& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }


      virtual void operator()( AstDeclFun& e ) 
      {
         // discard type and body
         // if body is undeclared, multiple function definitions can be delcared

         // TODO must check it's arguments for overloading...
      }

      virtual void operator()( AstImport& e )
      {
         _filesToImport.insert( e.getStr() );
      }

      virtual void operator()( AstInclude& e )
      {
         _filesToInclude.insert( e.getStr() );
      }

      virtual void operator()( AstDeclVar& e )
      {
         
      }

      virtual void operator()( AstExpSeq& e )
      {
         // TODO scope handling
         operator()( e.getExp() );
      }


   private:
      // disabled
      VisitorRegisterDeclarations& operator=( const VisitorRegisterDeclarations& );
      VisitorRegisterDeclarations( const VisitorRegisterDeclarations& );

   private:
      ParserContext&    _context;
      Symbols           _filesToInclude;
      Symbols           _filesToImport;
   };
}
}

#endif