#include "stdafx.h"
#include "visitor-translate.h"

using namespace mvv::parser;

int main( int argc, const char** argv)
{
   if ( argc != 2 )
   {
      std::cerr << "excepted: ./scriptTranslator fileToTranslate" << std::endl;
      std::cerr << " info: display the .ludo language in a closer form of C++ to be parsed by doxygen" << std::endl;
      return 1;
   }

   // init the parser
   ParserContext context; 
   Ast* exp = 0;

   // parse and convert file
   exp = context.parseFile( argv[ 1 ] );
   if ( !exp )
   {
      std::cerr << "error! cannot parse this file. Rzeason:" << context.getError().getMessage().str() << std::endl;
      return 1;
   }

   if ( std::string( argv[ 1 ] ).find("core.ludo") != std::string::npos )
   {
      std::cout << "/**" << std::endl;
      std::cout << "@mainpage Medical Volume Viewer" << std::endl;
      std::cout << "MVV is a customizable volume viewer/editor. It is based on simple & configurable building blocks," << std::endl;
      std::cout << "glued with a powerful script engine. The main principle is that any building block can be" << std::endl;
      std::cout << "queried & modified using scripts in an interactive manner. This is especially useful to" << std::endl;
      std::cout << "automate repetitive tasks (doing the same task on a lot of volumes, or working on the same" << std::endl;
      std::cout << "volume but having the viewer set in a particular configuration for example)." << std::endl;

      std::cout << "@note this documentation has been produced with a transductor from the script language to C++. Consequently Comments are not directly available from the doxygen. However, they are in the original file and can be easily looked up." << std::endl;

      std::cout << "@author Ludovic Sibille" << std::endl;
      std::cout << "@version " << MVV_VERSION << std::endl;
      std::cout << "*/" << std::endl;
   }

   std::ofstream f( "c:/tmp.txt", std::ios::out | std::ios::app );
   f << argv[ 1 ] << std::endl;

   VisitorTranslate translator( std::cout );
   translator( *exp );

   return 0;
}