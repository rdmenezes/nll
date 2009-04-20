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
      typedef typename Classifier<Point>::Database  Database;

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
         std::ifstream i( f.c_str() );
         ensure( i.is_open(), "file not found" );
         read( i );
      }

      /**
       @brief Write to a file the status of the algorithm
       */
      void write( const std::string& f )
      {
         std::ofstream o( f.c_str() );
         ensure( o.is_open(), "file not found" );
         write( o );
      }

      /**
       @brief A process a full database according to the transformation.

       It is made virtual so that it could be changed if an optimized way other than the
       generic one is possible.
       */
      virtual Database process( const Database& dat ) const
      {
         Database newDat;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            typename Database::Sample  sample;
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
