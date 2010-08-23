#ifndef MVV_PARSER_COMPILER_DUMMY_INTERFACE_H_
# define MVV_PARSER_COMPILER_DUMMY_INTERFACE_H_

# include <mvvPlatform/symbol.h>

namespace mvv
{
namespace parser
{
   class InterpreterRuntimeInterface
   {
   public:
      virtual std::ostream& getStdOut() const = 0;
      virtual void setStdOut( std::ostream* s ) = 0;
      virtual bool interpret( const std::string& cmd ) = 0;
      virtual std::string getVariableText( const mvv::Symbol& s ) const = 0;
      virtual const std::string& getLastErrorMesage() const = 0;
      virtual void clearError() = 0;
   };
}
}

#endif