# ifndef NLL_DEBUG_DECOMPOSE_H_
# define NLL_DEBUG_DECOMPOSE_H_

namespace nll
{
namespace debug
{
   /**
    @ingroup debug
    @brief decompose a vector into a collection of images ( like from Gabor features )
    */
   template <class ImageType, class Mapper, class VectorType>
   std::vector< core::Image<ImageType, Mapper> > decomposeVector( const core::Buffer1D<VectorType>& vec, ui32 sizex, ui32 sizey, ui32 nbcomp )
   {
      typedef core::Image<ImageType, Mapper> Image;
      typedef std::vector<Image>             Images;
      typedef core::Buffer1D<VectorType>     Vector;
      
      ui32 imageSize = sizex * sizey * nbcomp;
      assert( vec.size() % ( imageSize ) == 0 ); // else it is likely that image size doesn't match
      ui32 nbI = vec.size() / imageSize;
      Images is( nbI );
      for ( ui32 n = 0; n < nbI; ++n )
      {
         Image i( sizex, sizey, nbcomp, false );
         for ( ui32 nn = 0; nn < imageSize; ++nn ) 
            i[ nn ] = static_cast<ImageType> ( vec[ imageSize * n + nn ] );
         is[ n ] = i;
      }
      return is;
   }
}
}

#endif
