# include "segment-tool-manipulators.h"
# include "segment.h"

namespace mvv
{
namespace platform
{
   bool ToolManipulatorsPoint::checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e )
   {
      Sliceuc& slice = s.segment.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return false;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
      nll::core::vector2f p2 = slice.worldToSliceCoordinate( p1 );
      p2 += nll::core::vector2f( slice.size()[ 0 ] / 2, slice.size()[ 1 ] / 2 );
      nll::core::vector2f pos = nll::core::vector2f( positionStartStroke[ 0 ],
                                                     positionStartStroke[ 1 ] );

      if ( fabs( pos[ 0 ] - p2[ 0 ] ) < 8 &&
           fabs( pos[ 1 ] - p2[ 1 ] ) < 8 )
      {
         if ( e.isMouseLeftButtonPressed )
         {
            nll::core::vector2f v( (float)positionEndStroke[ 0 ] - (float)positionStartStroke[ 0 ],
                                   (float)positionEndStroke[ 1 ] - (float)positionStartStroke[ 1 ] );

            nll::core::vector3f vv( v[ 0 ] * slice.getAxisX()[ 0 ] * slice.getSpacing()[ 0 ] + v[ 1 ] * slice.getAxisY()[ 0 ] * slice.getSpacing()[ 1 ],
                                    v[ 0 ] * slice.getAxisX()[ 1 ] * slice.getSpacing()[ 0 ] + v[ 1 ] * slice.getAxisY()[ 1 ] * slice.getSpacing()[ 1 ],
                                    v[ 0 ] * slice.getAxisX()[ 2 ] * slice.getSpacing()[ 0 ] + v[ 1 ] * slice.getAxisY()[ 2 ] * slice.getSpacing()[ 1 ] );
            _position += vv;
         }
         return true;
      }
      return false;
   }

   void ToolManipulatorsPoint::draw( ResourceSliceuc& s, bool isActivated )
   {
      nll::core::vector3uc color = isActivated ? _color : _colorInactif;
      Sliceuc& slice = s.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
      nll::core::vector2f p2 = slice.worldToSliceCoordinate( p1 );
      nll::core::vector2i pi( static_cast<int>( p2[ 0 ] ),
                              static_cast<int>( p2[ 1 ] ) );
      nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                static_cast<int>( slice.size()[ 1 ] ) );

      if ( slice.contains( p2 ) )
      {
         const int pointSize = 5;

         i32 xmin = std::max( pi[ 0 ] - pointSize, -size[ 0 ] / 2 + 1 );
         i32 xmax = std::min( pi[ 0 ] + pointSize,  size[ 0 ] / 2 - 1 );

         i32 ymin = std::max( pi[ 1 ] - pointSize, -size[ 1 ] / 2 + 1 );
         i32 ymax = std::min( pi[ 1 ] + pointSize,  size[ 1 ] / 2 - 1 );

         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( xmin + slice.size()[ 0 ] / 2, static_cast<ui32>( p2[ 1 ] + slice.size()[ 1 ] / 2 ) );
         for ( i32 n = xmin; n < xmax; ++n, it.addx() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }

         it = slice.getIterator( static_cast<ui32>( p2[ 0 ] + slice.size()[ 0 ] / 2 ), ymin + slice.size()[ 1 ] / 2 );
         for ( i32 n = ymin; n < ymax; ++n, it.addy() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }
   }

