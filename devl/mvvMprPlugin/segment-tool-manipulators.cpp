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

   void SegmentToolManipulators::updateSegment( ResourceSliceuc& slice, Segment& s )
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

      bool canBeActivated = abs( (int)positionStartStroke[ 0 ] - pi[ 0 ] ) < 5 || abs( (int)positionStartStroke[ 1 ] - pi[ 1 ] ) < 5;
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
         return false;
      }
      _wasPanning = false;

      if ( !e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
      {
         float sign = ( diff[ 1 ] > 0 ) ? 1.0f : -1.0f;
         float d = fabs( (float)diff[ 1 ] ) * sign / 10.0;
         nll::core::vector3f pos( s.position.getValue()[ 0 ] + d * s.segment.getValue().getNormal()[ 0 ],
                                  s.position.getValue()[ 1 ] + d * s.segment.getValue().getNormal()[ 1 ],
                                  s.position.getValue()[ 2 ] + d * s.segment.getValue().getNormal()[ 2 ] );
         nll::core::vector3f posPointer( _position[ 0 ] + d * s.segment.getValue().getNormal()[ 0 ],
                                         _position[ 1 ] + d * s.segment.getValue().getNormal()[ 1 ],
                                         _position[ 2 ] + d * s.segment.getValue().getNormal()[ 2 ] );
         s.position.setValue( pos );
         _position = posPointer;
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
               Sliceuc& slice = (*it)->segment.getValue();
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
               zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] * (float)( 1 + fabs( _zoomUpdate ) / 100 );
               zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] * (float)( 1 + fabs( _zoomUpdate ) / 100 );
            } else {
               zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] / (float)( 1 + fabs( _zoomUpdate ) / 100 );
               zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] / (float)( 1 + fabs( _zoomUpdate ) / 100 );
            }
            (**it).zoom.setValue( zoom );
         }
         _needToSynchronizeZoom = false;
      }
   }
}
}