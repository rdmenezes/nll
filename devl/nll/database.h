#ifndef NLL_DATABASE_H_
# define NLL_DATABASE_H_

# include <vector>
# include <set>
# include "buffer1D.h"
# include "io.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a sample for a classification database.
    @sa Database
    */
   template <class TInput, class TOutput>
   struct ClassificationSample
   {
      /// define the type of the sample (use from the database to know what kind of sample it is)
      typedef ClassificationSample TypeSample;

      /// define the input of a sample
      typedef TInput           Input;

      /// define the output of a sample
      typedef TOutput          Output;

      /// define string of a sample
      typedef Buffer1D<i8>    String;

      /// define the type of a sample
      enum Type
      {
              TESTING,     ///< testing sample. Used by the classifier for learning purpose only
              LEARNING,    ///< learning sample. Used by the classifier for learning purpose only
              VALIDATION   ///< validation sample Used by the classifier and preprocessing to find the best parameters' model.
      };

      Input       input;
      Output      output;
      Type        type;
      String      debug;

      /**
       @brief build a classification sample initialized.
       */
      ClassificationSample( const Input& i, const Output& o, const Type t, const String d = make_buffer1D_from_string("noname") ) : input( i ), output( o ), type( t ), debug( d )
      {}

      /**
       @brief build a classification sample uninitialized.
       */
      ClassificationSample()
      {}

      /**
       @brief read a sample from a stream.
       */
      void read( std::istream& i )
      {
         nll::core::read<Input>( input, i );
         nll::core::read<Output>( output, i );
         int t = static_cast<int>( type );
         nll::core::read<int>( t, i );
         type = static_cast<Type>( t );
         nll::core::read<String>( debug, i );
      }

      /**
       @brief write a sample to a stream.
       */
      void write( std::ostream& o ) const
      {
         nll::core::write<Input>( input, o );
         nll::core::write<Output>( output, o );
         int t = static_cast<int>( type );
         nll::core::write<int>( t, o );
         nll::core::write<String>( debug, o );
      }

      /**
       @brief test if semantically 2 samples are equal
       */
      bool operator==( const ClassificationSample& s ) const
      {
         return input == s.input && output == s.output && type == s.type && debug == s.debug;
      }
   };

   /**
    @ingroup core
    @brief Define a generic database, simply a list of samples.
    @sa Classifier
    */
   template <class SampleType>
   class Database
   {
      typedef std::vector<SampleType>  Container;

   public:
      typedef SampleType               Sample;

   public:
      /**
       @brief add a sample to the database.
       */
      void add( const SampleType& s )
      {
         _container.push_back( s );
      }

      /**
       @brief write the database to a stream.
       */
      void write( const std::string& file ) const
      {
         std::ofstream f( file.c_str(), std::ios_base::binary );
         if ( f.is_open() )
            write( f );
      }

      /**
       @brief write the database to a stream.
       */
      void write( std::ostream& o ) const
      {
         ui32 size = static_cast<ui32> ( _container.size() );
         nll::core::write<ui32>( size, o );
         for (ui32 n = 0; n < size; ++n)
            nll::core::write<SampleType>( _container[n], o );
      }

      /**
       @brief read the database from a stream.
       */
      void read( std::istream& i )
      {
         ui32 size = 0;
         nll::core::read<ui32>( size, i );

         _container = Container( size );
         for (ui32 n = 0; n < size; ++n)
            nll::core::read<SampleType>( _container[n], i );
      }

      /**
       @brief read the database from a stream.
       */
      void read( const std::string& file )
      {
         std::ifstream f( file.c_str(), std::ios_base::binary );
         if ( f.is_open() )
            read( f );
      }

      /**
       @brief clear the database.
       */
      inline void clear()
      {
         _container.clear();
      }

      /**
       @brief test if semantically 2 databases are equal.
       */
      bool operator==( const Database& dat ) const
      {
         if ( dat.size() != size() )
            return false;
         for ( ui32 n = 0; n < size(); ++n )
            if ( !( _container[ n ] == dat[ n ] ) )
               return false;
         return true;
      }
      
      /**
       @brief return the i th sample.
       */
      inline const SampleType& operator[]( ui32 n ) const { return _container[ n ]; }

      /**
       @brief return the i th sample.
       */
      inline SampleType& operator[]( ui32 n ) { return _container[ n ]; }

      /**
       @brief return size of the database.
       */
      inline ui32 size() const { return static_cast<ui32> ( _container.size() ); }

   protected:
      Container   _container;
   };
}
}
#endif
