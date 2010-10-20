#ifndef MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_

# include "segment-tool.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   /**
    @brief A manipulator interface

    The process is simple:
     - checkEvent will be called everytime there is a mouse event on this MPR, this function
       must use notify() to trigger a visual modification of the manipulator (return true)
     - draw will be called each time the manipulator needs to be displayed
    */
   class MVVMPRPLUGIN_API ToolManipulatorsInterface
   {
   public:
      virtual ~ToolManipulatorsInterface(){}

      /**
       @brief Returns true if the tool is interacting (i.e. it needs to be redrawn). In this case, at the next engine check,
              a redaw will be issued, updating all connected segments.
       @note that only one manipulator can be interacting at the same time
       @param it is FORBIDEN to draw on the segment... this must be done only in the draw() method
       @param positionStartStroke the position (x, y) of the mouse in the segment taken at the begining of the mouse mouvement
       @param positionEndStroke the position (x, y) of the mouse in the segment taken at the end of the mouse mouvement
       */
      virtual bool checkEvent( const ResourceSliceuc& slice, const nll::core::vector2ui& positionStartStroke, const nll::core::vector2ui& positionEndStroke, const EventMouse& e ) = 0;

      /**
       @brief actually draw on the slice
       @param isActivated if true, the manipulator must be displayed with 'activated' style
       */
      virtual void draw( ResourceSliceuc& slice, bool isActivated ) = 0;
   };

   class MVVMPRPLUGIN_API ToolManipulatorsPoint : public ToolManipulatorsInterface
   {
   public:
      ToolManipulatorsPoint( const nll::core::vector3f& position, nll::core::vector3uc color ) : _position( position ), _color( color )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( const ResourceSliceuc& s, const nll::core::vector2ui& positionStartStroke, const nll::core::vector2ui& positionEndStroke, const EventMouse& e )
      {
         Sliceuc& slice = const_cast<ResourceSliceuc&>( s ).getValue();
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

      virtual void draw( ResourceSliceuc& s, bool isActivated )
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
         std::cout << "MANIPULATOR DRAW=" << slice.getAxisX()[ 0 ] << slice.getAxisX()[ 1 ] << slice.getAxisX()[ 2 ] << " "
                                          << slice.getAxisY()[ 0 ] << slice.getAxisY()[ 1 ] << slice.getAxisY()[ 2 ] << std::endl;
      }

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
   };

   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools
    */
   class MVVMPRPLUGIN_API SegmentToolManipulators : public SegmentTool
   {
      typedef std::vector< RefcountedTyped<ToolManipulatorsInterface> > Manipulators;
   public:
      SegmentToolManipulators() : SegmentTool( true ), _wasActivated( false )
      {
      }

      void add( RefcountedTyped<ToolManipulatorsInterface> manip )
      {
         _manipulators.push_back( manip );
      }

      virtual void receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& origin );

      virtual f32 priority() const
      {
         return 101;
      }

      virtual bool isSavingMprImage() const
      {
         return true;
      }

      virtual void updateSegment( ResourceSliceuc slice, Segment& s )
      {
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

      virtual bool interceptEvent() const
      {
         return _wasActivated;
      }

   protected:
      Manipulators                                 _manipulators;
      nll::core::vector2ui                         _leftMouseLastPos;
      RefcountedTyped<ToolManipulatorsInterface>   _lastActivated;      // store the manipulator that must be drawn the latest
      bool                                         _wasActivated;
   };
}
}

#endif
