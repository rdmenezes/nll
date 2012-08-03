#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute the joint histogram

    @note usually, storing the histogramm as (source, target)
    */
   class JointHistogram
   {
      typedef float                                                                    value_type;
      typedef core::Image<value_type, core::IndexMapperRowMajorFlat2DColorRGBnMask>    Storage;

   public:
      JointHistogram( size_t nbBins = 256 ) : _storage( nbBins, nbBins, 1 )
      {
      }

      /**
       @brief Increment by one the number of samples stored in the histogram
       */
      void increment()
      {
         ++_nbSamples;
      }

      /**
       @brief Set to 0 the joint histogram
       */
      void reset()
      {
         _nbSamples = 0;
         for ( Storage::iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            *it = 0;
         }
      }

      /**
       @brief return the number of bins of the histogram
       */
      size_t size() const
      {
         return _storage.sizex();
      }

      value_type& operator()( size_t sourceBin, size_t targetBin )
      {
         return _storage( sourceBin, targetBin, 0 );
      }

      const value_type operator()( size_t sourceBin, size_t targetBin ) const
      {
         return _storage( sourceBin, targetBin, 0 );
      }

   private:
      Storage  _storage;
      size_t   _nbSamples;
   };

   template <class T1, class Storage1, class T2, class Storage2>
   void computeHistogram_partialInterpolation( imaging::VolumeSpatial<T1, Storage1>& source, const imaging::TransformationAffine& tfmSourceTarget, const imaging::VolumeSpatial<T2, Storage2>& target, JointHistogram& histogram )
   {
      typedef imaging::VolumeSpatial<T1, Storage1> Volume1;
      typedef imaging::VolumeSpatial<T2, Storage2> Volume2

      #ifdef NLL_SECURE
      const T1 max1 = *std::std::max_element( source.begin(), source.end() );
      const T1 min1 = *std::std::min_element( source.begin(), source.end() );
      const T2 max2 = *std::std::max_element( target.begin(), target.end() );
      const T2 min2 = *std::std::min_element( target.begin(), target.end() );
      ensure( max1 < histogram.size(), "error: a discrete value in source is higher than the number of bins in histogram" );
      ensure( max2 < histogram.size(), "error: a discrete value in target is higher than the number of bins in histogram" );
      ensure( max1 >= 0, "error: a discrete value in source is lower than 0" );
      ensure( max2 >= 0, "error: a discrete value in target is lower than 0" );
      #endif
      
      histogram.reset();

      // TODO
   }

   /**
    @ingroup imaging
    @brief Outline of a basic processor, just for illustration as it doesn't do anything

    histogram is filled as (target, source)
    */
   template <class Volume>
   class VolumeTransformationProcessorPartialInterpolationHistogram : public core::NonCopyable
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;
      typedef imaging::InterpolatorTriLinearDummy<Volume>   Interpolator;

   public:
      VolumeTransformationProcessorPartialInterpolationHistogram( Volume& source ) : _source( source ), _interpolator( source )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {
         _interpolator.startInterpolation();
      }

      // called as soon as the volume mapper ended the mapping process
      void end()
      {
         _interpolator.endInterpolation();
      }

      // called for each slice
      void startSlice( size_t sliceId )
      {}

      // called after each slice
      void endSlice( size_t sliceId )
      {}

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const DirectionalIterator& targetPosition, const float* sourcePosition )
      {
         // compute the weights (partial interpolation) for the neighbours
         int ix, iy, iz;
         _interpolator.computeWeights( sourcePosition, _weights, ix, iy, iz );

         // get the values of the neightbours
         typename VolumeType::ConstDirectionalIterator it = _volume->getIterator( ix, iy, iz );
         typename VolumeType::ConstDirectionalIterator itz( it );
         itz.addz();

         v000 = *it;

         v100 = it.pickx();
         v101 = itz.pickx();
         v010 = it.picky();
         v011 = itz.picky();
         v001 = it.pickz();
         v110 = *it.addx().addy();
         v111 = it.pickz();

         // finally, update the joint histogram
         const typedef typename DirectionalIterator::value_type targetVal = *targetPosition;
         _jointHistogram( v000, targetVal ) += _weights[ 0 ];
         _jointHistogram( v001, targetVal ) += _weights[ 1 ];
         _jointHistogram( v011, targetVal ) += _weights[ 2 ];
         _jointHistogram( v010, targetVal ) += _weights[ 3 ];
         _jointHistogram( v100, targetVal ) += _weights[ 4 ];
         _jointHistogram( v101, targetVal ) += _weights[ 5 ];
         _jointHistogram( v111, targetVal ) += _weights[ 6 ];
         _jointHistogram( v110, targetVal ) += _weights[ 7 ];
         _jointHistogram.increment();
      }

   private:
      Volume&           _source;
      Interpolator      _interpolator;
      JointHistogram&   _jointHistogram;
      float             _weights[8];
   };
}
}

class TestJointHistogram
{
public:
   typedef imaging::VolumeSpatial<ui8>                   Volume;
   typedef imaging::InterpolatorTriLinearDummy<Volume>   Interpolator;

   void testWeigthsInterpolatorDummy()
   {
      Volume v;
      Interpolator interpolator( v );

      float weights[ 8 ];
      int ix, iy, iz;

      //
      // test each corner against expected return index
      //
      {
         interpolator.computeWeights( core::vector3f( 0, 0, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 0;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999, 0, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 1;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999, 0.999, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 2;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0, 0.999, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 3;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      // other level
      {
         interpolator.computeWeights( core::vector3f( 0, 0, 0.999 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 4;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999, 0, 0.999 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 5;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999, 0.999, 0.999 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 6;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0, 0.999, 0.999 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 7;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      //
      // test the sum is 1
      //
      for ( ui32 n = 0; n < 1000; ++n )
      {
         core::vector3f pos( core::generateUniformDistribution(0, 1),
                             core::generateUniformDistribution(0, 1),
                             core::generateUniformDistribution(0, 1) );
         interpolator.computeWeights( core::vector3f( 0, 0.999, 0 ).getBuf(), weights, ix, iy, iz );

         const double val = core::generic_norm2<float*, float, 8>( weights );
         TESTER_ASSERT( core::equal<double>( val, 1, 1e-3 ) );
      }
   }

   void testJointHistogram()
   {

   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestJointHistogram);
TESTER_TEST(testJointHistogram);
TESTER_TEST(testWeigthsInterpolatorDummy);
TESTER_TEST_SUITE_END();
#endif