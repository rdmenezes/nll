#ifndef FEATURE_TRANSFORMATION_KERNEL_PCA_H_
# define FEATURE_TRANSFORMATION_KERNEL_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    Non linear feature transformation using a kernel PCA algorithm.
    */
   template <class Point, class Kernel>
   class FeatureTransformationKernelPca : public FeatureTransformation<Point>
   {
   public:
      typedef  FeatureTransformation<Point>  Base;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   protected:
      typedef core::DatabaseInputAdapterRead<Database>   PointsAdapter;

   public:
      FeatureTransformationKernelPca( const Kernel& kernel )
      {
         _kernel = kernel.clone();
      }

      virtual ~FeatureTransformationKernelPca()
      {
         delete _kernel;
      }

      template <class Points>
      bool compute( const Points& points, ui32 nbFeatures )
      {
         PointsAdapter adapter( points );
         return _algorithm.compute( adapter, nbFeatures, *_kernel );
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _algorithm.transform( p );
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _algorithm.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _algorithm.write( o );
      }

   protected:
      Kernel*                    _kernel;
      KernelPca<Point, Kernel>   _algorithm;
   };
}
}

#endif
