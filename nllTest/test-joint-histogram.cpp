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
         reset();
      }

      /**
       @brief Increment by one the number of samples stored in the histogram
       */
      void increment()
      {
         ++_nbSamples;
      }

      size_t getNbSamples() const
      {
         return _nbSamples; 
      }

      void setNumberOfSamples( size_t nb )
      {
         _nbSamples = nb;
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

      void print( std::ostream& o ) const
      {
         o << "joint histogram (source = y, target = x):" << std::endl;
         for ( size_t y = 0; y < _storage.sizey(); ++y )
         {
            for ( size_t x = 0; x < _storage.sizex(); ++x )
            {
               o << _storage( x, y, 0 ) << " ";
            }
            o << std::endl;
         }
      }

   private:
      Storage  _storage;
      size_t   _nbSamples;
   };

   /**
    @brief Computes the joint histogram using partial interpolation
    */
   template <class T1, class Storage1>
   void computeHistogram_partialInterpolation( const imaging::VolumeSpatial<T1, Storage1>& source, const imaging::TransformationAffine& tfmSourceTarget, const imaging::VolumeSpatial<T1, Storage1>& target, JointHistogram& histogram )
   {
      typedef imaging::VolumeSpatial<T1, Storage1> Volume1;

      #ifdef NLL_SECURE
      const T1 max1 = *std::max_element( source.begin(), source.end() );
      const T1 min1 = *std::min_element( source.begin(), source.end() );
      const T1 max2 = *std::max_element( target.begin(), target.end() );
      const T1 min2 = *std::min_element( target.begin(), target.end() );
      ensure( max1 < histogram.size(), "error: a discrete value in source is higher than the number of bins in histogram" );
      ensure( max2 < histogram.size(), "error: a discrete value in target is higher than the number of bins in histogram" );
      ensure( min1 >= 0, "error: a discrete value in source is lower than 0" );
      ensure( min2 >= 0, "error: a discrete value in target is lower than 0" );
      #endif

      VolumeTransformationProcessorPartialInterpolationHistogram<Volume1> processor( source, histogram );
      imaging::VolumeTransformationMapper mapper;
      mapper.run( processor, target, tfmSourceTarget, const_cast<Volume1&>( source ) );   // the volume is a const volume mapper...
   }

   /**
    @brief Computes the joint histogram using a simple nearest neighbor
    */
   template <class T1, class Storage1>
   void computeHistogram_nearestNeighbor( const imaging::VolumeSpatial<T1, Storage1>& source, const imaging::TransformationAffine& tfmSourceTarget, const imaging::VolumeSpatial<T1, Storage1>& target, JointHistogram& histogram )
   {
      typedef imaging::VolumeSpatial<T1, Storage1> Volume1;

      #ifdef NLL_SECURE
      const T1 max1 = *std::max_element( source.begin(), source.end() );
      const T1 min1 = *std::min_element( source.begin(), source.end() );
      const T1 max2 = *std::max_element( target.begin(), target.end() );
      const T1 min2 = *std::min_element( target.begin(), target.end() );
      ensure( max1 < histogram.size(), "error: a discrete value in source is higher than the number of bins in histogram" );
      ensure( max2 < histogram.size(), "error: a discrete value in target is higher than the number of bins in histogram" );
      ensure( min1 >= 0, "error: a discrete value in source is lower than 0" );
      ensure( min2 >= 0, "error: a discrete value in target is lower than 0" );
      #endif

      VolumeTransformationProcessorNearestNeighborHistogram<Volume1> processor( source, histogram );
      imaging::VolumeTransformationMapper mapper;
      mapper.run( processor, target, tfmSourceTarget, const_cast<Volume1&>( source ) );   // the volume is a const volume mapper...
   }

   /**
    @brief Function measuring the similarity represented as a joint histogram
    */
   class SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const = 0;

      ~SimilarityFunction()
      {}
   };

   /**
    @brief Sum of square differences similarity function
    */
   class SimilarityFunctionSumOfSquareDifferences : public SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const
      {
         double sum = 0;

         for ( size_t y = 0; y < jh.size(); ++y )
         {
            for ( size_t x = 0; x < jh.size(); ++x )
            {
               sum += jh( x, y ) * core::sqr( double( x ) - double( y ) );
            }
         }

         if ( sum <= 0 )
            return std::numeric_limits<double>::min();
         return - sum / jh.getNbSamples();
      }
   };

   /**
    @ingroup imaging
    @brief Outline of a basic processor, just for illustration as it doesn't do anything

    histogram is filled as (target, source)

    In a multihtreading context, we are using an accumulate-reduce pattern where each thread has its own histogram store
    */
   template <class Volume>
   class VolumeTransformationProcessorPartialInterpolationHistogram
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;
      typedef imaging::InterpolatorTriLinearDummy<Volume>   Interpolator;

   private:
      struct SharedData
      {
         std::vector<JointHistogram>   _localJointHistograms;
      };

   public:
      VolumeTransformationProcessorPartialInterpolationHistogram( const Volume& source, JointHistogram& jointHistogram ) : _source( source ), _interpolator( source ), _jointHistogram( jointHistogram )
      {
         // -2 because the interpolator needs a voxel more!
         _size = core::vector3f( static_cast<float>( source.sizex() ) - 1,
                                 static_cast<float>( source.sizey() ) - 1,
                                 static_cast<float>( source.sizez() ) - 1 );

         _sharedData = std::shared_ptr<SharedData>( new SharedData() );
      }


      // called as soon as the volume mapper started the mapping process
      void start()
      {
         #ifndef NLL_NOT_MULTITHREADED
            const size_t maxNumberOfThread = omp_get_max_threads();
         #else
            const size_t maxNumberOfThread = 1;
         #endif

         _sharedData->_localJointHistograms.clear();
         for ( size_t n = 0; n < maxNumberOfThread; ++n )
         {
            _sharedData->_localJointHistograms.push_back( JointHistogram( _jointHistogram.size() ) );
         }
      }

      // called as soon as the volume mapper ended the mapping process
      void end()
      {
         // if only one histogram, then simply copy
         if ( _sharedData->_localJointHistograms.size() == 1 )
         {
            _jointHistogram = _sharedData->_localJointHistograms[ 0 ];
         } else {
            // we need to accumulate the histograms...
            for ( size_t y = 0; y < _jointHistogram.size(); ++y )
            {
               for ( size_t x = 0; x < _jointHistogram.size(); ++x )
               {
                  double val = 0;
                  for ( size_t n = 0; n < _sharedData->_localJointHistograms.size(); ++n )
                  {
                     val += _sharedData->_localJointHistograms[ n ]( y, x );
                  }
                  _jointHistogram( y, x ) = val;
               }
            }

            size_t nbSamples = 0;
            for ( size_t n = 0; n < _sharedData->_localJointHistograms.size(); ++n )
            {
               nbSamples += _sharedData->_localJointHistograms[ n ].getNbSamples();
            }
            _jointHistogram.setNumberOfSamples( nbSamples );
         }
      }

      // called for each slice
      void startSlice( size_t sliceId )
      {
         
         #ifndef NLL_NOT_MULTITHREADED
         size_t threadId = omp_get_thread_num();
         #else
         size_t threadId = 0;
         #endif

         _localJointHistogram = &_sharedData->_localJointHistograms[ threadId ];
         _interpolator.startInterpolation();
      }

      // called after each slice
      void endSlice( size_t sliceId )
      {
         _interpolator.endInterpolation();
      }

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const DirectionalIterator& targetPosition, const float* sourcePosition )
      {
         typedef typename Volume::value_type value_type;
         // compute the weights (partial interpolation) for the neighbours
         int ix, iy, iz;

         if ( sourcePosition[ 0 ] < 0 || sourcePosition[ 1 ] < 0 || sourcePosition[ 2 ] < 0 ||
              sourcePosition[ 0 ] >= _size[ 0 ] ||
              sourcePosition[ 1 ] >= _size[ 1 ] ||
              sourcePosition[ 2 ] >= _size[ 2 ] )
         {
            // we are outside the volume, so just skip this voxel
            return;
         }
         _interpolator.computeWeights( sourcePosition, _weights, ix, iy, iz );

         // get the values of the neightbours
         typename Volume::ConstDirectionalIterator it = _source.getIterator( ix, iy, iz );
         typename Volume::ConstDirectionalIterator itz( it );
         itz.addz();

         const value_type v000 = *it;

         const value_type v100 = it.pickx();
         const value_type v101 = itz.pickx();
         const value_type v010 = it.picky();
         const value_type v011 = itz.picky();
         const value_type v001 = it.pickz();
         const value_type v110 = *it.addx().addy();
         const value_type v111 = it.pickz();

         // finally, update the joint histogram
         const value_type targetVal = *targetPosition;

         #ifndef NLL_NOT_MULTITHREADED
         size_t threadId = omp_get_thread_num();
         #else
         size_t threadId = 0;
         #endif
         
         (*_localJointHistogram)( v000, targetVal ) += _weights[ 0 ];
         (*_localJointHistogram)( v001, targetVal ) += _weights[ 1 ];
         (*_localJointHistogram)( v011, targetVal ) += _weights[ 2 ];
         (*_localJointHistogram)( v010, targetVal ) += _weights[ 3 ];
         (*_localJointHistogram)( v100, targetVal ) += _weights[ 4 ];
         (*_localJointHistogram)( v101, targetVal ) += _weights[ 5 ];
         (*_localJointHistogram)( v111, targetVal ) += _weights[ 6 ];
         (*_localJointHistogram)( v110, targetVal ) += _weights[ 7 ];
         _localJointHistogram->increment();
      }

   private:
      const Volume&     _source;
      Interpolator      _interpolator;
      JointHistogram&   _jointHistogram;
      float             _weights[8];
      core::vector3f    _size;

      JointHistogram*               _localJointHistogram;
      std::shared_ptr<SharedData>   _sharedData;
   };

   /**
    @ingroup imaging
    @brief Outline of a basic processor, just for illustration as it doesn't do anything

    histogram is filled as (target, source)
    */
   template <class Volume>
   class VolumeTransformationProcessorNearestNeighborHistogram
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;
      typedef imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      /*
   private:
      struct SharedData
      {
         std::vector<JointHistogram>   _localJointHistograms;
      };
      */
   public:
      VolumeTransformationProcessorNearestNeighborHistogram( const Volume& source, JointHistogram& jointHistogram ) : _source( source ), _interpolator( source ), _jointHistogram( jointHistogram )
      {
         _size = core::vector3f( static_cast<float>( source.sizex() ) - 0.5f,
                                 static_cast<float>( source.sizey() ) - 0.5f,
                                 static_cast<float>( source.sizez() ) - 0.5f );
      }

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
         typedef typename Volume::value_type value_type;
         if ( sourcePosition[ 0 ] < 0 || sourcePosition[ 1 ] < 0 || sourcePosition[ 2 ] < 0 ||
              sourcePosition[ 0 ] >= _size[ 0 ] ||
              sourcePosition[ 1 ] >= _size[ 1 ] ||
              sourcePosition[ 2 ] >= _size[ 2 ] )
         {
            // we are outside the volume, so just skip this voxel
            return;
         }

         // simply round to the neares integer and compute the source and target values
         const value_type v000 = _interpolator( sourcePosition );
         const value_type targetVal = *targetPosition;         
         ++_localJointHistogram( v000, targetVal );
         _localJointHistogram.increment();
      }

   private:
      const Volume&     _source;
      Interpolator      _interpolator;
      JointHistogram&   _jointHistogram;
      core::vector3f    _size;
      JointHistogram    _localJointHistogram;
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
      core::Matrix<float> tfmMat = core::identityMatrix<core::Matrix<float>>( 4 );
      //tfmMat( 0, 3 ) = -1;
      imaging::TransformationAffine tfm( tfmMat );

      //Volume v( core::vector3ui( 8, 8, 8), core::identityMatrix<core::Matrix<float>>( 4 ) );
      Volume v( core::vector3ui( 512, 512, 256), core::identityMatrix<core::Matrix<float>>( 4 ) );
      for ( Volume::iterator it = v.begin(); it != v.end(); ++it )
      {
         *it = rand() % 8;
      }
      algorithm::JointHistogram jh( 8 );

      for ( ui32 n = 0; n < 10; ++n )
      {
         jh.reset();   

         core::Timer timer;
         //algorithm::computeHistogram_nearestNeighbor( v, tfm, v, jh );
         algorithm::computeHistogram_partialInterpolation( v, tfm, v, jh );

         /*
         std::cout << "JointHistogramTime=" << timer.getCurrentTime() << std::endl;

         algorithm::SimilarityFunctionSumOfSquareDifferences ssd;
         std::cout << "similarity=" << ssd.evaluate( jh ) << std::endl;

         jh.print( std::cout );

         std::cout << "samples=" << jh.getNbSamples() << std::endl;
         */
         TESTER_ASSERT( ( v.getSize()[ 0 ] - 1 ) * ( v.getSize()[ 1 ] - 1 ) * ( v.getSize()[ 2 ] - 1 ) == jh.getNbSamples() );
      }

   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestJointHistogram);
TESTER_TEST(testJointHistogram);
//TESTER_TEST(testWeigthsInterpolatorDummy);
TESTER_TEST_SUITE_END();
#endif