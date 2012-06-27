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

#ifndef NLL_ALGORITHM_SURF_3D_H_
# define NLL_ALGORITHM_SURF_3D_H_

// if defined, SURF will use a global angle detection mecanism which seems more efficient/robust
#define NLL_SURF_USE_FEATURE_ANGLE_UPGRADE

#define NLL_SURF_3D_NB_AREA_PER_FEATURE         4.0f
#define NLL_SURF_3D_NB_AREA_PER_FEATURE_SIZE    20.0f
#define NLL_SURF_3D_NB_SUBAREA_PER_AREA         5.0f

namespace nll
{
namespace algorithm
{
	/**
    @ingroup algorithm
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm for 3 dimentional data

           Feature detector which intend to be robust to scaling, illumination and rotation

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper

    @note we are assuming the spacing is the same for all dimensions
    */
   class SpeededUpRobustFeatures3d
   {
      typedef double                            value_type;
      typedef FastHessianDetPyramid3d::Volume   Volume;
      typedef core::Matrix<value_type>          Matrix;

      struct LocalPoint
      {
         LocalPoint( value_type x, value_type y, value_type z ) : dx( x ), dy( y ), dz( z )
         {}

         value_type dx;
         value_type dy;
         value_type dz;
      };

   public:
      struct Point
      {
         typedef core::Buffer1D<value_type> Features;
         Point( core::vector3i p, ui32 s ) : position( p ), scale( s ), features( static_cast<int>( 6 * ( NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE ) ) ), weight( 1 )
         {}

         Point() : features( static_cast<int>( 6 * ( NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE ) ) ), weight( 1 )
         {}

         Features                   features;
         value_type                 orientation1;     // dxy
         value_type                 orientation2;     // drz
         core::vector3i             position;
         ui32                       scale;
         float                      weight;        // this will be used to weight the points in the <AffineRegistrationPointBased2d>. By default the algorithm will set it to 1, but a third party could change this value to give more weight to specific points

         void write( std::ostream& o ) const
         {
            core::write<Features>( features, o );
            core::write<value_type>( orientation1, o );
            core::write<value_type>( orientation2, o );
            position.write( o );
            core::write<ui32>( scale, o );
            core::write<float>( weight, o );
         }

         void read( std::istream& i )
         {
            core::read<Features>( features, i );
            core::read<value_type>( orientation1, i );
            core::read<value_type>( orientation2, i );
            position.read( i );
            core::read<ui32>( scale, i );
            core::read<float>( weight, i );
         }

         void print( std::ostream& o ) const
         {
            o << "position=" << position[ 0 ] << " " << position[ 1 ] << " " << position[ 2 ] << std::endl;
            o << "angle=" << orientation1 << " " << orientation2 << std::endl;
            o << "scale=" << scale << std::endl;
            features.print( o );
         }
      };

      typedef core::Buffer1D<Point> Points;

      /**
       @brief Expose the <point>'s <features> array as if it was stored as an array only
       */
      class PointsFeatureWrapper
      {
      public:
         typedef core::Buffer1D<SpeededUpRobustFeatures3d::value_type>  value_type;

      public:
         PointsFeatureWrapper( const Points& points ) : _points( points )
         {}

         ui32 size() const
         {
            return _points.size();
         }

         const core::Buffer1D<SpeededUpRobustFeatures3d::value_type>& operator[]( ui32 n ) const
         {
            return _points[ n ].features;
         }

      private:
         PointsFeatureWrapper( const PointsFeatureWrapper& );
         PointsFeatureWrapper& operator=( const PointsFeatureWrapper& );

      private:
         const Points& _points;
      };


      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures3d( ui32 octaves = 5, ui32 intervals = 4, ui32 init_step = 2, value_type threshold = 0.0000012 ) : _threshold( threshold )
      {
         //std::cout << "intervals=" << intervals << " o=" << octaves << std::endl;
         ui32 step = init_step;
         for ( ui32 o = 1; o <= octaves; ++o )
         {
            for ( ui32 i = 1; i <= intervals; ++i )
            {
               const ui32 filterSize = core::round( 3 * ( std::pow( 2.0, (int)o ) * i + 1 ) );
               
               if ( _filterSizes.size() == 0 || *_filterSizes.rbegin() < filterSize )
               {
                  _filterSizes.push_back( filterSize );
                  _filterSteps.push_back( step );
               }
            }
            step *= 2;
         }
      }

