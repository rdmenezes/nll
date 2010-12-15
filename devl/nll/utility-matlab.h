#ifndef NLL_CORE_UTILITY_MATLAB_H_
# define NLL_CORE_UTILITY_MATLAB_H_

namespace nll
{
namespace core
{
   /**
    @brief Utility to export a vector of vectors as a matlab Matrix file
    */
   template <class Vectors>
   void exportVectorToMatlabAsRow( const Vectors& v, const std::string& file )
   {
      std::ofstream f( file.c_str() );
      if ( !f.good() )
         throw std::runtime_error( "Problem!" );

      f << "[";
      for ( ui32 n = 0; n < v.size(); ++n )
      {
         for ( ui32 nn = 0; nn < v[ n ].size(); ++nn )
            f << v[ n ][ nn ] << " ";
         f << "; ";
      }

      f << "]" << std::endl;
   }

   /**
    @brief Utility to import a matlab Matrix file (exported with dlwrite) as a vector of vectors
    */
   template <class Vectors>
   Vectors readVectorFromMatlabAsColumn( const std::string& file )
   {
      Vectors v;

      std::vector< std::vector< double > > vectors;
      std::ifstream f( file.c_str() );
      if ( !f.good() )
         throw std::runtime_error( "Problem!" );

      while ( !f.eof() )
      {
         std::string line;
         std::getline( f, line );
         std::vector<const char*> splits = core::split( line, ',' );
         
         std::vector<double> record( splits.size() );
         for ( ui32 n = 0; n < splits.size(); ++n )
            record[ n ] = core::str2val<double>( splits[ n ] );
         if ( record.size() )
            vectors.push_back( record );
      }

      typedef typename Vectors::value_type Vector;

      v = Vectors( vectors.size() );
      for ( ui32 n = 0; n < vectors.size(); ++n )
      {
         Vector vv( vectors[ n ].size() );
         for ( ui32 nn = 0; nn < vectors[ n ].size(); ++nn )
         {
            vv[ nn ] = vectors[ n ][ nn ];
         }
         v[ n ] = vv;
      }

      return v;
   }
}
}

#endif
