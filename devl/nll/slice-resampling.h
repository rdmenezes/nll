#ifndef NLL_IMAGING_SLICE_RESAMPLING_H_
# define NLL_IMAGING_SLICE_RESAMPLING_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample an input slice to a output geometry
    @param input the input slice
    @param interpolator2D the interpolator to be used for resampling
    @param output the output slice. It must be already allocated. The input will be resampled to the output
           geometry
    @note the input & output slices must share the same normal
    */
   template <class T, class Interpolator2D>
   void resampling( const Slice<T>& input, Slice<T>& output )
   {
      core::Buffer1D<float> buf( output.size()[ 2 ] );

      typedef Slice<T>  SliceType;
      ensure( output.size()[ 0 ] && output.size()[ 1 ] && output.size()[ 2 ], "the output slice is invalid!" );
      ensure( ( core::equal<float>( input.getNormal()[ 0 ], output.getNormal()[ 0 ], 1e-5 ) &&
                core::equal<float>( input.getNormal()[ 1 ], output.getNormal()[ 1 ], 1e-5 ) &&
                core::equal<float>( input.getNormal()[ 2 ], output.getNormal()[ 2 ], 1e-5 ) ) || 
              ( core::equal<float>( -input.getNormal()[ 0 ], output.getNormal()[ 0 ], 1e-5 ) &&
                core::equal<float>( -input.getNormal()[ 1 ], output.getNormal()[ 1 ], 1e-5 ) &&
                core::equal<float>( -input.getNormal()[ 2 ], output.getNormal()[ 2 ], 1e-5 ) ), "Input & output slice must be in the same plane" );
      ensure( output.size()[ 2 ] == input.size()[ 2 ], "input and output values must be of the same type" );
      ensure( output.isInPlane( input.getOrigin() ), "input and output slices must be in the same plan" );
      
      // compute ( ( 1, 0 ) - ( 0, 0 ) ) and ( ( 0, 1 ) - ( 0, 0 ) ) in input slice coordinate
      // compute doutputx = u1 * dinputx + v1 * dinputy
      // compute doutputy = u2 * dinputx + v2 * dinputy
      const core::vector2f outOriginInInput = input.worldToSliceCoordinate( output.getOrigin() );
      const core::vector2f dx = input.worldToSliceCoordinate( output.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - outOriginInInput;
      const core::vector2f dy = input.worldToSliceCoordinate( output.sliceToWorldCoordinate( core::vector2f( 0, 1 ) ) ) - outOriginInInput;


      Interpolator2D interpolator( input.getStorage() );
      SliceType::DirectionalIterator it = output.getIterator( 0, 0 );

      // find the top left corner of the output slice in world coordinate
      const core::vector3f cornerInWorldSpace = output.sliceToWorldCoordinate( core::vector2f( - static_cast<float>( output.size()[ 0 ] ) / 2,
                                                                                               - static_cast<float>( output.size()[ 1 ] ) / 2 ) );
      const core::vector2f cornerInInput = input.worldToSliceCoordinate( cornerInWorldSpace );
      core::vector2f start = cornerInInput + core::vector2f( static_cast<float>( input.size()[ 0 ] ) / 2, static_cast<float>( input.size()[ 1 ] ) / 2 );
      for ( ui32 y = 0; y < output.size()[ 1 ]; ++y )
      {
         core::vector2f pixelIter = start;
         SliceType::DirectionalIterator itline = it;
         for ( ui32 x = 0; x < output.size()[ 0 ]; ++x )
         {
            interpolator.interpolate( pixelIter[ 0 ], pixelIter[ 1 ], buf.getBuf() );
            for ( ui32 c = 0; c < output.size()[ 2 ]; ++c )
               itline.pickcol( c ) = buf[ c ];
            itline.addx();
            pixelIter = pixelIter + dx;
         }
         it.addy();
         start = start + dy;
      }
   }
}
}

#endif