      /**
       @param this computes only points of interest (position and scaling) but doesn't compute the features (orientation, feature vector)
       */
      template <class Volume>
      Points computesPoints( const Volume& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF point detection started..." );
         FastHessianDetPyramid3d pyramid;
         Points points = _computesPoints( i, pyramid );

         core::Timer angle;
         _computeAngle( pyramid.getIntegralImage(), points );
         std::cout << "Angle computation=" << angle.getCurrentTime() << std::endl;

         ss << "Number of points detected=" << points.size();
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Compute the SURF features
       */
      template <class Volume>
      Points computesFeatures( const Volume& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF3D feature detection started..." );

         FastHessianDetPyramid3d pyramid;
         Points points = _computesPoints( i, pyramid );
         ss << "Number of points detected=" << points.size() << std::endl;

         core::Timer timeOrientation;
         _computeAngle( pyramid.getIntegralImage(), points );
         ss << "Orientation time=" << timeOrientation.getCurrentTime() << std::endl;

         core::Timer timeFeeatures;
         _computeFeatures( pyramid.getIntegralImage(), points );
         ss << "Features time=" << timeFeeatures.getCurrentTime();

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Given a list of points (position + scale), compute the features (orientation + feature)
       */
      template <class Volume>
      void computeFeatures( const Volume& i, Points& points )
      {
         FastHessianDetPyramid3d pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         _computeAngle( pyramid.getIntegralImage(), points );
         _computeFeatures( pyramid.getIntegralImage(), points );
      }

   private:
      // sig = standard deviation
      static value_type gaussian(value_type x, value_type y, value_type z, value_type sig)
      {
         // TODO check different values here...

         // get rid of the normalization constant: we don't need it! We just want to give more importance to the points
         // in the center for the robustness, not shift the results toward zero which will cause numerical problems
         return std::exp( -( x * x + y * y + z * z ) / ( 2.0 * sig * sig ) );
      }

      void _computeFeatures( const IntegralImage3d& image, Points& points ) const
      {
         int nbPoints = static_cast<ui32>( points.size() );
         const f32 area_size = NLL_SURF_3D_NB_AREA_PER_FEATURE_SIZE / NLL_SURF_3D_NB_AREA_PER_FEATURE;
         const f32 area_pos_min = - NLL_SURF_3D_NB_AREA_PER_FEATURE / 2 * area_size;
         const f32 area_pos_max =   NLL_SURF_3D_NB_AREA_PER_FEATURE / 2 * area_size;
         const f32 area_pos_center =   area_pos_min + area_size / 2;
         const f32 dd5x5 = (float)(area_size / NLL_SURF_3D_NB_SUBAREA_PER_AREA);
         
         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            Point& point = points[ n ];
            RotationFromSpherical rotation( point.orientation1, point.orientation2 );

            // this constant is to find the gaussian's sigma
            // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
            // so for a filter of size X, sigma = 1.2 / 9 * X
            static const value_type scaleFactor = 1.2 / 9;
            value_type scale = core::round( scaleFactor * point.scale );
            if ( scale < 1 )
               scale = 1;  // this is the minimum possible scale

            const int size = (int)core::sqr( 2 * scale );

            ui32 count = 0;
            value_type len = 0;

            // (i, j, k) the bottom left corners of the 4x4 area, in the unrotated space
            // (cx, cy, ck) the centers of the 4x4 area, in the unrotated space
            value_type cz = area_pos_center;
            for ( value_type k = area_pos_min; k < area_pos_max; k += area_size, cz += area_size )
            {
               value_type cy = area_pos_center;
               for ( value_type j = area_pos_min; j < area_pos_max; j += area_size, cy += area_size )
               {
                  value_type cx = area_pos_center;
                  for ( value_type i = area_pos_min; i < area_pos_max; i += area_size, cx += area_size )
                  {
                     // the feature for each 4x4 region
                     value_type dx = 0;
                     value_type dy = 0;
                     value_type dz = 0;
                     value_type mdx = 0;
                     value_type mdy = 0;
                     value_type mdz = 0;

                     // now compute the 5x5 points for each 4x4 region
                     for ( value_type dk = k; dk < k + area_size; dk += dd5x5 )
                     {
                        for ( value_type dj = j; dj < j + area_size; dj += dd5x5 )
                        {
                           for ( value_type di = i; di < i + area_size; di += dd5x5 )
                           {
                              // center on the rotated axis
                              const core::vector3d gridpoint = rotation.transform( core::vector3d( di * scale, dj * scale, dk * scale ) );
                              const int sample_x = core::round( gridpoint[ 0 ] );
                              const int sample_y = core::round( gridpoint[ 1 ] );
                              const int sample_z = core::round( gridpoint[ 2 ] );
                              if ( sample_x < scale ||
                                   sample_y < scale ||
                                   sample_z < scale )
                              {
                                 // out of bounds
                                 continue;
                              }

                              //Get the gaussian weighted x and y responses
                              const value_type gauss_s1 = gaussian( di - cx, dj - cy, dk - cz, scale );

                              core::vector3ui bl( core::round( sample_x - scale ),
                                                  core::round( sample_y - scale ),

                                                  core::round( sample_z - scale ) );
                              core::vector3ui tr( core::round( sample_x + scale ),
                                                  core::round( sample_y + scale ),
                                                  core::round( sample_z + scale ));

                              if ( tr[ 0 ] < image.size()[ 0 ] && 
                                   tr[ 1 ] < image.size()[ 1 ] &&
                                   tr[ 2 ] < image.size()[ 2 ] )
                              {
                                 const value_type ry = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DY,
                                                                                          image,
                                                                                          bl,
                                                                                          tr ) / size;
                                 const value_type rx = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DX,
                                                                                          image,
                                                                                          bl,
                                                                                          tr ) / size;
                                 const value_type rz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DZ,
                                                                                          image,
                                                                                          bl,
                                                                                          tr ) / size;

                                 //Get the gaussian weighted x and y responses on rotated axis
                                 const core::StaticVector<value_type, 3> f = rotation.transform( core::vector3d( rx, ry, rz ) ) * gauss_s1;

                                 dx += f[ 0 ];
                                 dy += f[ 1 ];
                                 dz += f[ 2 ];
                                 mdx += fabs( f[ 0 ] );
                                 mdy += fabs( f[ 1 ] );
                                 mdz += fabs( f[ 2 ] );
                              }
                           }
                        }
                     }

                     //Add the values to the descriptor vector
                     const value_type gauss_s2 = gaussian( cx, cy, cz, 3.3 * scale );
                     point.features[ count++ ] = dx * gauss_s2;
                     point.features[ count++ ] = dy * gauss_s2;
                     point.features[ count++ ] = dz * gauss_s2;
                     point.features[ count++ ] = mdx * gauss_s2;
                     point.features[ count++ ] = mdy * gauss_s2;
                     point.features[ count++ ] = mdz * gauss_s2;

                     len += ( dx * dx + dy * dy + dz * dz + mdx * mdx + mdy * mdy + mdz * mdz ) * gauss_s2 * gauss_s2;
                  }
               }
            }

