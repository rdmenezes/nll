#ifndef NLL_FEATURE_TRANSFORMATION_H_
# define NLL_FEATURE_TRANSFORMATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm. From a set of features, another set is computed.
    */
   template <class Point>
   class FeatureTransformation
   {
   public:
      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const = 0;

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i ) = 0;

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const = 0;

      /**
       @brief Virtual destructor
       */
      virtual ~FeatureTransformation(){}

   public:
      /**
       @brief Read from a file the status of the algorithm
       */
      void read( const std::string& f )
      {
         std::ifstream i( f.c_str(), std::ios::binary );
         ensure( i.is_open(), "file not found" );
         read( i );
      }

      /**
       @brief Write to a file the status of the algorithm
       */
      void write( const std::string& f )
      {
         std::ofstream o( f.c_str(), std::ios::binary );
         ensure( o.is_open(), "file not found" );
         write( o );
      }

      /**
       @brief Process a full database according to the transformation defined by <code>process</code>
       */
      template <class TDatabase>
      TDatabase process( const TDatabase& dat ) const
      {
         TDatabase newDat;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            typename TDatabase::Sample  sample;
            sample.input   = process( dat[ n ].input );
            sample.output  = dat[ n ].output;
            sample.type    = dat[ n ].type;
            sample.debug   = dat[ n ].debug;
            newDat.add( sample );
         }
         return newDat;
      }
   };
}
}

#endif
