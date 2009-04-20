#ifndef LOAD_DATABASE_H_
# define LOAD_DATABASE_H_

# include <nll/nll.h>
# include <iostream>
# include <fstream>
# include "database-benchmark.h"

# define PROBEN_PATH    "../../tutorial/data/proben1/"

namespace nll
{
namespace benchmark
{
   namespace impl
   {
      
      struct ProbenHeader
      {
         ui32 values[ 7 ];

         enum valuesType
         {
            I_BOOLEAN,
            I_REAL,
            O_BOOLEAN,
            O_REAL,
            NB_TRAINING,
            NB_VALIDATION,
            NB_TEST
         };

         void process( std::istream& i )
         {
            static const char* hd[] =
            {
               "bool_in",
               "real_in",
               "bool_out",
               "real_out",
               "training_examples",
               "validation_examples",
               "test_examples"
            };

            for ( ui32 n = 0; n < 7; ++n )
            {
               std::string line;
               std::getline( i, line );
               std::vector<const char*> sps = nll::core::split( line, '=' );
               ensure( sps.size() == 2, "expected 2 tokens only" );
               ensure( strcmp( sps[ 0 ], hd[ n ] ) == 0, "wrong header line:" + nll::core::val2str( n ) );

               values[ n ] = atoi( sps[ 1 ] );
            }
         }
      };
   }

   /**
    @brief Define how the input vector is encoded. Doesn't fill the field 'type'
           of the sample.
    */
   class Encoder
   {
   public:
      virtual BenchmarkDatabases::Database::Sample encode( const impl::ProbenHeader& header, const std::string& line ) = 0;
   };

   /**
    @brief Encode the header: from the input line to a sample of the database. Doesn't fill the field 'type'
           of the sample.
    */
   class EncoderClassification : public Encoder
   {
   public:
      virtual BenchmarkDatabases::Database::Sample encode( const impl::ProbenHeader& header, const std::string& line )
      {
         ui32 nbi = header.values[ impl::ProbenHeader::I_REAL ] +
                    header.values[ impl::ProbenHeader::I_BOOLEAN ];
         ui32 nbo = header.values[ impl::ProbenHeader::O_REAL ] +
                    header.values[ impl::ProbenHeader::O_BOOLEAN ];
         
         BenchmarkDatabases::Database::Sample sample;
         sample.input = BenchmarkDatabases::Database::Sample::Input( nbi );

         ensure( header.values[ impl::ProbenHeader::O_REAL ] == 0, "only classification database" );
         std::string l = line;
         std::vector<const char*> tokens = nll::core::split( l, ' ' );
         ensure( ( nbi + nbo ) == tokens.size(), "attribut missing" );
         for ( ui32 n = 0; n < nbi; ++n )
            sample.input[ n ] = atof( tokens[ n ] );
         ui32 sum = 0;
         ui32 indx = (ui32)-1;
         for ( ui32 n = nbi; n < nbi + nbo; ++n )
            if ( atof( tokens[ n ] ) > 0.5 )
            {
               ++sum;
               indx = n;
            }
         ensure( sum == 1, "only 1 class possible" );
         sample.output = indx - nbi;
         return sample;
      }
   };

   /**
    @brief Helper class to load the Proben1 database.

    The database can be downloaded here: ftp://ftp.ira.uka.de/pub/neuron/proben1.tar.gz
    Paper describing goals can be found here: citeseer.ist.psu.edu/275530.html or
                                              http://page.mi.fu-berlin.de/prechelt/Biblio/1994-21.pdf
    */
   class ProbenDatabase
   {
   public:
      typedef BenchmarkDatabases::Database   Database;
   
   public:
      static Database load( const std::string& datfile, Encoder* encoder )
      {
         std::ifstream i( datfile.c_str() );
         ensure( i.is_open(), "can't find:" + datfile );

         Database dat;
         impl::ProbenHeader hd;
         hd.process( i ) ;

         ui32 n = 0;
         std::string line;
         std::getline( i, line );
         while ( !i.eof() )
         {
            Database::Sample sample = encoder->encode( hd, line );
            sample.debug = nll::core::make_buffer1D_from_string( datfile + ":" + nll::core::val2str( n ) );
            sample.type = ( n < hd.values[ impl::ProbenHeader::NB_TRAINING ] ) ? Database::Sample::LEARNING :
                          ( n < ( hd.values[ impl::ProbenHeader::NB_VALIDATION ] + hd.values[ impl::ProbenHeader::NB_TRAINING ] ) ) ?
                              Database::Sample::VALIDATION : Database::Sample::TESTING;
            dat.add( sample );

            ++n;
            std::getline( i, line );
         }
         std::cout << "proben size=" << dat.size() << std::endl;
         return dat;
      }
   };
}
}

#endif
