#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Load, Manipulate an apply transformation to an image 

    The goals of this tutorial are:
    - how to manipulate images and apply basic transformations
    
    */
   struct TutorialDistanceTransform
   { 
      void runDistanceTransform()
      {
         // we define a regular RGB U8 bitmap
         typedef nll::core::Image<unsigned char> Bitmap;

         Bitmap i;

         // load the image as a BMP. The image type has to be 'unsigned char' so that it is correctly
         // loaded as a regular bitmap. However, if necessary, it can be encoded/decoded as float
         // or all other type.
         nll::core::readBmp( i, NLL_DATABASE_PATH "data/image/test-image2.bmp" );

         // we want to transform it to a greyscale image
         nll::core::decolor( i );

         // we apply a distance transform, using a regular euclidian metric
         nll::core::Image<double> dt = nll::core::distanceTransform( i );

         // we transform back the distance transform from a double image to an unsigned one
         // the image will be correcly saturated, using only the [0-255] range
         nll::core::convert( dt, i );

         // The image is extended to a RGB image and saved
         nll::core::extend( i, 3 );
         nll::core::writeBmp( i, NLL_DATABASE_PATH "data/out/test1.bmp" );
      }
   };
}
}