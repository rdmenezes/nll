#ifndef NLL_DATABASE_BUILDER_H_
# define NLL_DATABASE_BUILDER_H_

# include <nll/nll.h>
# include "config.h"

template <class Point>
typename nll::algorithm::Classifier<Point>::Database buildXorDatabase()
{
   //typedef typename nll::algorithm::Classifier<Point>::Database Database;
   typedef typename Point::value_type TYPE;

   typename nll::algorithm::Classifier<Point>::Database dat;
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 0), 0, nll::algorithm::Classifier<Point>::Database::Sample::LEARNING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 0), 1, nll::algorithm::Classifier<Point>::Database::Sample::LEARNING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 1), 1, nll::algorithm::Classifier<Point>::Database::Sample::LEARNING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 1), 0, nll::algorithm::Classifier<Point>::Database::Sample::LEARNING));

   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 0), 0, nll::algorithm::Classifier<Point>::Database::Sample::TESTING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 0), 1, nll::algorithm::Classifier<Point>::Database::Sample::TESTING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 1), 1, nll::algorithm::Classifier<Point>::Database::Sample::TESTING));
   dat.add(typename nll::algorithm::Classifier<Point>::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 1), 0, nll::algorithm::Classifier<Point>::Database::Sample::TESTING));

   return dat;
}

template <class T>
void _loadFileSpect( const std::string& file, int type, typename nll::algorithm::Classifier<T>::Database& dat )
{
   std::ifstream f( file.c_str() );
   ensure( f.is_open(), "can't find:" + file );

   double classification;
   while ( ! f.eof() )
   {
      std::string buf;
      std::getline(f, buf);
      std::vector<const char*> vals = nll::core::split( buf, ',' );
      ensure(vals.size() == 23, "error file");
      typedef typename nll::algorithm::Classifier<T>::Database DatabaseT;
      typename DatabaseT::Sample::Input i( 22 );
      for ( unsigned n = 0; n < 22; ++n )
         i[ n ] = ( typename DatabaseT::Sample::Input::value_type)atof( vals[ n + 1 ] );
      classification = atof( vals[ 0 ] );

      dat.add(typename DatabaseT::Sample(i, (nll::ui32)classification, static_cast<typename DatabaseT::Sample::Type>(type)));
   }
}

template <class T>
typename nll::algorithm::Classifier<T>::Database loadDatabaseSpect()
{
   typedef typename nll::algorithm::Classifier<T> ClassifierT;
   typename ClassifierT::Database dat;
   _loadFileSpect<T>( NLL_TEST_PATH "data/spect/SPECT.train", 1, dat );
   _loadFileSpect<T>( NLL_TEST_PATH "data/spect/SPECT.test", 0, dat );
   return dat;
}

inline nll::algorithm::Classifier< nll::core::Buffer1D<double> >::Database createSwissRoll()
{
   typedef nll::algorithm::Classifier< nll::core::Buffer1D<double> >::Database Database;

   Database dat;
   std::ifstream f( NLL_TEST_PATH "data/manifolds/SwissRollSmall.txt" );
   if ( !f.good() )
      throw std::runtime_error( "cannot locate the dataset" );

   while ( !f.eof() )
   {
      std::string line;
      std::getline( f, line );

      std::stringstream ss( line );
      Database::Sample::Input i( 3 );

      ss >> i[ 0 ] >> i[ 1 ] >> i[ 2 ];

      dat.add( Database::Sample( i, 0, Database::Sample::LEARNING ) );
   }

   return dat;
}

inline nll::algorithm::Classifier< nll::core::Buffer1D<double> >::Database sampleSwissRoll()
{
   typedef nll::algorithm::Classifier< nll::core::Buffer1D<double> >::Database Database;

   Database dat;
   
   const double max = 1.0;
   const double step = 0.02;
   const unsigned nbPoints = static_cast<unsigned>( max / step );


   for ( unsigned n = 0; n < nbPoints; ++n )
   {
      const double tt0 = ( 1 + 2 * n * step ) * 3 * nll::core::PI / 2;
      for ( unsigned nn = 0; nn < 50; ++nn )
      {
         Database::Sample s( nll::core::make_buffer1D<double>( tt0 * cos( tt0 ), nll::core::generateUniformDistribution( 0, 1 ) * 21, tt0 * sin( tt0 ) ), 0, Database::Sample::LEARNING );
         dat.add( s );
      }
   }

   return dat;
}


#endif
