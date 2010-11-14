#ifndef NLL_ALGORITHM_NAIVE_BAYES_H_
# define NLL_ALGORITHM_NAIVE_BAYES_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief NaiveBayes classifier implementation

    See "http://en.wikipedia.org/wiki/Naive_Bayes_classifier"
    */
   class NayiveBayes
   {
      typedef core::Buffer1D<double>   Vector;

      struct Class
      {
         Vector      mean;
         Vector      var;

         template <class Point>
         double computeProbability( const Point& p ) const
         {
            double pp = 0;
            for ( ui32 n = 0; n < mean.size(); ++n )
            {
               // if we have a log of features, better do a log for numerical stability
               const double pf = _sqrdiv[ n ] * exp( - core::sqr( p[ n ] - mean[ n ] ) / _norm[ n ] );
               pp += log( pf );
            }
            return exp( pp );
         }

         void precompute()
         {
            _sqrdiv = Vector( mean.size() );
            _norm = Vector( mean.size() );
            for ( ui32 n = 0; n < mean.size(); ++n )
            {
               _sqrdiv[ n ] = 1.0 / ( sqrt( 2 * nll::core::PI * core::sqr( var[ n ] ) ) + 1e-8 );
               _norm[ n ] = 2.0 * core::sqr( var[ n ] ) + 1e-8;
            }
         }

      private:
         Vector _sqrdiv;
         Vector _norm;
      };
      typedef std::vector<Class> Classes;

   public:
      template <class Database>
      void compute( const Database& database )
      {
         if ( database.size() == 0 )
            return;
         const ui32 nbFeatures = database[ 0 ].input.size();

         // compute some constants
         const ui32 nbClasses = getNumberOfClass( database );
         _classes = Classes( nbClasses );

         // first filter the training samples
         Database training = core::filterDatabase( database, core::make_vector<ui32>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

         for ( ui32 classid = 0; classid < nbClasses; ++classid )
         {
            _classes[ classid ].mean = Vector( nbFeatures );
            _classes[ classid ].var  = Vector( nbFeatures );

            Vector& mean = _classes[ classid ].mean;
            Vector& var  = _classes[ classid ].var;

            // filter the samples by class
            typedef core::DatabaseInputAdapterClass<Database> Adapter;
            Adapter adapter( training, classid );

            // compute the mean
            for ( ui32 n = 0; n < adapter.size(); ++n )
            {
               Adapter::Point& s = adapter[ n ];
               for ( ui32 f = 0; f < nbFeatures; ++f )
               {
                  mean[ f ] += s[ f ];
               }
            }

            for ( ui32 f = 0; f < nbFeatures; ++f )
            {
               mean[ f ] /= adapter.size();
            }

            // compute the variance
            for ( ui32 n = 0; n < adapter.size(); ++n )
            {
               Adapter::Point& s = adapter[ n ];
               for ( ui32 f = 0; f < nbFeatures; ++f )
               {
                  double val = s[ f ] - mean[ f ];
                  var[ f ] += core::sqr( val );
               }
            }

            for ( ui32 f = 0; f < nbFeatures; ++f )
            {
               var[ f ] /= adapter.size();
            }

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
            _classes[ n ].mean.write( o );
            _classes[ n ].var.write( o );
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
            _classes[ classid ].mean.read( i );
            _classes[ classid ].var.read( i );
            _classes[ classid ].precompute();
         }
      }

      template <class Point>
      ui32 test( const Point& p, core::Buffer1D<double>* probability = 0 ) const
      {
         double max = (double)INT_MIN;
         double sum = 0;
         ui32 index = (ui32)_classes.size();

         if ( probability )
         {
            *probability = core::Buffer1D<double>( (ui32)_classes.size() );
         }

         for ( ui32 n = 0; n < (ui32)_classes.size(); ++n )
         {
            double v = _classes[ n ].computeProbability( p );
            if ( v > max )
            {
               max = v;
               index = n;
            }

            if ( probability )
            {
               sum += v;
               ( *probability )[ n ] = v;
            }
         }

         if ( probability )
         {
            for ( ui32 n = 0; n < (ui32)_classes.size(); ++n )
               ( *probability )[ n ] /= sum;
         }

         assert( index < _classes.size() );
         return index;
      }

      const Classes& getParameters() const
      {
         return _classes;
      }

   private:
      Classes  _classes;
   };
}
}