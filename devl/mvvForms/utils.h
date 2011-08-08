#ifndef MVV_FORMS_UTILS_H_
# define MVV_FORMS_UTILS_H_

#include <string>

namespace mvv
{
   inline std::wstring getWideString( const std::string& s )
   {
      std::wstring str2(s.length(), L' '); // Make room for characters

      // Copy string to wstring.
      std::copy(s.begin(), s.end(), str2.begin());
      return str2;
   }
}

#endif