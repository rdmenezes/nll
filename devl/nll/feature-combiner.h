#ifndef NLL_FEATURE_COMBINER_H_
# define NLL_FEATURE_COMBINER_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Helper function Combine 2 feature transformation into one by concatenating the different feature vectors
    */
   class FeatureCombiner
   {
   public:

      /**
       @brief It is assumed both databases have exactly the same number of samples and they match (i.e., input1[ n ]
              correspond to the same sample than input2[ n ] )
       */
      template <class Database>
      static Database transform( const Database& input1, const Database& input2 )
      {
         Database out;

         ensure( input1.size() == input2.size(), "samples from input1 and input2 must match!!" );
         for ( ui32 n = 0; n < input1.size(); ++n )
         {
            assert( input1[ n ].output == input2[ n ].output ); // "samples must match!!"
            typename Database::Sample s = input1[ n ];
            s.input = process( input1[ n ].input, input2[ n ].input );

            out.add( s );
         }

         return out;
      }

      /**
       @brief Concats all the features of both points
       */
      template <class Point>
      static Point process( const Point& input1, const Point& input2 )
      {
         Point p( input1.size() + input2.size() );
         for ( ui32 n = 0; n < input1.size(); ++n )
            p[ n ] = input1[ n ];
         for ( ui32 n = 0; n < input2.size(); ++n )
            p[ n + input1.size() ] = input2[ n ];
         return p;
      }
   };
}
}

#endif
