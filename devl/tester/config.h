#ifndef NLL_CONFIG_H_
# define NLL_CONFIG_H_

# include <map>
# include <iostream>
# include <fstream>
# include <vector>
# include <nll/nll.h>

namespace nll
{
namespace tester
{
   /**
    @brief Save/Load/Modify a simple configuration file for regression testing
    */
   class Config
   {
   public:
      typedef std::map<std::string, std::string>   Directory;
      typedef std::map<std::string, Directory>     Storage;

   public:
      Config()
      {
      }

      Config( std::istream& i )
      {
         read( i );
      }

      Config( const std::string& i )
      {
         read( i );
      }

      const std::string& getDirectory() const
      {
         return _dir;
      }

      void setDirectory( const std::string& dir )
      {
         _dir = dir;
      }

      std::string& operator[]( const std::string& s )
      {
         return _storage[ _dir ][ s ];
      }

      void write( const std::string& file ) const
      {
         std::ofstream f( file.c_str() );
         if ( !f.is_open() )
            return;
         write( f );
      }

      void write( std::ostream& o ) const
      {
         for ( Storage::const_iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            o << "[" << it->first << "]" << std::endl;
            for ( Directory::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 )
            {
               o << it2->first << "=" << it2->second << std::endl;
            }
            o << std::endl;
         }
      }

      void read( const std::string& file )
      {
         std::ifstream f( file.c_str() );
         if ( !f.is_open() )
            return;
         read( f );
      }

      void read( std::istream& i )
      {
         _storage.clear();
         std::string line;
         std::string directory = "empty";

         std::getline( i, line );
         while ( !i.eof() )
         {
            size_t posStart = line.find_first_of( '[' );
            size_t posEnd = line.find_last_of( ']' );
            if ( posStart != std::string::npos && posEnd != std::string::npos )
            {
               directory = std::string( &line[ posStart + 1 ], &line[ posEnd ] );
            } else {
               std::vector<const char*> args = nll::core::split( line, '=' );
               if ( args.size() == 2 )
               {
                  _storage[ directory ][ args[ 0 ] ] = args[ 1 ];
               }
            }
            std::getline( i, line );
         }
      }

   public:
      Storage      _storage;

   private:
      std::string  _dir;
   };
}
}

#endif
