#ifndef FEATURE_TRANSFORMATION_KERNEL_PCA_H_
# define FEATURE_TRANSFORMATION_KERNEL_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    */
   template <class Point, class Kernel>
   class FeatureTransformationKernelPca : public FeatureTransformation<Point>
   {
      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return p;
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
      }
   };
}
}

#endif
