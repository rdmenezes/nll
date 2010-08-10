#ifndef MVV_PLATFORM_LAYOUT_PANE_H_
# define MVV_PLATFORM_LAYOUT_PANE_H_

# include "mvvPlatform.h"
# include "event-mouse-receiver.h"
# include "event-keyboard-receiver.h"
# include "refcounted.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Represent a region of the screen, able to react to events and draw in a buffer

    A pane can contains nested sub panes and other panes that will be overlayed on this pane.
    */
   class MVVPLATFORM_API Pane : public EventMouseReceiver, public EventKeyboardReceiver
   {
   public:
      typedef RefcountedTyped<Pane>    PaneRef;
      typedef std::vector<PaneRef>     Panes;

   public:
      Pane( const nll::core::vector2ui& origin,
            const nll::core::vector2ui& size,
            const nll::core::vector3uc background = nll::core::vector3uc( 255, 255, 255 ) );

      virtual ~Pane()
      {
         std::cout << "pane=" << this << " destroyed" << std::endl;
      }

      // signal the layout that they are going to be killed...
      // we need this as if we interface with compiler, the user will have several references on the layouts
      // wich makes it impossible to destroy...
      virtual void destroy() = 0;

      void insert( PaneRef widget )
      {
         _widgets.push_back( widget );
      }

      void erase( PaneRef widget )
      {
         Panes::iterator it = std::find( _widgets.begin(), _widgets.end(), widget );
         if ( it != _widgets.end() )
         {
            _widgets.push_back( widget );
         }
      }

      /**
       @brief return the origin of the pane
       */
      const nll::core::vector2ui& getOrigin() const;

      /**
       @brief return the size of the pane
       */
      const nll::core::vector2ui& getSize() const;

      /**
       @brief Returns true if the position given is inside the pane
       @param pos the position in the whole screen (not only this pane)
       */
      bool isInside( const nll::core::vector2ui& pos );

      /**
       @brief set if this pane is visible or not.
       */
      void setVisible( bool isVisible );

      /**
       @return true if visible
       */
      bool isVisible() const;

      /**
       @brief Draw into the image the pane. It is expected that <b>only</b> the part of the image
              located at <code>origin</code> with a size <code>size</code> should be modified.
              it must use _origin, the origin in pixel where the Pane should be rendered
              and _size the size of the pane in pixel to be rendered
       @param image the global image of the pane.
       */
      virtual void _draw( Image& image ) = 0;

      virtual void draw( Image& image );

      /**
       @brief Update the layout in case the child changed origin/size/visibility/inactivation
       */
      virtual void updateLayout() = 0;

      /**
       @brief set the origin of the plane. (0, 0) is the bottom left
       */
      void setOrigin( const nll::core::vector2ui& origin );

      /**
       @brief set the size of the pane in pixel
       */
      void setSize( const nll::core::vector2ui& size );

      void setBackground( const nll::core::vector3uc& col )
      {
         _background = col;
      }


      /**
       @brief If return true, the last inserted Widget of a pane only will receive the event
       */
      virtual bool dispatchMouseEventToWidgetOnly( const EventMouse& ) const
      {
         return false;
      }

      /**
       @brief If return true, the last inserted Widget of a pane only will receive the event
       */
      virtual bool dispatchMouseEventToWidgetOnly( const EventKeyboard& ) const
      {
         return false;
      }

      virtual void receive( const EventMouse& event )
      {
         for ( Panes::reverse_iterator it = _widgets.rbegin(); it != _widgets.rend(); ++it )
         {
            if ( (**it).dispatchMouseEventToWidgetOnly( event ) )
            {
               // we dispatch it to the child only, it is prioritary
               (**it).receive( event );
               return;
            }
         }

         // dispatch it to all widgets and sub panes
         for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
            (**it)._receive( event );
         _receive( event );
      }

      virtual void receive( const EventKeyboard& event )
      {
         for ( Panes::reverse_iterator it = _widgets.rbegin(); it != _widgets.rend(); ++it )
         {
            if ( (**it).dispatchMouseEventToWidgetOnly( event ) )
            {
               // we dispatch it to the child only, it is prioritary
               (**it).receive( event );
               return;
            }
         }

         // dispatch it to all widgets and sub panes
         for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
            (**it)._receive( event );
         _receive( event );
      }

   protected:
      /**
       @brief reimplement this method to handle mouse events
       */
      virtual void _receive( const EventMouse& )
      {
         // just do nothing yet...
      }

      virtual void _receive( const EventKeyboard& )
      {
         // just do nothing yet...
      }

   protected:
      bool                    _visible;
      nll::core::vector2ui    _origin;
      nll::core::vector2ui    _size;
      bool                    _disabled;

      Panes                   _widgets;
      nll::core::vector3uc    _background;
   };

   /**
    @ingroup platform
    @brief Base class that holds a list of pane to display. draw() and updateLayout() method must be implemented
    */
   class PaneList : public Pane
   {
   public:
      PaneList( const nll::core::vector2ui& origin,
                const nll::core::vector2ui& size ) : Pane( origin, size )
      {}


      /**
       @brief Dispatch the event to all panes
       */
      virtual void handle( const EventMouse& event )
      {
         if ( _disabled || !_visible)
            return;

         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
         {
            (**it).receive( event );
         }
      }

      void _draw( Image& image )
      {
         if ( !_visible )
            return;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
            ( **it ).draw( image );
      }

      virtual void destroy()
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
            (**it).destroy();
      }

   protected:
      /**
       @brief add a child to display. It must be an allocated pointer. It will be automatically deallocated by this
              class when it is destroyed
       */
      void addChild( PaneRef pane )
      {
         _panes.push_back( pane );
      }

      virtual void _receive( const EventMouse& e )
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
         {
            if ( e.isMouseLeftButtonPressed )
            {
               if ( (**it).isInside( e.mouseLeftClickedPosition ) )
               {
                  // propagate to the sub pane concerned
                  (**it).receive( e );
               }
            } else if ( e.isMouseRightButtonPressed )
            {
               if ( (**it).isInside( e.mouseRightClickedPosition ) )
               {
                  // propagate to the sub pane concerned
                  (**it).receive( e );
               }
            } else if ( (**it).isInside( e.mousePosition ) )
            {
               (**it).receive( e );
            }
         }
      }

      virtual void _receive( const EventKeyboard& e )
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
         {
            //if ( (**it).isInside( e.mousePosition ) )
            {
               (**it).receive( e );
            }
         }
      }

   protected:
      Panes    _panes;
   };

   /**
    @ingroup platform
    @brief Arrange Horizontally panes
    */
   class PaneListHorizontal : public PaneList
   {
      typedef std::vector<double>   Ratios;
      typedef PaneList              Base;

   public:
      PaneListHorizontal( const nll::core::vector2ui& origin,
                          const nll::core::vector2ui& size ) : PaneList( origin, size )
      {}

      /**
       @brief add a child to display. The sum of the child's ratio must be equal to 1
       */
      void addChild( PaneRef pane, double ratio )
      {
         Base::addChild( pane );
         _ratios.push_back( ratio );
      }

      /**
       @brief Reorganize horizontally the panes
       */
      virtual void updateLayout()
      {
         double checkRatio = 0;
         double ratioToDraw = 0;
         ui32 n = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
         {
            checkRatio += _ratios[ n ];
            if ( ( **it ).isVisible() )
               ratioToDraw += _ratios[ n ];
         }
         ensure( nll::core::equal( checkRatio, 1.0, 1e-6 ), "must sum to 1" );

         // reorganize
         n = 0;
         ui32 ratioShift = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
            if ( ( **it ).isVisible() )
            {
               PaneRef p = *it;
               double ratio = _ratios[ n ] / ratioToDraw;
               (*p).setOrigin( nll::core::vector2ui( getOrigin()[ 0 ] + ratioShift, getOrigin()[ 1 ] ) );
               (*p).setSize( nll::core::vector2ui( (ui32)nll::core::round( ratio * getSize()[ 0 ] ), getSize()[ 1 ] ) );
               ratioShift += (*p).getSize()[ 0 ];
               (*p).updateLayout();
            }
      }

   protected:
      Ratios   _ratios;
   };

   /**
    @ingroup platform
    @brief Arrange vertically panes
    */
   class PaneListVertical : public PaneList
   {
      typedef std::vector<double>   Ratios;
      typedef PaneList              Base;

   public:
      PaneListVertical( const nll::core::vector2ui& origin,
                        const nll::core::vector2ui& size ) : PaneList( origin, size )
      {}

      /**
       @brief add a child to display. The sum of the child's ratio must be equal to 1
       */
      void addChild( PaneRef pane, double ratio )
      {
         Base::addChild( pane );
         _ratios.push_back( ratio );
      }

      void setRatio( ui32 pane, double ratio )
      {
         _ratios[ pane ] = ratio;
      }

      /**
       @brief Reorganize vertically the panes
       */
      virtual void updateLayout()
      {
         double checkRatio = 0;
         double ratioToDraw = 0;
         ui32 n = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
         {
            checkRatio += _ratios[ n ];
            if ( ( **it ).isVisible() )
               ratioToDraw += _ratios[ n ];
         }
         ensure( nll::core::equal( checkRatio, 1.0, 1e-6 ), "must sum to 1" );

         // reorganize
         n = 0;
         ui32 ratioShift = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
            if ( ( **it ).isVisible() )
            {
               PaneRef p = *it;
               double ratio = _ratios[ n ] / ratioToDraw;
               (*p).setOrigin( nll::core::vector2ui( getOrigin()[ 0 ], getOrigin()[ 1 ] + ratioShift ) );
               (*p).setSize( nll::core::vector2ui( getSize()[ 0 ], (ui32)nll::core::round( ratio * getSize()[ 1 ] ) ) );
               ratioShift += (*p).getSize()[ 1 ];
               (*p).updateLayout();
            }
      }

   protected:
      Ratios   _ratios;
   };

   class PaneEmpty : public Pane
   {
   public:
      PaneEmpty( const nll::core::vector2ui& origin,
                 const nll::core::vector2ui& size,
                 const nll::core::vector3uc background ) : Pane( origin, size, background )
      {}

      virtual void destroy()
      {
      }

      virtual void updateLayout()
      {}

      void _draw( Image& image )
      {
         for ( ui32 y = 0; y < getSize()[ 1 ]; ++y )
         {
            Image::DirectionalIterator output = image.getIterator( getOrigin()[ 0 ], y + getOrigin()[ 1 ], 0 );
            for ( ui32 x = 0; x < getSize()[ 0 ]; ++x )
            {
               output.pickcol( 0 ) = _background[ 0 ];
               output.pickcol( 1 ) = _background[ 1 ];
               output.pickcol( 2 ) = _background[ 2 ];
               ++output;
            }
         }
      }
   };
}
}

#endif
