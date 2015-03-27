# Tutorial 1: Image and Distance transform #
Distance transforms are an important tool in computer vision, image processing and pattern recognition. A distance transform of a binary image speciﬁes the distance from each pixel to the nearest non-zero pixel. Distance transforms play a central role in the comparison of binary images, particularly for images resulting from local feature detection techniques such as edge or corner detection.

Let G be a regular grid and ![http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/eq2.gif](http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/eq2.gif) a function on the grid, we define the distance transform _f_ to be:

![http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/eq1.gif](http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/eq1.gif)

where _d(p, q)_ is some measure of the distance between _p_ and _q_

## Example ##
![http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/input.png](http://nll.googlecode.com/svn/devl/wiki/DistanceTransform/input.png)

Example of a 2D distance transform with left the original image, right the transformed image using the euclidian distance.

## Code ##
In this tutorial, the very basic operations on images will be covered:
  * loading/saving images
  * conversion float/char and greyscale/RGB images
  * apply a distance transform

A distance transform, also known as distance map or distance field, is a representation of an image. The map supplies each pixel of the image with the distance to the nearest obstacle pixel for a binary image.


```
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
```

The code can easily be extended to other metrics and datatypes.