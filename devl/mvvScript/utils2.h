#ifndef MVV_PARSER_UTILS2_H_
# define MVV_PARSER_UTILS2_H_

namespace mvv
{
namespace parser
{
   /**
    @brief Check is a symbol starts by *match*
    */
   inline bool isMatch( const mvv::Symbol& symbol, const std::string& match )
   {
      for ( ui32 n = 0; n < match.size(); ++n )
      {
         char val = symbol.getName()[ n ];
         if ( val == 0 || val != match[ n ] )
            return false;
      }
      return true;
   }
}
}

#endif