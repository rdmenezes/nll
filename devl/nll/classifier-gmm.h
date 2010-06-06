#ifndef NLL_CLASSIFIER_GMM_H_
# define NLL_CLASSIFIER_GMM_H_

# include "gmm.h"
# include "classifier.h"
# include "io.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Gaussian mixture model algorithm with expectation-maximization for learning

     Points should be viewed as 1 point for the classifier: 1 point is actually composed of a sequence points
     */
   template <class Points>
   class ClassifierGmm : public Classifier<Points>
   {
   public:
      typedef Gmm                                           TGmm;
      typedef std::vector<TGmm>                             Gmms;
      typedef Classifier<Points>                            Base;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

   public:
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 10, 6, 4, 1 ) );
         parameters.push_back( new ParameterOptimizerGaussianInteger( 3, 15, 6, 4, 1 ) );
         return parameters;
      }

   public:
      ClassifierGmm() : Base( buildParameters() )
      {}
      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierGmm* cgmm = new ClassifierGmm();
         cgmm->_gmms = Gmms( _gmms.size() );
         for ( ui32 n = 0; n < _gmms.size(); ++n )
            cgmm->_gmms[ n ].clone( _gmms[ n ] );
         cgmm->_crossValidationBin = this->_crossValidationBin;
         return cgmm;
      }

      virtual void read( std::istream& i )
      {
         ui32 size = 0;
         core::read<ui32>( size, i );
         if ( size == 0)
            return;
         for ( ui32 n = 0; n < size; ++n )
            core::read<TGmm>( _gmms[ n ], i );
      }

      virtual void write( std::ostream& o ) const
      {
         ui32 size = static_cast<ui32>( _gmms.size() );
         core::write<ui32>( size, o );
         for ( ui32 n = 0; n < size; ++n )
            core::write<TGmm>( _gmms[ n ], o );
      }


      virtual Output test( const Points& p ) const
      {
         double likelihood_max = INT_MIN;
         ui32 class_max = INT_MAX;
         for ( ui32 n = 0; n < _gmms.size(); ++n )
         {
            double l = _gmms[ n ].likelihood( p );
            if ( l > likelihood_max )
            {
               class_max = n;
               likelihood_max = l;
            }
         }
         assert( ! core::equal<double>( likelihood_max, INT_MIN ) );
         return class_max;
      }

      virtual Output test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         probability = core::Buffer1D<double>( (ui32)_gmms.size() );

         double likelihood_max = INT_MIN;
         double sum = 0;
         ui32 class_max = INT_MAX;
         for ( ui32 n = 0; n < _gmms.size(); ++n )
         {
            double l = _gmms[ n ].likelihood( p );
            if ( l > likelihood_max )
            {
               class_max = n;
               likelihood_max = l;
            }

            probability[ n ] = exp( l );
            sum += probability[ n ];
         }

         ensure( sum > 0, "probability error" );
         for ( ui32 n = 0; n < _gmms.size(); ++n )
            probability[ n ] /= sum;


         assert( ! core::equal<double>( likelihood_max, INT_MIN ) );
         return class_max;
      }

      /**
       @param parameters parameters of the learning phase 
              - parameters[ 0 ] = nbGaussians
              - parameters[ 1 ] = nbIter
       */
      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         std::map<ui32, ui32> sizeDat;
         for ( ui32 n = 0; n < dat.size(); ++n )
            if ( dat[ n ].type == Base::Sample::LEARNING )
               sizeDat[ dat[ n ].output ] += static_cast<ui32>( dat[ n ].input.size() );

         if ( !sizeDat.size() )
            return;

         for ( ui32 n = 0; n < sizeDat.size(); ++n )
         {
            std::map<ui32, ui32>::const_iterator it = sizeDat.find( n );
            assert( it != sizeDat.end() ); // a class is missing : database class must be continuous and starts at 0
            Points points( it->second );
            ui32 index = 0;
            for ( ui32 n2 = 0; n2 < dat.size(); ++n2 )
            {
               if ( dat[ n2 ].type == Base::Sample::LEARNING && dat[ n2 ].output == n )
                  for ( ui32 nn = 0; nn < dat[ n2 ].input.size(); ++nn )
                     points[ index++ ] = dat[ n2 ].input[ nn ];
            }

            TGmm gmm;
            ensure( points.size(), "empty points, error!" );
            ui32 pointSize = static_cast<ui32>( points[ 0 ].size() );
            ensure( pointSize, "point dimension is 0, error!" );
            assert( parameters.size() == this->_parametersPrototype.size() );
            gmm.em( points, pointSize, static_cast<ui32>( parameters[ 0 ] ), static_cast<ui32>( parameters[ 1 ] ) );
            _gmms.push_back( gmm );
         }
      }

      const Gmms& getModel() const { return _gmms; }

   private:
      Gmms     _gmms;
   };
}
}

#endif
