#ifndef NLL_LUT_H_
# define NLL_LUT_H_

namespace nll
{
   namespace imaging
   {
      /**
       @brief a default color mapper
       */
      template <class T>
      class MapperLutColor
      {
         typedef core::Buffer1D<T>     Vector;
         typedef core::Buffer1D<T*>    VectorIndex;

      public:
         MapperLutColor( ui32 size, ui32 components ) : _size( size ), _components( components ), _container( size * components, false )
         {
            _index = VectorIndex( size );
            for ( ui32 n = 0; n < size; ++n )
            {
               _index[ n ] = &_container[ n * components ];
            }
         }

         void set( ui32 index, const T* value )
         {
            for ( ui32 n = 0; n < _components; ++n )
               _index[ index ][ n ] = value[ n ];
         }

         const T* operator[]( ui32 index ) const
         {
            return _index[ index ];
         }

         ui32 getNbComponents() const
         {
            return _components;
         }

         ui32 getSize() const
         {
            return _size;
         }

      private:
         ui32        _size;
         ui32        _components;
         Vector      _container;
         VectorIndex _index;
      };

      /**
       @ingroup imaging
       @brief Look up table

       TMapper should provide
         const T* operator[](index) const
         getSize() const 
         getNbComponents() const
         set( ui32 index, T* value )
       */
      template <class T, class TMapper>
      class LookUpTransform
      {
      public:
         LookUpTransform( const TMapper& mapper, double minIntensity, double maxIntensity ) : _mapper( mapper ), _min( minIntensity ), _max( maxIntensity ), _interval( maxIntensity - minIntensity + 1 )
         {
            ensure( _interval > 0, "must be >0" );
            _ratio = static_cast<double>( _mapper.getSize() ) / _interval;
         }

         const T* transform( double value ) const
         {
            if ( value < _min )
               return _mapper[ 0 ];
            if ( value > _max )
               return _mapper[ _mapper.getSize() - 1 ];
            return _mapper[ (ui32)( ( value - _min ) * _ratio ) ];
         }

         ui32 getSize() const
         {
            return _mapper.getSize();
         }

         ui32 getNbComponents() const
         {
            return _mapper.getNbComponents();
         }

         void set( ui32 index, const T* value )
         {
            _mapper.set( index, value );
         }

      protected:
         TMapper  _mapper;
         double   _min;
         double   _max;
         double   _interval;
         double   _ratio;
      };

      class LookUpTransformWindowingRGB
      {
      public:
         typedef ui8                         value_type;

      protected:
         typedef MapperLutColor<value_type>  LutMapper;
         typedef LookUpTransform<value_type, MapperLutColor<value_type> > Lut;

      public:
         LookUpTransformWindowingRGB( double minIntensity, double maxIntensity, ui32 size, ui32 nbComponents = 3 ) :
            _lut( LutMapper( size, nbComponents ), minIntensity, maxIntensity )
         {}

         const value_type* transform( double value ) const
         {
            return _lut.transform( value );
         }

         ui32 getSize() const
         {
            return _lut.getSize();
         }

         ui32 getNbComponents() const
         {
            return _lut.getNbComponents();
         }

         void set( ui32 index, const value_type* value )
         {
            _lut.set( index, value );
         }

         void createGreyscale()
         {
            core::Buffer1D<value_type> vals( _lut.getNbComponents() );
            for ( ui32 n = 0; n < _lut.getSize(); ++n )
            {
               for ( ui32 i = 0; i < _lut.getNbComponents(); ++i )
               {
                  vals[ i ] = static_cast<value_type>( 256.0 / _lut.getSize() * n );
               }
               set( n, vals.getBuf() );
            }
         }

      protected:
         Lut      _lut;
      };
   }
}

#endif