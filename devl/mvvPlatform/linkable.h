#ifndef MVV_PLATFORM_LINKABLE_H_
# define MVV_PLATFORM_LINKABLE_H_

# pragma warning( push )
# pragma warning( disable:4355 ) // 'this' used in constructor, but this is correct as we don't use it in the constructor

namespace mvv
{
namespace platform
{
   template <class ObjectToHold>
   class LinkableSimple
   {
      typedef std::set<ObjectToHold>   LinkStorage;

   public:
      LinkableSimple()
      {}

      void addSimpleLink( ObjectToHold o )
      {
         _links.insert( o );
      }

      void eraseSimpleLink( ObjectToHold o )
      {
         LinkStorage::iterator it = _links.find( o );
         if ( it != _links.end() )
            _links.erase( it );
      }

   private:
      // copy disabled
      LinkableSimple& operator=( LinkableSimple& );
      LinkableSimple( const LinkableSimple& );

   protected:
      LinkStorage _links;
   };

   /**
    @ingroup mvv
    @brief The derived object will hold a set of references of ObjectToHold and the ObjectToHold will have this derived object too
           so ObjectToHold knows about the derived class and the derived class knows about ObjectToHold.

     The destructor of the derived class must call 'removeConnections'!

     connect and disconnect must be both ways!
    */
   template <class ObjectToHold, class DerivedClass>
   class LinkableDouble : public LinkableSimple<ObjectToHold>
   {
   public:
      LinkableDouble() : _hasBeenDestructed( false )
      {}

      virtual ~LinkableDouble()
      {
         ensure( _hasBeenDestructed, "The method removeConnections() must be called when the object is destroyed" );
      }

      virtual void connect( ObjectToHold o ) = 0;

      virtual void disconnect( ObjectToHold o ) = 0;

      void removeConnections()
      {
         for ( LinkStorage::iterator it = _links.begin(); it != _links.end(); ++it )
         {
            disconnect( *it );
         }
         _hasBeenDestructed = true;
      }

   private:
      // copy disabled
      LinkableDouble& operator=( LinkableDouble& );
      LinkableDouble( const LinkableDouble& );

   protected:
      DerivedClass  _this;
      bool          _hasBeenDestructed;
   };
}
}

# pragma warning( pop )

#endif