   void SegmentToolManipulators::receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& origin )
   {
      nll::core::vector2i strokeEnd( (int)e.mousePosition[ 0 ] - (int)origin[ 0 ],
                                      (int)e.mousePosition[ 1 ] - (int)origin[ 1 ] );
      nll::core::vector2i strokeStart = _leftMouseLastPos;
      _leftMouseLastPos = strokeEnd;

      // first check the last activated manipulator is not active
      const bool isEmpty = _lastActivated.isEmpty();
      const ToolManipulatorsInterface* ptr = isEmpty ? 0 : _lastActivated.getDataPtr();

      if ( ptr && (*_lastActivated).checkEvent( s, strokeStart, strokeEnd, e ) )
      {
         _wasActivated = true;
         notify();   // we must redaw all connected segment
         return;
      }

      // do the others...
      for ( ui32 n = 0; n < _manipulators.size(); ++n )
      {
         if ( _lastActivated != _manipulators[ n ] && (*_manipulators[ n ]).checkEvent( s, strokeStart, strokeEnd, e ) )
         {
            _lastActivated = _manipulators[ n ];
            _wasActivated = true;
            notify();   // we must redaw all connected segment
            return;
         }
      }

      // nobody has been activated so far, so remove the last activated manipulator
      _lastActivated.unref(); // no manipulator are activated
      if ( _wasActivated )
      {
         // means last refresh, it was activated, but it is not anymore so we need to display the inactivated state...
         notify();
         _wasActivated = false;
      }
   }

   void SegmentToolManipulators::updateSegment( ResourceSliceuc& slice, Segment& )
   {
      for ( ui32 n = 0; n < _manipulators.size(); ++n )
      {
         (*_manipulators[ n ]).dispatch( _links );
      }

      for ( ui32 n = 0; n < _manipulators.size(); ++n )
      {
         if ( _manipulators[ n ] != _lastActivated )
         {
            (*_manipulators[ n ]).draw( slice, false );
         }
      }

      if ( _wasActivated )
      {
         (*_lastActivated).draw( slice, true );
      }
   }

   bool ToolManipulatorsPointer::checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e )
   {
      Sliceuc& slice = s.segment.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return false;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
      nll::core::vector2f p2 = slice.worldToSliceCoordinate( p1 );

      nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                static_cast<int>( slice.size()[ 1 ] ) );
      nll::core::vector2i pi( static_cast<int>( p2[ 0 ] + size[ 0 ] / 2 ),
                              static_cast<int>( p2[ 1 ] + size[ 1 ] / 2 ) );
      nll::core::vector2f diff( (float)positionEndStroke[ 0 ] - (float)positionStartStroke[ 0 ],
                                (float)positionEndStroke[ 1 ] - (float)positionStartStroke[ 1 ] );

      bool canBeActivated = abs( (int)positionStartStroke[ 0 ] - pi[ 0 ] ) < _nbPixelForSelection || abs( (int)positionStartStroke[ 1 ] - pi[ 1 ] ) < _nbPixelForSelection;
      if ( ( canBeActivated && !e.isMouseRightButtonPressed && !_wasPanning ) )
      {
         if ( e.isMouseLeftButtonPressed )
         {
            nll::core::vector3f vv( diff[ 0 ] * slice.getAxisX()[ 0 ] * slice.getSpacing()[ 0 ] + diff[ 1 ] * slice.getAxisY()[ 0 ] * slice.getSpacing()[ 1 ],
                                    diff[ 0 ] * slice.getAxisX()[ 1 ] * slice.getSpacing()[ 0 ] + diff[ 1 ] * slice.getAxisY()[ 1 ] * slice.getSpacing()[ 1 ],
                                    diff[ 0 ] * slice.getAxisX()[ 2 ] * slice.getSpacing()[ 0 ] + diff[ 1 ] * slice.getAxisY()[ 2 ] * slice.getSpacing()[ 1 ] );
            if ( vv.dot( vv ) > 1e-7 )
            {
               // we have moved the position of the pointer, meaning we need to move all the attached segments
               _needToSynchronizePosition = true;
               _segmentIssuingDispatch = &s;
            }
            _position += vv;
         }
         return true;
      }

      // we are not on the pointer, but left click, so pan it!
      if ( e.isMouseLeftButtonPressed && !e.isMouseRightButtonPressed )
      {
         nll::core::vector3f vv( s.panning.getValue()[ 0 ] - diff[ 0 ] * slice.getAxisX()[ 0 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 0 ] * slice.getSpacing()[ 1 ],
                                 s.panning.getValue()[ 1 ] - diff[ 0 ] * slice.getAxisX()[ 1 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 1 ] * slice.getSpacing()[ 1 ],
                                 s.panning.getValue()[ 2 ] - diff[ 0 ] * slice.getAxisX()[ 2 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 2 ] * slice.getSpacing()[ 1 ] );
         s.panning.setValue( vv );
         _wasPanning = true;
         return true;   // we need to return true even though we are not activating the pointer => this is to make sure we are not selecting another tool...
      }
      _wasPanning = false;

      if ( !e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
      {
         float sign = ( diff[ 1 ] > 0 ) ? 1.0f : -1.0f;
         float val = fabs( (float)diff[ 1 ] ) * sign / _panningFactor;
         _zmovementPointer = nll::core::vector3f( val * s.segment.getValue().getNormal()[ 0 ],
                                                  val * s.segment.getValue().getNormal()[ 1 ],
                                                  val * s.segment.getValue().getNormal()[ 2 ] );
         _zmovementNormal = s.segment.getValue().getNormal();
         _needToSynchronizeZPos = true;
         return true;  // we have updated the segment, so the pointer will be at the correct location (no need to issue a notify())
      }
      if ( e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
      {
         _zoomUpdate = (float)diff[ 1 ];
         _needToSynchronizeZoom = true;
         return true;
      }
      return false;
   }

   void ToolManipulatorsPointer::draw( ResourceSliceuc& s, bool isActivated )
   {
      nll::core::vector3uc color = isActivated && !_wasPanning ? _color : _colorInactif;
      Sliceuc& slice = s.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
      nll::core::vector2f p2 = slice.worldToSliceCoordinate( p1 );

      nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                static_cast<int>( slice.size()[ 1 ] ) );
      nll::core::vector2i pi( static_cast<int>( p2[ 0 ] + size[ 0 ] / 2 ),
                              static_cast<int>( p2[ 1 ] + size[ 1 ] / 2 ) );

      if ( slice.contains( p2 ) )
      {
         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( pi[ 0 ], 0 );
         for ( ui32 n = 0; n < slice.size()[ 1 ]; ++n, it.addy() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }

         it = slice.getIterator( 0, pi[ 1 ] );
         for ( ui32 n = 0; n < slice.size()[ 0 ]; ++n, it.addx() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }

      std::stringstream ss;
      ss << "position=(" << _position[ 0 ] << ", " << _position[ 1 ] << ", " << _position[ 2 ] << ") mm";
      _font.setSize( _fontSize );
      _font.setColor( _fontColor );
      _font.write( ss.str(), nll::core::vector2ui( 0, 0 ), slice.getStorage() );
   }

   void ToolManipulatorsPointer::dispatch( std::set<Segment*>& segments )
   {
      if ( _needToSynchronizePosition )
      {
         // project the position on attached segments
         for ( std::set<Segment*>::iterator it = segments.begin(); it != segments.end(); ++it )
         {
            if ( _segmentIssuingDispatch != *it )
            {               
               nll::core::GeometryPlane plane( (**it).position.getValue(), (**it).directionx.getValue(), (**it).directiony.getValue() );
               nll::core::vector3f position = plane.getOrthogonalProjection( _position );
               nll::core::vector3f diff( position[ 0 ] - _position[ 0 ],
                                         position[ 1 ] - _position[ 1 ],
                                         position[ 2 ] - _position[ 2 ] );
               nll::core::vector3f final( (**it).position.getValue()[ 0 ] - diff[ 0 ],
                                          (**it).position.getValue()[ 1 ] - diff[ 1 ],
                                          (**it).position.getValue()[ 2 ] - diff[ 2 ] );
               (**it).position.setValue( final );
            }
         }

         _needToSynchronizePosition = false;
      }

      if ( _needToSynchronizeZoom )
      {
         for ( std::set<Segment*>::iterator it = segments.begin(); it != segments.end(); ++it )
         {
            nll::core::vector2f zoom;
            if ( _zoomUpdate > 0 )
            {
               zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] * (float)( 1 + fabs( _zoomUpdate ) / _zoomingFactor );
               zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] * (float)( 1 + fabs( _zoomUpdate ) / _zoomingFactor );
            } else {
               zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] / (float)( 1 + fabs( _zoomUpdate ) / _zoomingFactor );
               zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] / (float)( 1 + fabs( _zoomUpdate ) / _zoomingFactor );
            }
            (**it).zoom.setValue( zoom );
         }
         _needToSynchronizeZoom = false;
      }

      if ( _needToSynchronizeZPos )
      {
         // we need to update all parrallele segments
         for ( std::set<Segment*>::iterator it = segments.begin(); it != segments.end(); ++it )
         {
            Segment& s = **it;
            if ( nll::core::isCollinear( (**it).segment.getValue().getNormal(), _zmovementPointer ) )
            {
               float val = static_cast<float>( (**it).segment.getValue().getNormal().dot( _zmovementNormal ) );
               nll::core::vector3f pos( s.position.getValue()[ 0 ] + _zmovementPointer[ 0 ] * val,
                                        s.position.getValue()[ 1 ] + _zmovementPointer[ 1 ] * val,
                                        s.position.getValue()[ 2 ] + _zmovementPointer[ 2 ] * val );
               s.position.setValue( pos );
            }
         }

         _position = nll::core::vector3f( _zmovementPointer[ 0 ] + _position[ 0 ],
                                          _zmovementPointer[ 1 ] + _position[ 1 ],
                                          _zmovementPointer[ 2 ] + _position[ 2 ] );
         _needToSynchronizeZPos = false;
      }
   }

   bool ToolManipulatorsCuboid::checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e )
   {
      // project both points on the segment
      Sliceuc& slice = s.segment.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return false;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _min );
      nll::core::vector2f p1a = slice.worldToSliceCoordinate( p1 );

      nll::core::vector3f p2 = slice.getOrthogonalProjection( _max );
      nll::core::vector2f p2a = slice.worldToSliceCoordinate( p2 );

      nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                static_cast<int>( slice.size()[ 1 ] ) );
      nll::core::vector2i pi1( static_cast<int>( p1a[ 0 ] + size[ 0 ] / 2 ),
                               static_cast<int>( p1a[ 1 ] + size[ 1 ] / 2 ) );
      nll::core::vector2i pi2( static_cast<int>( p2a[ 0 ] + size[ 0 ] / 2 ),
                               static_cast<int>( p2a[ 1 ] + size[ 1 ] / 2 ) );

      // order the points
      nll::core::vector2i a1( std::min( pi1[ 0 ], pi2[ 0 ] ),
                              std::min( pi1[ 1 ], pi2[ 1 ] ) );
      nll::core::vector2i a2( std::max( pi1[ 0 ], pi2[ 0 ] ),
                              std::max( pi1[ 1 ], pi2[ 1 ] ) );

      nll::core::vector2i start( std::max<int>( 0, a1[ 0 ] ),
                                 std::max<int>( 0, a1[ 1 ] ) );
      nll::core::vector2i end( std::min<int>( size[ 0 ], a2[ 0 ] ),
                               std::min<int>( size[ 1 ], a2[ 1 ] ) );

      // localization
      bool a00 = a1[ 0 ] >= 0 && a1[ 1 ] >= 0 && a1[ 0 ] < size[ 0 ] && a1[ 1 ] < size[ 1 ];
      bool a10 = a1[ 0 ] >= 0 && a2[ 1 ] >= 0 && a1[ 0 ] < size[ 0 ] && a2[ 1 ] < size[ 1 ];
      bool a01 = a2[ 0 ] >= 0 && a1[ 1 ] >= 0 && a2[ 0 ] < size[ 0 ] && a1[ 1 ] < size[ 1 ];
      bool a11 = a2[ 0 ] >= 0 && a2[ 1 ] >= 0 && a2[ 0 ] < size[ 0 ] && a2[ 1 ] < size[ 1 ];

      // draw
      const int sx = end[ 0 ] - start[ 0 ];
      const int sy = end[ 1 ] - start[ 1 ];
      const int asx = a2[ 0 ] - a1[ 0 ];
      const int asy = a2[ 1 ] - a1[ 1 ];

      nll::core::vector2i center( ( a1[ 0 ] + a2[ 0 ] ) / 2,
                                  ( a1[ 1 ] + a2[ 1 ] ) / 2 );

      nll::core::vector2f diff(     (float)positionEndStroke[ 0 ] - (float)positionStartStroke[ 0 ],
                                - ( (float)positionEndStroke[ 1 ] - (float)positionStartStroke[ 1 ] ) );

      nll::core::vector3f vv( diff[ 0 ] * slice.getAxisX()[ 0 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 0 ] * slice.getSpacing()[ 1 ],
                                    diff[ 0 ] * slice.getAxisX()[ 1 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 1 ] * slice.getSpacing()[ 1 ],
                                    diff[ 0 ] * slice.getAxisX()[ 2 ] * slice.getSpacing()[ 0 ] - diff[ 1 ] * slice.getAxisY()[ 2 ] * slice.getSpacing()[ 1 ] );

      // check if close to a corner
      int nbPixel = 5;
      if ( abs( pi1[ 0 ] - positionStartStroke[ 0 ] ) < 5 &&
           abs( pi1[ 1 ] - positionStartStroke[ 1 ] ) < 5 )
      {
         if ( e.isMouseLeftButtonPressed )
         {
            _min[ 0 ] += vv[ 0 ];
            _min[ 1 ] += vv[ 1 ];
            _min[ 2 ] += vv[ 2 ];
         }
         return true;
      }

      if ( abs( pi2[ 0 ] - positionStartStroke[ 0 ] ) < 5 &&
           abs( pi2[ 1 ] - positionStartStroke[ 1 ] ) < 5 )
      {
         if ( e.isMouseLeftButtonPressed )
         {
            _max[ 0 ] += vv[ 0 ];
            _max[ 1 ] += vv[ 1 ];
            _max[ 2 ] += vv[ 2 ];
         }
         return true;
      }

      // check if at center
      if ( abs( positionStartStroke[ 0 ] - center[ 0 ] ) < asx / 8 &&
           abs( positionStartStroke[ 1 ] - center[ 1 ] ) < asy / 8 )
      {
         if ( e.isMouseLeftButtonPressed )
         {
            _max[ 0 ] += vv[ 0 ];
            _max[ 1 ] += vv[ 1 ];
            _max[ 2 ] += vv[ 2 ];

            _min[ 0 ] += vv[ 0 ];
            _min[ 1 ] += vv[ 1 ];
            _min[ 2 ] += vv[ 2 ];
         }
         return true;
      }
      return false;
   }

   void ToolManipulatorsCuboid::draw( ResourceSliceuc& s, bool isActivated )
   {
      // project both points on the segment
      nll::core::vector3uc color = isActivated ? _color : _colorInactif;
      Sliceuc& slice = s.getValue();
      if ( !slice.size()[ 0 ] ||
           !slice.size()[ 1 ] ||
           !slice.size()[ 2 ] ||
           slice.getAxisX().norm2() < 1e-4 || 
           slice.getAxisY().norm2() < 1e-4 )
      {
         return;
      }

      nll::core::vector3f p1 = slice.getOrthogonalProjection( _min );
      nll::core::vector2f p1a = slice.worldToSliceCoordinate( p1 );

      nll::core::vector3f p2 = slice.getOrthogonalProjection( _max );
      nll::core::vector2f p2a = slice.worldToSliceCoordinate( p2 );

      nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                static_cast<int>( slice.size()[ 1 ] ) );
      nll::core::vector2i pi1( static_cast<int>( p1a[ 0 ] + size[ 0 ] / 2 ),
                               static_cast<int>( p1a[ 1 ] + size[ 1 ] / 2 ) );
      nll::core::vector2i pi2( static_cast<int>( p2a[ 0 ] + size[ 0 ] / 2 ),
                               static_cast<int>( p2a[ 1 ] + size[ 1 ] / 2 ) );

      // order the points
      nll::core::vector2i a1( std::min( pi1[ 0 ], pi2[ 0 ] ),
                              std::min( pi1[ 1 ], pi2[ 1 ] ) );
      nll::core::vector2i a2( std::max( pi1[ 0 ], pi2[ 0 ] ),
                              std::max( pi1[ 1 ], pi2[ 1 ] ) );

      nll::core::vector2i start( std::max<int>( 0, a1[ 0 ] ),
                                 std::max<int>( 0, a1[ 1 ] ) );
      nll::core::vector2i end( std::min<int>( size[ 0 ], a2[ 0 ] ),
                               std::min<int>( size[ 1 ], a2[ 1 ] ) );

      // localization
      bool a00 = a1[ 0 ] >= 0 && a1[ 1 ] >= 0 && a1[ 0 ] < size[ 0 ] && a1[ 1 ] < size[ 1 ];
      bool a10 = a1[ 0 ] >= 0 && a2[ 1 ] >= 0 && a1[ 0 ] < size[ 0 ] && a2[ 1 ] < size[ 1 ];
      bool a01 = a2[ 0 ] >= 0 && a1[ 1 ] >= 0 && a2[ 0 ] < size[ 0 ] && a1[ 1 ] < size[ 1 ];
      bool a11 = a2[ 0 ] >= 0 && a2[ 1 ] >= 0 && a2[ 0 ] < size[ 0 ] && a2[ 1 ] < size[ 1 ];

      // draw
      const int sx = end[ 0 ] - start[ 0 ];
      const int sy = end[ 1 ] - start[ 1 ];

      ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( 0, 0 );
      if ( a00 || a10 )
      {
         it = slice.getIterator( start[ 0 ], start[ 1 ] );
         for ( int n = 0; n < sy; ++n, it.addy() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }

      if ( a00 || a01 )
      {
         it = slice.getIterator( start[ 0 ], start[ 1 ] );
         for ( int n = 0; n < sx; ++n, it.addx() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }

      if ( a10 || a11 )
      {
         it = slice.getIterator( start[ 0 ], end[ 1 ] );
         for ( int n = 0; n < sx; ++n, it.addx() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }

      if ( a01 || a11 )
      {
         it = slice.getIterator( end[ 0 ], start[ 1 ] );
         for ( int n = 0; n < sy; ++n, it.addy() )
         {
            it.pickcol( 0 ) = color[ 0 ];
            it.pickcol( 1 ) = color[ 1 ];
            it.pickcol( 2 ) = color[ 2 ];
         }
      }
   }
}
}