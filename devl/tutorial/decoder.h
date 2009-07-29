#ifndef NLL_TUTORIAL_DECODER_H_
# define NLL_TUTORIAL_DECODER_H_

# include <vector>
# include <iostream>
# include <string>
# include <nll/nll.h>

namespace nll
{
namespace tutorial
{
   namespace impl
   {
      /**
       Defines a serie of decoder/encoder classes
       */
      class Encoder
      {
      public:
         virtual std::vector<double> encode( const std::string& s ) const = 0;
      };

      class AttributEncoder : public Encoder
      {};

      class ClassEncoder : public Encoder
      {};

      class AttributFloatingEncoder : public AttributEncoder
      {
      public:
         virtual std::vector<double> encode( const std::string& s ) const
         {
            double val = atof( s.c_str() );
            return core::make_vector<double>( val );
         }
      };

      class AttributDiscardEncoder : public AttributEncoder
      {
      public:
         virtual std::vector<double> encode( const std::string& ) const
         {
            return std::vector<double>();
         }
      };

      class AttributDiscreteEncoder : public AttributEncoder
      {
      public:
         AttributDiscreteEncoder( const std::set<std::string>& values ) : _possibleValues( values )
         {
         }
         virtual std::vector<double> encode( const std::string& s ) const
         {
            ui32 n = 0;
            for ( std::set<std::string>::const_iterator it = _possibleValues.begin();
                  it != _possibleValues.end(); ++it, ++n )
            {
               if ( std::string( *it ) == s )
                  return nll::core::make_vector<double>( (double)n / _possibleValues.size() );
            }
            unreachable( "error" );
            return nll::core::make_vector<double>( -1000 );
         }
      protected:
         std::set<std::string>   _possibleValues;
      };

      class ClassEncoder1ByClass : public ClassEncoder
      {
      public:
         ClassEncoder1ByClass( const std::set<std::string>& values ) : _possibleValues( values )
         {
         }

         virtual std::vector<double> encode( const std::string& s ) const
         {
            std::vector<double> val( _possibleValues.size() );
            ui32 n = 0;
            for ( std::set<std::string>::const_iterator it = _possibleValues.begin();
                  it != _possibleValues.end(); ++it, ++n )
            {
               if ( *it == s )
               {
                  val[ n ] = 1;
                  return val;
               }
            }
            unreachable( "error" );
            val[ 0 ] = -1000;
            return val;
         }

      private:
         std::set<std::string> _possibleValues;
      };

      class ClassEncoderNumber : public ClassEncoder
      {
      public:
         ClassEncoderNumber( const std::set<std::string>& values ) : _possibleValues( values )
         {
         }

         virtual std::vector<double> encode( const std::string& s ) const
         {
            std::vector<double> val( 1 );
            ui32 n = 0;
            for ( std::set<std::string>::const_iterator it = _possibleValues.begin();
                  it != _possibleValues.end(); ++it, ++n )
            {
               if ( *it == s )
               {
                  val[ 0 ] = n;
                  return val;
               }
            }
            unreachable( "error" );
            val[ 0 ] = -1000;
            return val;
         }

      private:
         std::set<std::string> _possibleValues;
      };

      typedef std::vector<const Encoder*> Encoders;

      std::pair< std::vector<double>, std::vector<double> >
      inline encodeSample( const Encoders& encoders, const std::vector<const char*>& vals )
      {
         ensure( encoders.size() == vals.size(), "size doesn't match" );
         std::vector<double> encodedInput;
         encodedInput.reserve( 100 );
         std::vector<double> encodedOutput;
         encodedOutput.reserve( 100 );
         ui32 nnInput = 0;
         ui32 nnOutput = 0;
         for ( ui32 n = 0; n < vals.size(); ++n )
         {
            if ( dynamic_cast<const AttributEncoder*>( encoders[ n ] ) )
            {
               std::vector<double> tmp = encoders[ n ]->encode( std::string( vals[ n ] ) );
               for ( ui32 t = 0; t < tmp.size(); ++t, ++nnInput )
                  encodedInput.push_back( tmp[ t ] );
            } else {
               std::vector<double> tmp = encoders[ n ]->encode( std::string( vals[ n ] ) );
               for ( ui32 t = 0; t < tmp.size(); ++t, ++nnOutput )
                  encodedOutput.push_back( tmp[ t ] );
            }
         }
         return std::make_pair( encodedInput, encodedOutput );
      }
   }

   class DatabaseDecoderEncoder
   {
   public:
      typedef core::Buffer1D<double>                  Point;
      typedef algorithm::Classifier<Point>::Database  Database;
      enum AttributValue
      {
         CLASS,      // will create for N classes, N 0->1 outputs, parsed attribut will represent an output value
         DISCARD,    // discard the attribut
         FLOATING,   // convert the value to a double
         DISCRETE    // assume it is a discrete value, it is not assumed any order relationship inside
      };
      typedef std::vector<AttributValue> AttributValues;

   public:
      Database compute( const AttributValues& values, std::istream& i )
      {
         std::vector<const impl::Encoder*> encoders( values.size() );
         std::vector< std::set<std::string> > sets( values.size() );

         // preread the database so we can compute statistics for discrete/class data
         while ( !i.eof() )
         {
            std::string line;
            std::getline( i, line );
            if ( line.size() )
            {
               std::vector<const char*> vals = core::split( line );
               ensure( vals.size() == values.size(), "Error: size doesn't match" );
               for ( ui32 n = 0; n < values.size(); ++n )
                  if ( values[ n ] == CLASS || values[ n ] == DISCRETE )
                     sets[ n ].insert( std::string( vals[ n ] ) );
            }
         }

         // create the encoders/decoders
         for ( ui32 n = 0; n < values.size(); ++n )
            switch( values[ n ] )
            {
            case CLASS:
               encoders[ n ] = new impl::ClassEncoderNumber( sets[ n ] );
               break;
            case DISCARD:
               encoders[ n ] = new impl::AttributDiscardEncoder();
               break;
            case FLOATING:
               encoders[ n ] = new impl::AttributFloatingEncoder();
               break;
            case DISCRETE:
               encoders[ n ] = new impl::AttributDiscreteEncoder( sets[ n ] );
               break;
            default:
               unreachable( "cae not handled" );
            }
         Database dat;
         
         // finally create the database
         i.clear();
         i.seekg( 0, std::ios::beg );
         ui32 lineNb = 0;
         while ( !i.eof() )
         {
            std::string line;
            std::getline( i, line );
            if ( line.size() )
            {
               std::vector<const char*> vals = core::split( line );
               std::pair< std::vector<double>,
                          std::vector<double> > point = impl::encodeSample( encoders, vals );

               Database::Sample::Input i;
               core::convert( point.first, i, static_cast<ui32>( point.first.size() ) );

               Database::Sample::Output o;
               ensure( point.second.size() == 1, "error: only one attribut must be a class attribut" );
               o = static_cast<Database::Sample::Output>( point.second[ 0 ] );

               core::Buffer1D<i8> debug = core::make_buffer1D_from_string( "sample:" + core::val2str( lineNb ) );
               dat.add( Database::Sample( i, o, Database::Sample::LEARNING, debug ) );
               ++lineNb;
            }
         }
         return dat;
      }
   };
}
}

#endif