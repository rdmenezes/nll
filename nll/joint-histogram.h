/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_H_
# define NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      // forward declaration
      template <class Volume>
      class VolumeTransformationProcessorPartialInterpolationHistogram;

      template <class Volume>
      class VolumeTransformationProcessorNearestNeighborHistogram;
   }

   /**
    @ingroup algorithm
    @brief Compute the joint histogram

    @note usually, storing the histogramm as (source, target)
    */
   class JointHistogram
   {
   public:
      typedef float                                                                    value_type;
      typedef core::Image<value_type, core::IndexMapperRowMajorFlat2DColorRGBnMask>    Storage;

   public:
      JointHistogram( size_t nbBins = 256 ) : _storage( nbBins, nbBins, 1 ), _nbSamples( 0 )
      {}

      /**
       @brief Increment by one the number of samples stored in the histogram
       */
      void increment()
      {
         ++_nbSamples;
      }

      value_type getNbSamples() const
      {
         return _nbSamples; 
      }

      void setNbSamples( value_type nb )
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
      size_t getNbBins() const
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
      Storage     _storage;
      value_type  _nbSamples;
   };

   /**
    @brief Computes the joint histogram using partial interpolation
    */
   template <class T1, class Storage1>
   void computeHistogram_partialTrilinearInterpolation( const imaging::VolumeSpatial<T1, Storage1>& source, const imaging::Transformation& tfmSourceTarget, const imaging::VolumeSpatial<T1, Storage1>& target, JointHistogram& histogram )
   {
      typedef imaging::VolumeSpatial<T1, Storage1> Volume1;

      #ifdef NLL_SECURE
      const T1 max1 = *std::max_element( source.begin(), source.end() );
      const T1 min1 = *std::min_element( source.begin(), source.end() );
      const T1 max2 = *std::max_element( target.begin(), target.end() );
      const T1 min2 = *std::min_element( target.begin(), target.end() );
      ensure( max1 < histogram.getNbBins(), "error: a discrete value in source is higher than the number of bins in histogram" );
      ensure( max2 < histogram.getNbBins(), "error: a discrete value in target is higher than the number of bins in histogram" );
      ensure( min1 >= 0, "error: a discrete value in source is lower than 0" );
      ensure( min2 >= 0, "error: a discrete value in target is lower than 0" );
      #endif

      impl::VolumeTransformationProcessorPartialInterpolationHistogram<Volume1> processor( target, histogram );
      imaging::VolumeTransformationMapperChooser mapper;
      mapper.run( processor, target, tfmSourceTarget, const_cast<Volume1&>( source ) );   // the volume is a const volume mapper...
   }

   /**
    @brief Computes the joint histogram using a simple nearest neighbor
    */
   template <class T1, class Storage1>
   void computeHistogram_nearestNeighbor( const imaging::VolumeSpatial<T1, Storage1>& source, const imaging::Transformation& tfmSourceTarget, const imaging::VolumeSpatial<T1, Storage1>& target, JointHistogram& histogram )
   {
      typedef imaging::VolumeSpatial<T1, Storage1> Volume1;

      #ifdef NLL_SECURE
      const T1 max1 = *std::max_element( source.begin(), source.end() );
      const T1 min1 = *std::min_element( source.begin(), source.end() );
      const T1 max2 = *std::max_element( target.begin(), target.end() );
      const T1 min2 = *std::min_element( target.begin(), target.end() );
      ensure( max1 < histogram.getNbBins(), "error: a discrete value in source is higher than the number of bins in histogram" );
      ensure( max2 < histogram.getNbBins(), "error: a discrete value in target is higher than the number of bins in histogram" );
      ensure( min1 >= 0, "error: a discrete value in source is lower than 0" );
      ensure( min2 >= 0, "error: a discrete value in target is lower than 0" );
      #endif

      impl::VolumeTransformationProcessorNearestNeighborHistogram<Volume1> processor( target, histogram );
      imaging::VolumeTransformationMapperChooser mapper;
      mapper.run( processor, target, tfmSourceTarget, const_cast<Volume1&>( source ) );   // the volume is a const volume mapper...
   }

   namespace impl
   {
      /**
       @ingroup imaging
       @brief Outline of a basic processor, just for illustration as it doesn't do anything

       histogram is filled as (target, source)

       In a multihtreading context, we are using an accumulate-reduce pattern where each thread has its own histogram store
       */
      template <class Volume>
      class VolumeTransformationProcessorPartialInterpolationHistogram : public core::NonAssignable
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
               _sharedData->_localJointHistograms.push_back( JointHistogram( _jointHistogram.getNbBins() ) );
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
               for ( size_t y = 0; y < _jointHistogram.getNbBins(); ++y )
               {
                  for ( size_t x = 0; x < _jointHistogram.getNbBins(); ++x )
                  {
                     JointHistogram::value_type val = 0;
                     for ( size_t n = 0; n < _sharedData->_localJointHistograms.size(); ++n )
                     {
                        val += _sharedData->_localJointHistograms[ n ]( y, x );
                     }
                     _jointHistogram( y, x ) = val;
                  }
               }

               JointHistogram::value_type nbSamples = 0;
               for ( size_t n = 0; n < _sharedData->_localJointHistograms.size(); ++n )
               {
                  nbSamples += _sharedData->_localJointHistograms[ n ].getNbSamples();
               }
               _jointHistogram.setNbSamples( nbSamples );
            }
         }

         void startSlice( size_t )
         {
         
            #ifndef NLL_NOT_MULTITHREADED
            size_t threadId = omp_get_thread_num();
            #else
            size_t threadId = 0;
            #endif

            _localJointHistogram = &_sharedData->_localJointHistograms[ threadId ];
            _interpolator.startInterpolation();
         }

         void endSlice( size_t )
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

            (*_localJointHistogram)( v000, targetVal ) += _weights[ 0 ];
            (*_localJointHistogram)( v100, targetVal ) += _weights[ 1 ];
            (*_localJointHistogram)( v110, targetVal ) += _weights[ 2 ];
            (*_localJointHistogram)( v010, targetVal ) += _weights[ 3 ];
            (*_localJointHistogram)( v001, targetVal ) += _weights[ 4 ];
            (*_localJointHistogram)( v101, targetVal ) += _weights[ 5 ];
            (*_localJointHistogram)( v111, targetVal ) += _weights[ 6 ];
            (*_localJointHistogram)( v011, targetVal ) += _weights[ 7 ];
         
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
      class VolumeTransformationProcessorNearestNeighborHistogram : public core::NonAssignable
      {
      public:
         typedef typename Volume::DirectionalIterator          DirectionalIterator;
         typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;
         typedef imaging::InterpolatorNearestNeighbour<Volume> Interpolator;

      private:
         struct SharedData
         {
            std::vector<JointHistogram>   _localJointHistograms;
         };

      public:
         VolumeTransformationProcessorNearestNeighborHistogram( const Volume& source, JointHistogram& jointHistogram ) : _source( source ), _interpolator( source ), _jointHistogram( jointHistogram )
         {
            _size = core::vector3f( static_cast<float>( source.sizex() ) - 0.5f,
                                    static_cast<float>( source.sizey() ) - 0.5f,
                                    static_cast<float>( source.sizez() ) - 0.5f );
            _sharedData = std::shared_ptr<SharedData>( new SharedData() );
         }

      
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
               // for each thread, create a new histogram
               _sharedData->_localJointHistograms.push_back( JointHistogram( _jointHistogram.getNbBins() ) );
            }
         }

         void end()
         {
            // if only one histogram, then simply copy
            if ( _sharedData->_localJointHistograms.size() == 1 )
            {
               _jointHistogram = _sharedData->_localJointHistograms[ 0 ];
            } else {
               // we need to accumulate the histograms...
               for ( size_t y = 0; y < _jointHistogram.getNbBins(); ++y )
               {
                  for ( size_t x = 0; x < _jointHistogram.getNbBins(); ++x )
                  {
                     JointHistogram::value_type val = 0;
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
               _jointHistogram.setNbSamples( nbSamples );
            }
         }

         void startSlice( size_t )
         {
         
            #ifndef NLL_NOT_MULTITHREADED
            size_t threadId = omp_get_thread_num();
            #else
            size_t threadId = 0;
            #endif

            _localJointHistogram = &_sharedData->_localJointHistograms[ threadId ];
            _interpolator.startInterpolation();
         }

         void endSlice( size_t )
         {
            _interpolator.endInterpolation();
         }

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
            ++(*_localJointHistogram)( v000, targetVal );
            _localJointHistogram->increment();
         }

      private:
         const Volume&     _source;
         Interpolator      _interpolator;
         JointHistogram&   _jointHistogram;
         core::vector3f    _size;

         JointHistogram*               _localJointHistogram;
         std::shared_ptr<SharedData>   _sharedData;
      };
   }

   /**
    @ingroup algorithm
    @brief Class encapsulating the histogram creation mecanism
    */
   template <class T, class Storage>
   class HistogramMaker
   {
   public:
      typedef imaging::VolumeSpatial<T, Storage> Volume;

      virtual ~HistogramMaker()
      {}

      /**
       @brief Compute the joint histogram of a source and target transformed volume
       */
      virtual void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const = 0;
   };

   /**
    @ingroup algorithm
    @brief Create a joint histogram using a nearest neighbor histogram
    */
   template <class T, class Storage>
   class HistogramMakerNearestNeighbor : public HistogramMaker<T, Storage>
   {
   public:
      typedef HistogramMaker<T, Storage>     Base;
      typedef typename Base::Volume          Volume;

      void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const 
      {
         algorithm::computeHistogram_nearestNeighbor( source, tfmSourceTarget, target, histogram );
      }
   };

   /**
    @ingroup algorithm
    @brief Create a joint histogram using a trilinear weight interpolator
    */
   template <class T, class Storage>
   class HistogramMakerTrilinearPartial : public HistogramMaker<T, Storage>
   {
   public:
      typedef HistogramMaker<T, Storage>     Base;
      typedef typename Base::Volume          Volume;

      void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const 
      {
         algorithm::computeHistogram_partialTrilinearInterpolation( source, tfmSourceTarget, target, histogram );
      }
   };
}
}

#endif
