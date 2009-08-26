#ifndef MVV_LAYOUT_H_
# define MVV_LAYOUT_H_

# include "types.h"
# include "drawable.h"
# include <nll/nll.h>

namespace mvv
{
   /**
    @ingroup mvv
    @brief Base Class that hold a graphic display
    */
   class Pane
   {
   public:
      typedef nll::core::Image<ui8> Image;

   public:
      Pane( const nll::core::vector2ui& origin,
            const nll::core::vector2ui& size ) : _origin( origin ), _size( size ), _visible( true )
      {}

      virtual ~Pane()
      {}

      /**
       @brief return the origin of the pane
       */
      const nll::core::vector2ui& getOrigin() const
      {
         return _origin;
      }

      /**
       @brief return the size of the pane
       */
      const nll::core::vector2ui& getSize() const
      {
         return _size;
      }

      /**
       @brief set if this pane is visible or not.
       */
      void setVisible( bool isVisible )
      {
         _visible = isVisible;
      }

      /**
       @return true if visible
       */
      bool isVisible() const
      {
         return _visible;
      }

      /**
       @brief Draw into the image the pane. It is expected that <b>only</b> the part of the image
              located at <code>origin</code> with a size <code>size</code> should be modified.
              it must use _origin, the origin in pixel where the Pane should be rendered
              and _size the size of the pane in pixel to be rendered
       @param image the global image of the pane.
       */
      virtual void draw( Image& image ) = 0;

      /**
       @brief Update the layout in case the child changed origin/size/visibility
       */
      virtual void updateLayout() = 0;

   //protected: // TODO check visibility problem
      void setOrigin( const nll::core::vector2ui& origin )
      {
         _origin = origin;
      }

      void setSize( const nll::core::vector2ui& size )
      {
         _size = size;
      }

   protected:
      bool                    _visible;
      nll::core::vector2ui    _origin;
      nll::core::vector2ui    _size;
   };

   /**
    @ingroup mvv
    @brief Base class that holds a list of pane to display. draw() and updateLayout() method must be implemented
    */
   class PaneList : public Pane
   {
      typedef std::vector<Pane*> Panes;

   public:
      PaneList( const nll::core::vector2ui& origin,
                const nll::core::vector2ui& size ) : Pane( origin, size )
      {}

      virtual ~PaneList()
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
            delete *it;
      }

   protected:
      /**
       @brief add a child to display. It must be an allocated pointer. It will be automatically deallocated by this
              class when it is destroyed
       */
      void addChild( Pane* pane )
      {
         _panes.push_back( pane );
      }

   protected:
      Panes    _panes;
   };

   /**
    @ingroup mvv
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

      virtual void draw( Image& image )
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
            ( *it )->draw( image );
      }

      /**
       @brief add a child to display. The sum of the child's ratio must be equal to 1
       */
      void addChild( Pane* pane, double ratio )
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
            if ( ( *it )->isVisible() )
               ratioToDraw += _ratios[ n ];
         }
         ensure( nll::core::equal( checkRatio, 1.0, 1e-6 ), "must sum to 1" );

         // reorganize
         n = 0;
         ui32 ratioShift = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
            if ( ( *it )->isVisible() )
            {
               Pane* p = *it;
               double ratio = _ratios[ n ] / ratioToDraw;
               p->setOrigin( nll::core::vector2ui( getOrigin()[ 0 ] + ratioShift, getOrigin()[ 1 ] ) );
               p->setSize( nll::core::vector2ui( (ui32)nll::core::round( ratio * getSize()[ 0 ] ), getSize()[ 1 ] ) );
               ratioShift += p->getSize()[ 0 ];
               p->updateLayout();
            }
      }

   protected:
      Ratios   _ratios;
   };

   /**
    @ingroup mvv
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

      virtual void draw( Image& image )
      {
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it )
            ( *it )->draw( image );
      }

      /**
       @brief add a child to display. The sum of the child's ratio must be equal to 1
       */
      void addChild( Pane* pane, double ratio )
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
            if ( ( *it )->isVisible() )
               ratioToDraw += _ratios[ n ];
         }
         ensure( nll::core::equal( checkRatio, 1.0, 1e-6 ), "must sum to 1" );

         // reorganize
         n = 0;
         ui32 ratioShift = 0;
         for ( Panes::iterator it = _panes.begin(); it != _panes.end(); ++it, ++n )
            if ( ( *it )->isVisible() )
            {
               Pane* p = *it;
               double ratio = _ratios[ n ] / ratioToDraw;
               p->setOrigin( nll::core::vector2ui( getOrigin()[ 0 ], getOrigin()[ 1 ] + ratioShift ) );
               p->setSize( nll::core::vector2ui( getSize()[ 0 ], (ui32)nll::core::round( ratio * getSize()[ 1 ] ) ) );
               ratioShift += p->getSize()[ 1 ];
               p->updateLayout();
            }
      }

   protected:
      Ratios   _ratios;
   };

   /**
    @ingroup mvv
    @brief An actual Pane that can be rendered
    */
   class PaneDrawable : public Pane
   {
   public:
      PaneDrawable( Drawable& drawable,
                    const nll::core::vector2ui& origin,
                    const nll::core::vector2ui& size ) : Pane( origin, size ), _drawable( drawable )
      {}

      /**
       @brief draw the pane
       */
      virtual void draw( Image& image )
      {
         const Image& i = _drawable.draw();
         ensure( i.sizex() == getSize()[ 0 ], "must be the same size" );
      }

      /**
       @brief Update the layout in case the child changed origin/size/visibility
       */
      virtual void updateLayout()
      {
         _drawable.setImageSize( getSize()[ 0 ], getSize()[ 1 ] );
      }

   protected:
      Drawable&      _drawable;
   };
}

#endif