            //Convert to Unit Vector
            len = (sqrt( len ) + 1e-7);
            for( ui32 i = 0; i < point.features.size(); ++i )
               point.features[ i ] /= len;
         }
      }

      /**
       @brief assign a repeable orientation for each point       

         The dominant orientation is determined for each SURF 
         key point in order to guarantee the invariance of feature 
         description to image rotatio
       */
      
      static void _computeAngle( const IntegralImage3d& i, Points& points )
      {
         // preprocessed gaussian of size 2.5
         // we need to weight the response so that it is more tolerant to the noise. Indeed, the further
         // away from the centre, the more likely it is to be noisier
         static const value_type gauss25 [7][7][7]  = {
            {
               0.002666886202051,   0.002461846247067,   0.001936556847605,   0.001298112875237,   0.000741493840234, 0.000360923799514,   0.000149705024458,
               0.002461846247067,   0.002272570512958,   0.001787667281731,   0.001198309214586,   0.000684485084665, 0.000333174659132,   0.000138195155213,
               0.001936556847605,   0.001787667281731,   0.001406228890127,   0.000942623414366,   0.000538435038082, 0.000262084469474,   0.000108708159355,
               0.001298112875237,   0.001198309214586,   0.000942623414366,   0.000631859370512,   0.000360923799514, 0.000175680473643,   0.000072869258383,
               0.000741493840234,   0.000684485084665,   0.000538435038082,   0.000360923799514,   0.000206162945641, 0.000100350278886,   0.000041623580865,
               0.000360923799514,   0.000333174659132,   0.000262084469474,   0.000175680473643,   0.000100350278886, 0.000048845724634,   0.000020260371887,
               0.000149705024458,   0.000138195155213,   0.000108708159355,   0.000072869258383,   0.000041623580865, 0.000020260371887,   0.000008403656043
            },
            {
               0.002461846247067,   0.002272570512958,   0.001787667281731,   0.001198309214586,   0.000684485084665, 0.000333174659132,   0.000138195155213,
               0.002272570512958,   0.002097846988828,   0.001650224889666,   0.001106178824010,   0.000631859370512, 0.000307558974046,   0.000127570206769,
               0.001787667281731,   0.001650224889666,   0.001298112875237,   0.000870151082288,   0.000497038185121, 0.000241934457906,   0.000100350278886,
               0.001198309214586,   0.001106178824010,   0.000870151082288,   0.000583279713537,   0.000333174659132, 0.000162173516961,   0.000067266803563,
               0.000684485084665,   0.000631859370512,   0.000497038185121,   0.000333174659132,   0.000190312385141, 0.000092634982804,   0.000038423407891,
               0.000333174659132,   0.000307558974046,   0.000241934457906,   0.000162173516961,   0.000092634982804, 0.000045090286861,   0.000018702680473,
               0.000138195155213,   0.000127570206769,   0.000100350278886,   0.000067266803563,   0.000038423407891, 0.000018702680473,   0.000007757552263
            },
            {
               0.001936556847605,   0.001787667281731,   0.001406228890127,   0.000942623414366,   0.000538435038082, 0.000262084469474,   0.000108708159355,
               0.001787667281731,   0.001650224889666,   0.001298112875237,   0.000870151082288,   0.000497038185121, 0.000241934457906,   0.000100350278886,
               0.001406228890127,   0.001298112875237,   0.001021131754471,   0.000684485084665,   0.000390984084430, 0.000190312385141,   0.000078938325238,
               0.000942623414366,   0.000870151082288,   0.000684485084665,   0.000458824073463,   0.000262084469474, 0.000127570206769,   0.000052913941807,
               0.000538435038082,   0.000497038185121,   0.000390984084430,   0.000262084469474,   0.000149705024458, 0.000072869258383,   0.000030224923164,
               0.000262084469474,   0.000241934457906,   0.000190312385141,   0.000127570206769,   0.000072869258383, 0.000035469275908,   0.000014712049536,
               0.000108708159355,   0.000100350278886,   0.000078938325238,   0.000052913941807,   0.000030224923164, 0.000014712049536,   0.000006102306744
            },
            {
               0.001298112875237,   0.001198309214586,   0.000942623414366,   0.000631859370512,   0.000360923799514, 0.000175680473643,   0.000072869258383,
               0.001198309214586,   0.001106178824010,   0.000870151082288,   0.000583279713537,   0.000333174659132, 0.000162173516961,   0.000067266803563,
               0.000942623414366,   0.000870151082288,   0.000684485084665,   0.000458824073463,   0.000262084469474, 0.000127570206769,   0.000052913941807,
               0.000631859370512,   0.000583279713537,   0.000458824073463,   0.000307558974046,   0.000175680473643, 0.000085512866874,   0.000035469275908,
               0.000360923799514,   0.000333174659132,   0.000262084469474,   0.000175680473643,   0.000100350278886, 0.000048845724634,   0.000020260371887,
               0.000175680473643,   0.000162173516961,   0.000127570206769,   0.000085512866874,   0.000048845724634, 0.000023775766660,   0.000009861781723,
               0.000072869258383,   0.000067266803563,   0.000052913941807,   0.000035469275908,   0.000020260371887, 0.000009861781723,   0.000004090498537
            },
            {
               0.000741493840234220,   0.000684485084665209,   0.000538435038082152,   0.000360923799514429,   0.000206162945641395,   0.000100350278886302, 0.000041623580864519,
               0.000684485084665209,   0.000631859370512295,   0.000497038185120945,   0.000333174659131743,   0.000190312385140791,   0.000092634982804403, 0.000038423407891183,
               0.000538435038082152,   0.000497038185120945,   0.000390984084430091,   0.000262084469474381,   0.000149705024457774,   0.000072869258383364, 0.000030224923164329,
               0.000360923799514429,   0.000333174659131743,   0.000262084469474381,   0.000175680473643293,   0.000100350278886302,   0.000048845724634120, 0.000020260371886937,
               0.000206162945641395,   0.000190312385140791,   0.000149705024457774,   0.000100350278886302,   0.000057320988859612,   0.000027901120641273, 0.000011572908058766,
               0.000100350278886302,   0.000092634982804403,   0.000072869258383364,   0.000048845724634120,   0.000027901120641273,   0.000013580933415951, 0.000005633139105622,
               0.000041623580864519,   0.000038423407891183,   0.000030224923164329,   0.000020260371886937,   0.000011572908058766,   0.000005633139105622, 0.000002336529840137
            },
            {
               0.000360923799514429,   0.000333174659131743,   0.000262084469474381,   0.000175680473643293,   0.000100350278886302,   0.000048845724634120,  0.000020260371886937,
               0.000333174659131743,   0.000307558974046307,   0.000241934457905870,   0.000162173516961070,   0.000092634982804403,   0.000045090286860856,  0.000018702680472704,
               0.000262084469474381,   0.000241934457905870,   0.000190312385140791,   0.000127570206768727,   0.000072869258383364,   0.000035469275908233,  0.000014712049536454,
               0.000175680473643293,   0.000162173516961070,   0.000127570206768727,   0.000085512866873989,   0.000048845724634120,   0.000023775766659657,  0.000009861781722555,
               0.000100350278886302,   0.000092634982804403,   0.000072869258383364,   0.000048845724634120,   0.000027901120641273,   0.000013580933415951,  0.000005633139105622,
               0.000048845724634120,   0.000045090286860856,   0.000035469275908233,   0.000023775766659657,   0.000013580933415951,   0.000006610549978256,  0.000002741943167798,
               0.000020260371886937,   0.000018702680472704,   0.000014712049536454,   0.000009861781722555,   0.000005633139105622,   0.000002741943167798,  0.000001137311170805
            },
            {
               0.000149705024457774,   0.000138195155213183,   0.000108708159355106,   0.000072869258383364,   0.000041623580864519,   0.000020260371886937,  0.000008403656043015,
               0.000138195155213183,   0.000127570206768727,   0.000100350278886302,   0.000067266803562755,   0.000038423407891183,   0.000018702680472704,  0.000007757552262718,
               0.000108708159355106,   0.000100350278886302,   0.000078938325237764,   0.000052913941807354,   0.000030224923164329,   0.000014712049536454,  0.000006102306743534,
               0.000072869258383364,   0.000067266803562755,   0.000052913941807354,   0.000035469275908233,   0.000020260371886937,   0.000009861781722555,  0.000004090498537249,
               0.000041623580864519,   0.000038423407891183,   0.000030224923164329,   0.000020260371886937,   0.000011572908058766,   0.000005633139105622,  0.000002336529840137,
               0.000020260371886937,   0.000018702680472704,   0.000014712049536454,   0.000009861781722555,   0.000005633139105622,   0.000002741943167798,  0.000001137311170805,
               0.000008403656043015,   0.000007757552262718,   0.000006102306743534,   0.000004090498537249,   0.000002336529840137,   0.000001137311170805,  0.000000471737238914
            }
         };
         static const int id[] = { 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6 };

         // this constant is to find the gaussian's sigma
         // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
         // so for a filter of size X, sigma = 1.2 / 9 * X
         static const value_type scaleFactor = 1.2 / 9;
         const int nbPoints = static_cast<int>( points.size() );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            const Point& point = points[ n ];
            const int scale = core::round( scaleFactor * point.scale );

            std::vector<LocalPoint> localPoints;
            localPoints.reserve( 1331 );

            for ( int w = -6; w <= 6; ++w )
            {
               for ( int v = -6; v <= 6; ++v )
               {
                  for ( int u = -6; u <= 6; ++u )
                  {
                     if ( u * u + v * v + w * w < 6 * 6 )
                     {
                        const value_type gauss = gauss25[ id[ u + 6 ] ][ id[ v + 6 ] ][ id[ w + 6 ] ];
                        const int x = point.position[ 0 ] + u * scale;
                        const int y = point.position[ 1 ] + v * scale;
                        const int z = point.position[ 2 ] + w * scale;
                        const core::vector3ui bl( x - 2 * scale, y - 2 * scale, z - 2 * scale );
                        const core::vector3ui tr( x + 2 * scale, y + 2 * scale, z + 2 * scale );
                        
                        if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && bl[ 2 ] >= 0 && tr[ 0 ] < i.size()[ 0 ] && tr[ 1 ] < i.size()[ 1 ] && tr[ 2 ] < i.size()[ 2 ] && z > 2 * scale && y > 2 * scale && x > 2 * scale )
                        {
                           const value_type dy = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFY,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           
                           const value_type dx = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFX,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           
                           const value_type dz = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFZ,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           localPoints.push_back( LocalPoint( dx, dy, dz ) );
                        }
                     }
                  }
               }
            }
           
