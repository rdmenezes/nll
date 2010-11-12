#ifndef NLL_QUADRATIC_DISCRIMINANT_ANALYSIS_H_
# define NLL_QUADRATIC_DISCRIMINANT_ANALYSIS_H_

namespace nll
{
namespace algorithm
{
   class QuadraticDiscriminantAnalysis
   {
      typedef core::Buffer1D<f64>    Point;
      typedef core::Matrix<f64>      Matrix;

      struct      Class
      {
         f64      prior;
         Point    mean;
         Matrix   cov;

         template <class InputPoint>
         double evaluateDiscriminant( const InputPoint& p ) const
         {
            Matrix pm( p.size(), 1 );
            for ( ui32 n = 0; n < p.size(); ++n )
               pm[ n ] = p[ n ];

            Matrix t = pm - meanm;
            Matrix tp;
            tp.clone( t );
            core::transpose( tp );

            return lp - 0.5 * log( det ) - 0.5 * ( tp * covinv * t )( 0, 0 );
         }

         void precompute()
         {
            lp = log( prior );
            covinv.clone( cov );
            bool r = core::inverse( covinv, &det );

            // if the matrix is singular, regularize it by adding a small diagonal...
            while ( !r )
            {
               core::LoggerNll::write( core::LoggerNll::ERROR, "covariance is singular in quadratic discriminant analysis" );
               std::cerr << "covariance is singular in quadratic discriminant analysis" << std::endl;
               for ( ui32 n = 0; n < covinv.sizex(); ++n )
                  covinv( n, n ) += 1e-2;
               r = core::inverse( covinv, &det );
            }

            meanm = Matrix( mean, mean.size(), 1 );
         }

      private:
         double lp;
         double det;
         Matrix covinv;
         Matrix meanm;
      };
      typedef std::vector<Class> Classes;

   public:
      /**
       @brief Compute the model

       Database must define an input = InputPoints and output = ui32

       InputPoints must define:
         InputPoint operator[](index)
         value size()

       InputPoint must define
         value operator[](index)
         value size()
       */
      template <class Database>
      void compute( const Database& database )
      {
         if ( database.size() == 0 )
            return;

         // compute some constants
         const ui32 nbClasses = getNumberOfClass( database );
         _classes = Classes( nbClasses );

         // first filter the training samples
         Database training = core::filterDatabase( database, core::make_vector<ui32>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

         ui32 nbSamples = 0;
         for ( ui32 classid = 0; classid < nbClasses; ++classid )
         {
            // filter the samples by class
            core::DatabaseInputAdapterClass<Database> adapter( training, classid );

            _classes[ classid ].prior = adapter.size();
            _classes[ classid ].cov   = core::covariance( adapter, &_classes[ classid ].mean );

            nbSamples += adapter.size();
         }

         for ( ui32 classid = 0; classid < nbClasses; ++classid )
         {
            _classes[ classid ].prior /= nbSamples;
            _classes[ classid ].precompute();   // precompute the constants
         }
      }

      void read( std::istream& i )
      {
         if ( !i.good() )
            throw std::runtime_error( "cannot read from stream" );
         
         // read
         ui32 nbC = 0;
         core::read<ui32>( nbC, i );
         _classes = Classes( nbC );
         for ( ui32 classid = 0; classid < nbC; ++classid )
         {
            core::read<f64>( _classes[ classid ].prior, i );
            _classes[ classid ].mean.read( i );
            _classes[ classid ].cov.read( i );
            _classes[ classid ].precompute();
         }
      }

      void write( std::ostream& o ) const
      {
         if ( !o.good() )
            throw std::runtime_error( "cannot read from stream" );

         ui32 nbC = (ui32)_classes.size();
         core::write<ui32>( nbC, o );
         for ( ui32 n = 0; n < nbC; ++n )
         {
            core::write<f64>( _classes[ n ].prior, o );
            _classes[ n ].mean.write( o );
            _classes[ n ].cov.write( o );
         }
      }

      /**
       @brief Returns the current model's parameters
       */
      const Classes& getParameters() const
      {
         return _classes;
      }

      /**
       @brief Detect to what class it belongs
       */
      template <class InputPoint>
      ui32 test( const InputPoint& p ) const
      {
         double max = (double)INT_MIN;
         ui32 index = (ui32)_classes.size();

         for ( ui32 n = 0; n < (ui32)_classes.size(); ++n )
         {
            double v = _classes[ n ].evaluateDiscriminant( p );
            if ( v > max )
            {
               max = v;
               index = n;
            }
         }

         assert( index < _classes.size() );
         return index;
      }

      /**
       @brief Project the point on each discriminant and return this vector
       */
      template <class InputPoint>
      Point project( const InputPoint& p ) const
      {
         Point pp( (ui32)_classes.size() );
         for ( ui32 n = 0; n < (ui32)_classes.size(); ++n )
         {
            pp[ n ] = _classes[ n ].evaluateDiscriminant( p );
         }

         return pp;
      }

   private:
      Classes  _classes;
   };
}
}

#endif
