#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_

# include "segment-tool-pointer.h"
# include <mvvPlatform/font.h>

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolPointer : public SegmentTool
   {
      typedef std::map<Segment*, bool> MapSegments;

      class SegmentPositionListener : public Engine
      {
      public:
         SegmentPositionListener( SegmentToolPointer& pointer, nll::core::vector3f& position,  Segment& segment, EngineHandler& handler ) : Engine( handler ), _pointer( pointer ), _position( position ), _segment( segment )
         {
            segment.position.connect( this );
         }

         virtual void unnotify()
         {
            _needToRecompute = false;
         }

         virtual bool _run()
         {
            if ( _pointer.interceptEvent() )
            {
               // we have the control, and the pointer is likely to be moving, so don't update any position else
               // it will create moving artifacts.
               return true;
            }

            // a segment position has been updated, we want to set the position of the pointer on the normal-axis coordinate only            
            Sliceuc& slice = _segment.segment.getValue();
            if ( slice.getNormal().dot( slice.getNormal() ) > 1e-4 )
            {
               // we need to use the segment position... and not the slice! Indeed it might not be in sync while moving!
               nll::core::GeometryPlane plane( _segment.position.getValue(), _segment.directionx.getValue(), _segment.directiony.getValue() );
               _position = plane.getOrthogonalProjection( _position );
               _pointer.refreshConnectedSegments();
               _pointer.unnotifyOtherEngines( this );
               _segment.refreshTools();
               return true;
            } else {
               // in case no volume is loaded
               return false;
            }
         }

         const Segment* getSegment() const
         {
            return &_segment;
         }

      private:
         Segment&                _segment;
         nll::core::vector3f&    _position;
         SegmentToolPointer&     _pointer;
         bool                    _hasBeenInitialized;
      };

   public:
      SegmentToolPointer( Font& font, ui32 fontSize, EngineHandler& handler ) : SegmentTool( true ), _font( font ), _fontSize( fontSize ), _handler( handler )
      {
      }

      virtual bool isSavingMprImage() const
      {
         return true;
      }

      virtual bool interceptEvent() const
      {
         // if one segment is active, then we must intercept the signal. Only one tool
         // is allowed to handle a signal
         for ( MapSegments::const_iterator it = _active.begin(); it != _active.end(); ++it )
         {
            if ( it->second )
               return true;
         }
         return false;
      }

      bool isSegmentActive( Segment* s ) const
      {
         MapSegments::const_iterator it = _active.find( s );
         if ( it != _active.end() )
            return it->second;
         return false;
      }

      virtual f32 priority() const
      {
         return 100;
      }

      virtual void updateSegment( ResourceSliceuc segment, Segment& s )
      {
         ResourceSliceuc::value_type& slice = segment.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] || !slice.size()[ 2 ] )
            return;

         bool isHighlighted = false;
         MapSegments::iterator ii = _active.find( &s );
         if ( ii != _active.end() && ii->second )
            isHighlighted = true;

         nll::core::vector3f p = slice.getOrthogonalProjection( _position );
         nll::core::vector2f pplane = slice.worldToSliceCoordinate( p );
         try
         {
            if ( !slice.contains( pplane ) )
            {
               // out of the slice, don't display anything
               return;
            }
         }
         catch( ... )
         {
            return;
         }

         ui8 val = isHighlighted ? 255 : 180;
         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( static_cast<ui32>( pplane[ 0 ] + slice.size()[ 0 ] / 2 ),                                                                       0 );
         for ( ui32 n = 0; n < slice.size()[ 1 ]; ++n, it.addy() )
         {
            *it = val;
         }

         it = slice.getIterator( 0, static_cast<ui32>( pplane[ 1 ] + slice.size()[ 1 ] / 2 ) );
         for ( ui32 n = 0; n < slice.size()[ 0 ]; ++n, ++it )
         {
            *it = val;
         }

         std::stringstream ss;
         ss << "position=(" << _position[ 0 ] << ", " << _position[ 1 ] << ", " << _position[ 2 ] << ") mm";
         _font.setSize( _fontSize );
         _font.setColor( nll::core::vector3uc( 255, 255, 255 ) );
         _font.write( ss.str(), nll::core::vector2ui( 0, 0 ), slice.getStorage() );
      }

      void refreshConnectedSegments()
      {
         for ( LinkStorage::iterator it = _links.begin(); it != _links.end(); ++it )
         {
            (*it)->refreshTools();
         }
      }

      void unnotifyOtherEngines( SegmentPositionListener* e )
      {
         typedef std::vector< RefcountedTyped<SegmentPositionListener> > Links;
         for ( Links::iterator it = _positionListeners.begin(); it != _positionListeners.end(); ++it )
         {
            if ( (*it).getDataPtr() == e )
               continue;
            (**it).unnotify();
         }
      }

      virtual void receive( Segment& segment, const EventMouse& e, const nll::core::vector2ui& windowOrigin )
      {
         if ( e.isMouseLeftButtonJustPressed )
         {
            _leftMouseLastPos = e.mousePosition;
         }

         // if the current segment is active and left mouse button pressed, update the
         // camera position
         MapSegments::iterator it = _active.find( &segment );
         if ( it != _active.end() && it->second && e.isMouseLeftButtonPressed )
         {
            nll::core::vector2i diffMouse( - (int)e.mousePosition[ 0 ] + (int)_leftMouseLastPos[ 0 ],
                                           - (int)e.mousePosition[ 1 ] + (int)_leftMouseLastPos[ 1 ] );
            if ( diffMouse[ 0 ] == 0 && diffMouse[ 1 ] == 0 )
            {
               // no displacement
               return;
            }

            nll::core::vector3f directionx = segment.directionx.getValue();
            nll::core::vector3f directiony = segment.directiony.getValue();
            nll::core::vector2f zoom = segment.zoom.getValue();

            nll::core::vector3f pos( _position[ 0 ] - ( diffMouse[ 0 ] * directionx[ 0 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 0 ] / zoom[ 1 ] ),
                                     _position[ 1 ] - ( diffMouse[ 0 ] * directionx[ 1 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 1 ] / zoom[ 1 ] ),
                                     _position[ 2 ] - ( diffMouse[ 0 ] * directionx[ 2 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 2 ] / zoom[ 1 ] ) );
            _leftMouseLastPos = e.mousePosition;

            for ( LinkStorage::iterator it = _links.begin(); it != _links.end(); ++it )
            {
               if ( *it == &segment )
               {
                  // we will refresh the current segment anyway...
                  continue;
               }
               try
               {
                  // update the position in other slices...
                  Sliceuc& slice = (*it)->segment.getValue();
                  nll::core::vector3f vec = pos - _position;
                  float mag = sqrtf( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
                  if ( fabs( mag ) > 1e-8 )
                  {
                     nll::core::vector3f displacement = slice.getNormal() * slice.getNormal().dot( vec );
                     if ( displacement.dot( displacement ) < 1e-2 )
                     {
                        // if no displacement, we need at least to refresh the tool as it has been moved but on the
                        // same plane than the slice
                        (*it)->refreshTools();
                     }
                     else
                     {
                        // refresh the slice: movement not in the same plane than the slice
                        (*it)->position.setValue( (*it)->position.getValue() + displacement );
                     }
                  }
               } catch( ... ) {
                  // skip, unknow error!
               }
            }

            _position = pos;
            segment.refreshTools();
            return;
         }

         // if mouse is pressed, we don't want to update the activation/deactivation state
         // (as it is not in syn, we can move the pointer even if the user didn't select it)
         if ( e.isMouseLeftButtonPressed || e.isMouseRightButtonPressed )
            return;


         // project the point on the plane
         ResourceSliceuc::value_type slice = segment.segment.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] || !slice.size()[ 2 ] )
            return;

         nll::core::vector3f p = slice.getOrthogonalProjection( _position );
         nll::core::vector2f pplane = slice.worldToSliceCoordinate( p );
         pplane[ 0 ] += static_cast<f32>( slice.size()[ 0 ] ) / 2;
         pplane[ 1 ] += static_cast<f32>( slice.size()[ 1 ] ) / 2;

         //MapSegments::iterator it = _active.find( &segment );
         bool justMakeItActive = false;
         if ( it == _active.end() || !it->second )
         {
            // the crosshair may possibly needs to be highlighted
            if ( abs( pplane[ 0 ] - ( e.mousePosition[ 0 ] - windowOrigin[ 0 ] ) ) < 10 ||
                 abs( pplane[ 1 ] - ( e.mousePosition[ 1 ] - windowOrigin[ 1 ] ) ) < 10 )
            {
               justMakeItActive = true;
               _active[ &segment ] = true;
               segment.refreshTools();
            }
         }

         if ( !justMakeItActive && !e.isMouseLeftButtonPressed && !e.isMouseRightButtonPressed )
         {
            // we know none should be highlighted
            for ( MapSegments::iterator it = _active.begin(); it != _active.end(); ++it )
            {
               bool isOutside = abs( pplane[ 0 ] - ( e.mousePosition[ 0 ] - windowOrigin[ 0 ] ) ) >= 10 &&
                                abs( pplane[ 1 ] - ( e.mousePosition[ 1 ] - windowOrigin[ 1 ] ) ) >= 10;
               if ( ( isOutside || it->first != &segment ) && it->second )
               {
                  SegmentTool::LinkStorage::iterator ii = _links.find( it->first );
                  it->second = false;
                  if ( ii != _links.end() )
                  {
                     (*ii)->refreshTools();
                  }
                  break;
               }
            }
         }
         
      }

      virtual void _addSimpleLink( Segment* o )
      {
         std::pair<LinkStorage::iterator, bool> r = _links.insert( o );
         if ( r.second )
         {
            _positionListeners.push_back( RefcountedTyped<SegmentPositionListener>( new SegmentPositionListener( *this, _position, *o, _handler ) ) );
         }
      }

      virtual void _eraseSimpleLink( Segment* o )
      {
         MapSegments::iterator ii = _active.find( o );
         if ( ii != _active.end() )
            _active.erase( ii );
         LinkStorage::iterator it = _links.find( o );
         if ( it != _links.end() )
            _links.erase( it );

         typedef std::vector< RefcountedTyped<SegmentPositionListener> > Container;
         for ( Container::iterator it = _positionListeners.begin(); it != _positionListeners.end(); ++it )
         {
            if ( (**it).getSegment() == o )
            {
               _positionListeners.erase( it );
               break;
            }
         }
      }

      void setPosition( const nll::core::vector3f& p )
      {
         _position = p;
      }

   private:
      // copy disabled
      SegmentToolPointer& operator=( const SegmentToolPointer& );
      SegmentToolPointer( const SegmentToolPointer& );

   protected:
      Font&                   _font;
      ui32                    _fontSize;
      nll::core::vector3f     _position;
      MapSegments             _active;
      nll::core::vector2ui    _leftMouseLastPos;

      EngineHandler&          _handler;
      std::vector< RefcountedTyped<SegmentPositionListener> > _positionListeners;
   };
}
}

#endif