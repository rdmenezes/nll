#ifndef NLL_DATABASE_INPUT_ADAPTER_H_
# define NLL_DATABASE_INPUT_ADAPTER_H_

# pragma warning( push )
# pragma warning( disable:4512 ) // can't generate assignment operator

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Create a wrapper on a Database to match the interface of an
           array(considering Input of the database only) (define methods size() and operator[])

           It is used as a vector adapter on the database's inputs. A typical use is for 
           example to partionnate the database's inputs using a kd-tree as in ClassifierNearestNeighbor.

           While the adapter is used, the original database must be kept alive.
    @sa Database
    */
   template <class Database>
   class DatabaseInputAdapter
   {
   public:
      typedef typename Database::Sample::Input Point;

   public:
      DatabaseInputAdapter( Database& database ) : _database( database )
      {}
      const Point& operator[]( ui32 n ) const
      {
         assert( n < size() );
         return _database[ n ].input;
      }
      Point& operator[]( ui32 n )
      {
         assert( n < size() );
         return _database[ n ].input;
      }
      ui32 size() const
      {
         return _database.size();
      }

   private:
      Database&    _database;
   };

   /**
    @ingroup core
    @brief Create a wrapper on a Database to match the interface of an
           array(considering Input of the database only) (define methods size() and operator[])
           For read access only.

           It is used as a vector adapter on the database's inputs. A typical use is for 
           example to partionnate the database's inputs using a kd-tree as in ClassifierNearestNeighbor.

           While the adapter is used, the original database must be kept alive.

    @sa Database
    */
   template <class Database>
   class DatabaseInputAdapterRead
   {
   public:
      typedef typename Database::Sample::Input Point;
      typedef Point                            value_type;

   public:
      DatabaseInputAdapterRead( const Database& database ) : _database( database )
      {}
      const Point& operator[]( ui32 n ) const
      {
         assert( n < size() );
         return _database[ n ].input;
      }
      ui32 size() const
      {
         return _database.size();
      }

   private:
      const Database&    _database;
   };

   /**
    @ingroup core
    @brief Create a wrapper on a Database to match the interface of an
           array(considering Input of the database only) (define methods size() and operator[]). Only a specific
           type of samples can be filtered.

           It is used as a vector adapter on the database's inputs. A typical use is for 
           example to partionnate the database's inputs using a kd-tree as in ClassifierNearestNeighbor.

           While the adapter is used, the original database must be kept alive and must <b>not</b> change
           since an internal index is built and won't be up to date.
    @sa Database
    */
   template <class Database>
   class DatabaseInputAdapterType
   {
   public:
      typedef typename Database::Sample::Input Point;
      typedef Point                            value_type;

   public:
      /**
       @param database the database to be mapped
       @param types the types to be mapped (i.e. <code>Database::Type</code>). The others are discarded.
       */
      DatabaseInputAdapterType( Database& database, const std::vector<ui32>& types ) : _database( database )
      {
         // construct the internal index
         _buildIndex( types );
      }
      const Point& operator[]( ui32 n ) const
      {    
         assert( n < size() );
         return _database[ _index[ n ] ].input;
      }
      Point& operator[]( ui32 n )
      {
         assert( n < size() );
         return _database[ _index[ n ] ].input;
      }
      ui32 size() const
      {
         return static_cast<ui32>( _index.size() );
      }

   private:
      /**
       @brief build an index to address only the specified types.
       */
      void _buildIndex( const std::vector<ui32>& types )
      {
         ui32 size = 0;

         // compute the size of the index
         for ( ui32 n = 0; n < _database.size(); ++n )
            if ( _isIn( n, types ) )
               ++size;
         if ( !size )
            return;

         // fill the index
         _index.reserve( size );
         for ( ui32 n = 0; n < _database.size(); ++n )
            if ( _isIn( n, types ) )
               _index.push_back( n );
      }

      /**
       @brief check if the <code>index</code> needs to be addressed
       @return false if filtered out
       */
      inline bool _isIn( ui32 index, const std::vector<ui32>& types ) const
      {
         for ( ui32 n = 0; n < types.size(); ++n )
            if ( _database[ index ].type == static_cast<typename Database::Sample::Type>( types[ n ] ) )
               return true;
         return false;
      }

   private:
      Database&         _database;
      std::vector<ui32> _index;
   };


   /**
    @ingroup core
    @brief Create a wrapper on a Database to match the interface of an
           array(considering Input of the database only) (define methods size() and operator[]). The samples
           are filtered on their class

           It is used as a vector adapter on the database's inputs. A typical use is for 
           example to partionnate the database's inputs using a kd-tree as in ClassifierNearestNeighbor.

           While the adapter is used, the original database must be kept alive and must <b>not</b> change
           since an internal index is built and won't be up to date.
    @sa Database
    */
   template <class Database>
   class DatabaseInputAdapterClass
   {
   public:
      typedef typename Database::Sample::Input Point;
      typedef Point                            value_type;

   public:
      /**
       @param database the database to be mapped
       @param types the types to be mapped (i.e. <code>Database::Type</code>). The others are discarded.
       */
      DatabaseInputAdapterClass( Database& database, ui32 classToMap ) : _database( database )
      {
         // construct the internal index
         _buildIndex( classToMap );
      }
      const Point& operator[]( ui32 n ) const
      {    
         assert( n < size() );
         return _database[ _index[ n ] ].input;
      }
      Point& operator[]( ui32 n )
      {
         assert( n < size() );
         return _database[ _index[ n ] ].input;
      }
      ui32 size() const
      {
         return static_cast<ui32>( _index.size() );
      }

   private:
      DatabaseInputAdapterClass& operator=( const DatabaseInputAdapterClass& );

   private:
      /**
       @brief build an index to address only the specified types.
       */
      void _buildIndex( ui32 classToMap )
      {
         ui32 size = 0;

         // compute the size of the index
         for ( ui32 n = 0; n < _database.size(); ++n )
            if ( _database[ n ].output == classToMap )
               ++size;
         if ( !size )
            return;

         // fill the index
         _index.reserve( size );
         for ( ui32 n = 0; n < _database.size(); ++n )
            if ( _database[ n ].output == classToMap )
               _index.push_back( n );
      }

   private:
      Database&         _database;
      std::vector<ui32> _index;
   };
}
}

# pragma warning( pop )

#endif