            const int nbLocalPoints = static_cast<int>( localPoints.size() );

            
            //
            // simple averaging seems better than the technique described in the original paper...
            //  
            double dx = 0;
            double dy = 0;
            double dz = 0;
            for ( int nn = 0; nn < nbLocalPoints; ++nn )
            {
               dx += localPoints[ nn ].dx;
               dy += localPoints[ nn ].dy;
               dz += localPoints[ nn ].dz;
            }
            if ( nbLocalPoints )
            {
               dx /= nbLocalPoints;
               dy /= nbLocalPoints;
               dz /= nbLocalPoints;
            }
            double norm;
            core::cartesianToSphericalCoordinate( dx, dy, dz, norm, points[ n ].orientation1, points[ n ].orientation2 );
         }
      }

      template <class VolumeT>
      Points _computesPoints( const VolumeT& i, FastHessianDetPyramid3d& pyramid )
      {
         ui32 nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         #ifndef NLL_NOT_MULTITHREADED
            const ui32 maxNumberOfThread = omp_get_max_threads();
         #else
            const ui32 maxNumberOfThread = 1;
         #endif
         std::vector< std::vector<Point> > bins( maxNumberOfThread );


         core::Timer timePyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         {
            std::stringstream ss;
            ss << "Pyramid construction time=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         timePyramid.start();

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            if ( pyramid.getPyramidDetHessian().size() <= filter )
               break; // the filter was not used in the pyramid...
            const Volume& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.size()[ 0 ] );
            const int sizey = static_cast<int>( f.size()[ 1 ] );
            const int sizez = static_cast<int>( f.size()[ 2 ] );
            const ui32 step = _filterSteps[ filter + 1 ];

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for reduction(+ : nbPoints)
            #endif
            for ( int z = 0; z < sizez; ++z )
            {
               for ( int y = 0; y < sizey; ++y )
               {
                  for ( int x = 0; x < sizex; ++x )
                  {
                     const value_type val = f( x, y, z );
                     if ( val > _threshold )
                     {
                        //std::cout << "val=" << val << "pos=" << x << " " << y << " " << z << " step=" << step << std::endl;
                        bool isMax = pyramid.isDetHessianMax( val, x, y, z, filter, filter )     &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter + 1 ) &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter - 1 );
                        if ( isMax )
                        {
                           
                           core::vector4d hessianGradient = pyramid.getHessianGradient( x, y, z, filter );
                           
                           Matrix hessianHessian = pyramid.getHessianHessian( x, y, z, filter );
                           
                           const bool inverted = core::inverse( hessianHessian );
                           core::vector4d interpolatedPoint = core::mat4Mulv( hessianHessian, hessianGradient );
                           if ( inverted && interpolatedPoint[ 0 ] < 0.5 &&
                                            interpolatedPoint[ 1 ] < 0.5 &&
                                            interpolatedPoint[ 2 ] < 0.5 )
                           {
                              
                              const int size = _filterSizes[ filter ];
                              const int half = size / 2;
                              // here we need to compute the step between the two scales (i.e., their difference in size and not the step as for the position)
                              const int filterStep = static_cast<int>( _filterSizes[ filter + 1 ] - _filterSizes[ filter ] );

                              // TODO PUT IT BACK AFTER DEBUG. Seems to get wrong results with it
                              /*
                              int px    = core::round( ( x    - interpolatedPoint[ 0 ] ) * _filterSteps[ filter ] );
                              int py    = core::round( ( y    - interpolatedPoint[ 1 ] ) * _filterSteps[ filter ] );
                              int pz    = core::round( ( z    - interpolatedPoint[ 2 ] ) * _filterSteps[ filter ] );
                              int scale = core::round( size   - interpolatedPoint[ 3 ]   * filterStep );
                              */
                              int px    = core::round( x );
                              int py    = core::round( y );
                              int pz    = core::round( z );
                              int scale = core::round( size );
                              if ( scale <= 0 )
                                 continue;   // should not happen, but just in case!

                              #ifndef NLL_NOT_MULTITHREADED
                              ui32 threadId = omp_get_thread_num();
                              #else
                              ui32 threadId = 0;
                              #endif
                              bins[ threadId ].push_back( Point( core::vector3i( px, py, pz ), scale ) );

                              ++nbPoints;
                              
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            std::stringstream ss;
            ss << "non maximal suppression and localization=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Points points( nbPoints );
         ui32 cur = 0;
         for ( size_t bin = 0; bin < bins.size(); ++bin )
         {
            for ( size_t p = 0; p < bins[ bin ].size(); ++p )
            {
               points[ cur++ ] = bins[ bin ][ p ];
            }
         }
         return points;
      }

   public:
      /**
       @brief Given spherical angles, compute the equivalent transformation matrix

       This is just a helper class, public just for the test...
       */
      class RotationFromSpherical
      {
      public:
         typedef double                      value_type;
         typedef core::Matrix<value_type>    Matrix;

      public:
         RotationFromSpherical( value_type phi, value_type theta )
         {
            //
            // TODO: very ugly hack... replace it with a direct computation
            // here we are simply creating a mapping (source points -> transformed points) and estimate the transformation from the point's mapping
            //
            algorithm::EstimatorTransformSimilarityIsometric estimator;
            std::vector<core::vector3d> source;
            std::vector<core::vector3d> target;

            source.push_back( core::vector3d( 1, 0, 0 ) );
            source.push_back( core::vector3d( 2, 0, 0 ) );
         
            std::for_each( source.begin(), source.end(), [&]( const core::vector3d& val )
            {
               core::vector3d spherical = core::cartesianToSphericalCoordinate( val );
               spherical[ 1 ] += phi;
               spherical[ 2 ] += theta;
               target.push_back( core::sphericalToCartesianCoordinate( spherical ) );
            });

            _rotation = estimator.compute( source, target );
         }

         /**
          @brief transform a point p into a new point
          */
         core::vector3d transform( const core::vector3d& p ) const
         {
            return core::transf4( _rotation, p );
         }

      private:
         Matrix _rotation;
      };

   private:
      std::vector<ui32> _filterSizes;
      std::vector<ui32> _filterSteps;
      value_type _threshold;
   };
}
}

#endif