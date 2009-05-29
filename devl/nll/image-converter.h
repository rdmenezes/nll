#ifndef NLL_IMAGE_CONVERTER_H_
# define NLL_IMAGE_CONVERTER_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief convert from one image type and mapper to another. If target type is too small, the value is truncated.
    */
   template <class Ti, class Mapperi, class To, class Mappero>
   void convert( const Image<Ti, Mapperi>& i, Image<To, Mappero>& out )
   {
      Image<To, Mappero> o( i.sizex(), i.sizey(), i.getNbComponents() );
      for ( ui32 ny = 0; ny < i.sizey(); ++ny )
         for ( ui32 nx = 0; nx < i.sizex(); ++nx )
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
               o( nx, ny, c ) = static_cast<To> ( NLL_BOUND( i( nx, ny, c ), (To)Bound<To>::min, (To)Bound<To>::max ) );
      out = o;
   }
}
}

#endif
