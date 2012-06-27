#include <nll/nll.h>
#include <stack>
#include <tester/register.h>
#include "config.h"


using namespace nll;
using namespace nll::core;


namespace nll
{
namespace algorithm
{
   /**
    @brief Efficiently represent a factored gaussian joint distribution
    @note we are building up on the PotentialGaussian which are used to do the math

    The domain must be ordered so that domain[ 0 ] < domain[ 1 ] < domain[ 2 ] ... meaning that in a graph,
    the parent nodes must have a higher domain than the children.

    We model the conditional gaussian as below: let's define  variables y; x and its corresponding weight vector w
    p(y | x1..xn) = N(b0 + b^t * x; stddev^2 )

    with b0 = mean_y - Cov_yx * Cov_xx^-1 * mean_x
         b = Cov_xx^-1 * Cov_yx
         stddev^2 = Cov_yy - Cov_yx * Cov_xx^-1 * Cov_xy
    */
   class PotentialLinearGaussian
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<ui32>        VectorI;

   public:
      struct Dependency
      {
         Dependency()
         {}

         Dependency( std::shared_ptr<PotentialLinearGaussian> p, value_type w ) : potential( p ), weight( w )
         {}

         std::shared_ptr<PotentialLinearGaussian>  potential;
         value_type weight;
      };

      PotentialLinearGaussian()
      {}

      /**
       @brief Intanciate a potential without dependencies
       @param m mean
       @param c covariance
       @param id naming of the variable
       */
      PotentialLinearGaussian( const Vector& m, const Matrix& c, const VectorI& ids ) : _mean( m ),
         _cov( c ), _ids( ids )
      {
         ensure( m.size() == ids.size(), "dimention mismatch" );
         ensure( m.size() == c.sizex(), "dimention mismatch" );
         ensure( m.size() == c.sizey(), "dimention mismatch" );

         ensure( ids.size() == 1, "NOT TESTED" );
      }

      /**
      @brief Intanciate a potential with dependencies
       @param m mean
       @param c covariance
       @param id naming of the variable
       @param dependencies the dependencies of the linear gaussian. They must have exactly the same dimentionality as the current potential
       */
      PotentialLinearGaussian( const Vector& m, const Matrix& c, const VectorI& ids, std::vector<Dependency>& dependencies ) : _mean( m ),
         _cov( c ), _ids( ids ), _dependencies( dependencies )
      {
         ensure( ids.size() == 1, "NOT TESTED" );
         ensure( dependencies.size() > 0, "use other constructor!" );

         ui32 id = dependencies[ 0 ].potential->getIds()[ 0 ];
         for ( size_t n = 1; n < dependencies.size(); ++n )
         {
            ui32 curId = dependencies[ n ].potential->getIds()[ 0 ];
            ensure( id > curId, "the dependencies must be ordered from min->max id" );
         }

         ensure( m.size() == ids.size(), "dimention mismatch" );
         ensure( m.size() == c.sizex(), "dimention mismatch" );
         ensure( m.size() == c.sizey(), "dimention mismatch" );
      }

      const Vector& getMean() const
      {
         return _mean;
      }

      const Matrix& getCov() const
      {
         return _cov;
      }

      const VectorI& getIds() const
      {
         return _ids;
      }

      /**
       @brief Create a canonical potential out of the linear gaussian format. The purpose is to create a joint gaussian instead of the conditional gaussian!
       */
      PotentialGaussianCanonical toGaussianCanonical() const
      {
         // first, get our merged ids
         std::vector<const VectorI*> idsptr( _dependencies.size() + 1 );
         for ( size_t n = 0; n < _dependencies.size(); ++n )
         {
            idsptr[ n ] = &(_dependencies[ n ].potential->getIds());
         }
         idsptr[ _dependencies.size() ] = &_ids;
         const std::vector<ui32> ids = mergeIds( idsptr );

         // now just prepare the parameters of the canonical potential
         Matrix sinv;
         sinv.clone( _cov );
         core::inverse( sinv );


         const value_type gaussCte = getGaussNorm( _cov );
         const value_type g = -0.5 * core::fastDoubleMultiplication( _mean, sinv ) + std::log( gaussCte );

         Vector w( static_cast<ui32>( ids.size() ) - _ids.size() );
         Vector h( static_cast<ui32>( ids.size() ) );
         for ( size_t n = 0; n < _dependencies.size(); ++n )
         {
            const ui32 nui = static_cast<ui32>( n );
            h[ nui + 1 ] = - _dependencies[ nui ].weight * sinv[ 0 ] * _mean[ 0 ];
            w[ nui ] = _dependencies[ nui ].weight;
         }
         h[ 0 ] = sinv[ 0 ] * _mean[ 0 ];

         Matrix ksub = Matrix( w, w.size(), 1 ) * sinv * Matrix( w, 1, w.size() );
         Matrix k( (ui32)ids.size(), (ui32)ids.size() );
         for ( ui32 nx = 0; nx < ksub.sizex(); ++nx )
         {
            for ( ui32 ny = 0; ny < ksub.sizey(); ++ny )
            {
               k( ny + 1, nx + 1 ) = ksub( ny, nx );
            }

            const value_type v = -w[ nx ] * sinv[ 0 ];
            k( nx + 1, 0 ) = v;
            k( 0, nx + 1 ) = v;
         }
         k( 0, 0 ) = sinv[ 0 ];

         VectorI idsi( static_cast<ui32>( ids.size() ) );
         std::copy( ids.begin(), ids.end(), idsi.begin() );
         return PotentialGaussianCanonical( h, k, g, idsi );
      }

   private:
      // compute the gaussian normalization factor
      static value_type getGaussNorm( const Matrix& m )
      {
         const value_type length = static_cast<value_type>( m.size() );
         return std::pow( 2 * core::PI, -length / 2 ) * std::pow( core::det( m ), -0.5 );
      }

      /**
       @brief given a list if domains, build the merged domain
       */
      static std::vector<ui32> mergeIds( const std::vector<const VectorI*>& lists )
      {
         ui32 maxIds = 0;
         for ( ui32 n = 0; n < lists.size(); ++n )
         {
            maxIds += lists[ n ]->size();
         }

         if ( maxIds == 0 )
            return std::vector<ui32>();   // empty list

         std::vector<ui32> merged;
         merged.reserve( maxIds );
         std::vector<ui32> pointer( lists.size() );


         // the list are already sorted, so we can simply do a mergesort
         while (1)
         {
            ui32 lastMin = std::numeric_limits<ui32>::max();
            ui32 minIndex = 0;
            ui32 min = std::numeric_limits<ui32>::max();  // we don't want duplicates...
            for ( ui32 n = 0; n < lists.size(); ++n )
            {
               const ui32 pointerIndex = pointer[ n ];
               if ( pointerIndex < lists[ n ]->size() )
               {
                  const ui32 id = static_cast<ui32>( (*lists[ n ])[ pointerIndex ] );
                  if ( id < min && id != lastMin )
                  {
                     minIndex = n;
                     min = id;
                  }
               }
            }

            if ( min == std::numeric_limits<ui32>::max() || ( merged.size() && min == *merged.rbegin() ) )
               break;   // no more min this iteration
            merged.push_back( min );
            ++pointer[ minIndex ];
            lastMin = min;
         }

         return merged;
      }

   private:
      Vector      _mean;
      Matrix      _cov;
      VectorI     _ids;
      std::vector<Dependency>  _dependencies;
   };

   /**
    @brief Traits to instanciate a Bayesian potential

    In the general case, instanciating a potential is not required. However for Linear Gaussian potential for example, each node must be transformed
    as a Gaussian canonical potential with all the operations associated with potentials
    */
   template <>
   struct TraitsInstanciateBayesianPotential<PotentialLinearGaussian>
   {
      typedef PotentialGaussianCanonical PotentialInstanciationType;

      static PotentialInstanciationType create( const PotentialLinearGaussian& potential )
      {
         // default behaviour: just copy the potential
         return potential.toGaussianCanonical();
      }
   };
}

namespace core
{
   template <class Function>
   struct FunctorCreator
   {
   };
}
}


class TestGaussianBayesianInference
{
public:
   enum DomainVariable
   {
      // the order is important for efficiency reason, however for practicality this is not the best. So use PotentialTable::reorderTable
      WETGRASS = 50,
      RAIN,
      SPRINKLER,
      CLOUDY
   };

   typedef algorithm::PotentialTable               Factor;
   typedef algorithm::BayesianNetwork<Factor>      BayesianNetwork;
   typedef algorithm::PotentialLinearGaussian      Factorg;
   typedef algorithm::BayesianNetwork<Factorg>     BayesianNetworkg;

   static std::auto_ptr<BayesianNetwork> buildSprinklerNet()
   {
      std::auto_ptr<BayesianNetwork> bnet( new BayesianNetwork() );

      //
      // cloudy
      //
      Factor::VectorI domainCloudyTable( 1 );
      domainCloudyTable[ 0 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityCloudy( 1 );
      cardinalityCloudy[ 0 ] = 2;
      Factor cloudy( core::make_buffer1D<double>( 0.5, 0.5 ), domainCloudyTable, cardinalityCloudy );

      //
      // sprinkler
      //
      Factor::VectorI domainSprinklerTable( 2 );
      domainSprinklerTable[ 0 ] = (int)SPRINKLER;
      domainSprinklerTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalitySprinkler( 2 );
      cardinalitySprinkler[ 0 ] = 2;
      cardinalitySprinkler[ 1 ] = 2;
      // note: the table is encoded by domain table order starting by FF then FT, TF, TT and so on for more domain...
      Factor sprinkler( core::make_buffer1D<double>( 0.5, 0.5, 0.9, 0.1 ), domainSprinklerTable, cardinalitySprinkler );

      //
      // Rain
      //
      Factor::VectorI domainRainTable( 2 );
      domainRainTable[ 0 ] = (int)RAIN;
      domainRainTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityRain( 2 );
      cardinalityRain[ 0 ] = 2;
      cardinalityRain[ 1 ] = 2;
      Factor rain( core::make_buffer1D<double>( 0.8, 0.2, 0.2, 0.8 ), domainRainTable, cardinalityRain );

      //
      // Wet
      //
      Factor::VectorI domainWetTable( 3 );
      domainWetTable[ 0 ] = (int)WETGRASS;
      domainWetTable[ 1 ] = (int)RAIN;
      domainWetTable[ 2 ] = (int)SPRINKLER;
      Factor::VectorI  cardinalityWet( 3 );
      cardinalityWet[ 0 ] = 2;
      cardinalityWet[ 1 ] = 2;
      cardinalityWet[ 2 ] = 2;
      Factor wet( core::make_buffer1D<double>( 1, 0, 0.1, 0.9, 0.1, 0.9, 0.01, 0.99 ), domainWetTable, cardinalityWet );

      //
      // create network
      //
      BayesianNetwork::NodeDescritor cloudyNode    = bnet->addNode( "CLOUDY",     cloudy );
      BayesianNetwork::NodeDescritor wetNode       = bnet->addNode( "WET",        wet );
      BayesianNetwork::NodeDescritor sprinklerNode = bnet->addNode( "SPRINKLER",  sprinkler );
      BayesianNetwork::NodeDescritor rainNode      = bnet->addNode( "RAIN",       rain );

      bnet->addLink( cloudyNode, sprinklerNode );
      bnet->addLink( cloudyNode, rainNode );
      bnet->addLink( sprinklerNode, wetNode );
      bnet->addLink( rainNode, wetNode );

      return bnet;
   }

   static std::auto_ptr<BayesianNetwork> buildSprinklerNet2()
   {
      std::auto_ptr<BayesianNetwork> bnet( new BayesianNetwork() );

      //
      // cloudy
      //
      Factor::VectorI domainCloudyTable( 1 );
      domainCloudyTable[ 0 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityCloudy( 1 );
      cardinalityCloudy[ 0 ] = 3;
      Factor cloudy( core::make_buffer1D<double>( 0.2, 0.5, 0.3 ), domainCloudyTable, cardinalityCloudy );

      //
      // sprinkler
      //
      Factor::VectorI domainSprinklerTable( 2 );
      domainSprinklerTable[ 0 ] = (int)SPRINKLER;
      domainSprinklerTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalitySprinkler( 2 );
      cardinalitySprinkler[ 0 ] = 2;
      cardinalitySprinkler[ 1 ] = 3;
      // note: the table is encoded by domain table order starting by FF then FT, TF, TT and so on for more domain...
      Factor sprinkler( core::make_buffer1D<double>( 0.5, 0.5, 0.7, 0.3, 0.9, 0.1 ), domainSprinklerTable, cardinalitySprinkler );

      //
      // Rain
      //
      Factor::VectorI domainRainTable( 2 );
      domainRainTable[ 0 ] = (int)RAIN;
      domainRainTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityRain( 2 );
      cardinalityRain[ 0 ] = 2;
      cardinalityRain[ 1 ] = 3;
      Factor rain( core::make_buffer1D<double>( 0.8, 0.2, 0.4, 0.6, 0.2, 0.8 ), domainRainTable, cardinalityRain );

      //
      // Wet
      //
      Factor::VectorI domainWetTable( 3 );
      domainWetTable[ 0 ] = (int)WETGRASS;
      domainWetTable[ 1 ] = (int)RAIN;
      domainWetTable[ 2 ] = (int)SPRINKLER;
      Factor::VectorI  cardinalityWet( 3 );
      cardinalityWet[ 0 ] = 2;
      cardinalityWet[ 1 ] = 2;
      cardinalityWet[ 2 ] = 2;
      Factor wet( core::make_buffer1D<double>( 1, 0, 0.1, 0.9, 0.1, 0.9, 0.01, 0.99 ), domainWetTable, cardinalityWet );

      //
      // create network
      //
      BayesianNetwork::NodeDescritor cloudyNode    = bnet->addNode( "CLOUDY",     cloudy );
      BayesianNetwork::NodeDescritor wetNode       = bnet->addNode( "WET",        wet );
      BayesianNetwork::NodeDescritor sprinklerNode = bnet->addNode( "SPRINKLER",  sprinkler );
      BayesianNetwork::NodeDescritor rainNode      = bnet->addNode( "RAIN",       rain );

      bnet->addLink( cloudyNode, sprinklerNode );
      bnet->addLink( cloudyNode, rainNode );
      bnet->addLink( sprinklerNode, wetNode );
      bnet->addLink( rainNode, wetNode );

      return bnet;
   }

   template <class InferenceAlgo>
   void testBasicInfPotentialTableImpl()
   {
      // see example from BNT http://bnt.googlecode.com/svn/trunk/docs/usage.html
      

      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();


      //
      // inference test. Checked against Matlab BNT
      //

      InferenceAlgo inference;
      Factor query1 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query1.getDomain().size() == 1 );
      TESTER_ASSERT( query1.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query1.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query1.getTable()[ 0 ], 0.5702, 1e-2 ) );
      TESTER_ASSERT( core::equal( query1.getTable()[ 1 ], 0.4298, 1e-2 ) );

      Factor query2 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query2.getDomain().size() == 1 );
      TESTER_ASSERT( query2.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query2.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query2.getTable()[ 0 ], 0.9379, 1e-2 ) );
      TESTER_ASSERT( core::equal( query2.getTable()[ 1 ], 0.0621, 1e-2 ) );
      
      Factor query3 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query3.getDomain().size() == 1 );
      TESTER_ASSERT( query3.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query3.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query3.getTable()[ 0 ], 0.8696, 1e-2 ) );
      TESTER_ASSERT( core::equal( query3.getTable()[ 1 ], 0.1304, 1e-2 ) );

      Factor query4 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query4.getDomain().size() == 1 );
      TESTER_ASSERT( query4.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query4.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query4.getTable()[ 0 ], 0.1639, 1e-2 ) );
      TESTER_ASSERT( core::equal( query4.getTable()[ 1 ], 0.8361, 1e-2 ) );

      Factor query5 = inference.run( *bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query5.getDomain().size() == 1 );
      TESTER_ASSERT( query5.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query5.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query5.getTable()[ 0 ], 0.7, 1e-2 ) );
      TESTER_ASSERT( core::equal( query5.getTable()[ 1 ], 0.3, 1e-2 ) );

      Factor query6 = inference.run( *bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query6.getDomain().size() == 1 );
      TESTER_ASSERT( query6.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query6.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query6.getTable()[ 0 ], 0.5, 1e-2 ) );
      TESTER_ASSERT( core::equal( query6.getTable()[ 1 ], 0.5, 1e-2 ) );

      Factor query7 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query7.getDomain().size() == 1 );
      TESTER_ASSERT( query7.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query7.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query7.getTable()[ 0 ], 0.4242, 1e-2 ) );
      TESTER_ASSERT( core::equal( query7.getTable()[ 1 ], 0.5758, 1e-2 ) );
   }

   template <class InferenceAlgo>
   void testBasicInfPotentialTableImpl2()
   {
      // see example from BNT http://bnt.googlecode.com/svn/trunk/docs/usage.html
      

      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet2();

     
      //
      // inference test. Checked against Matlab BNT
      //
      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.6032, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.3968, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.3358, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.6642, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 2 ), core::make_buffer1D<ui32>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.2548, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.7452, 1e-2 ) );
      }
      
      
      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 2 ), core::make_buffer1D<ui32>( (int)WETGRASS, (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 2 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getDomain()[ 1 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 4 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.2520, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.6480, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 2 ], 0.0028, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 3 ], 0.0972, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::Buffer1D<ui32>(), core::Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)WETGRASS, (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 2 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getDomain()[ 1 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 4 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.3186, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.4014, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 2 ], 0.0159, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 3 ], 0.2641, 1e-2 ) );
      }
   }

   void testBasicInfPotentialTable()
   {
      
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      
      testBasicInfPotentialTableImpl2<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl2<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
   }

   void testPotentialTableMlParametersEstimation()
   {
      typedef algorithm::BayesianNetwork<algorithm::PotentialTable> BayesianNetwork;

      // check against BNT results
      std::vector< std::vector< double > > file1 = core::readVectorFromMatlabAsColumn( NLL_TEST_PATH "data/bnt/bnt_samples_sprinkler.txt");

      // reorganize the data
      std::vector< BayesianNetwork::Factor::EvidenceValue > samples;
      for ( size_t id = 0; id < file1.size(); ++id )
      {
         std::vector< double >& sample = file1[ id ];
         BayesianNetwork::Factor::EvidenceValue newSample( (ui32)file1.size() );
         for ( size_t n = 0; n < sample.size(); ++n )
         {
            newSample[ (ui32)n ] = static_cast<ui32>( sample[ (ui32)n ] - 1 );
         }
         samples.push_back( newSample );
      }

      // do some ML estimation
      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();
      algorithm::BayesianNetworkMaximumLikelihoodParameterEstimation<BayesianNetwork> mlEstimator;
      mlEstimator.compute( core::make_buffer1D<ui32>( (int)CLOUDY, (int)SPRINKLER, (int)RAIN, (int)WETGRASS ), samples, *bnet );

      std::vector<const BayesianNetwork::Factor*> factors;
      algorithm::getFactors( *bnet, factors );
      for ( ui32 n = 0; n < factors.size(); ++n )
      {
         factors[ n ]->print( std::cout );
      }

      // compare against BNT
      TESTER_ASSERT( factors.size() == 4 );
      TESTER_ASSERT( factors[ 0 ]->getDomain().size() == 1 );
      TESTER_ASSERT( factors[ 0 ]->getDomain()[ 0 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 0 ]->getTable().size() == 2 );
      TESTER_ASSERT( core::equal( factors[ 0 ]->getTable()[ 0 ], 0.46, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 0 ]->getTable()[ 1 ], 0.53, 0.01 ) );

      TESTER_ASSERT( factors[ 1 ]->getDomain().size() == 2 );
      TESTER_ASSERT( factors[ 1 ]->getDomain()[ 0 ] == (int)SPRINKLER );
      TESTER_ASSERT( factors[ 1 ]->getDomain()[ 1 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 1 ]->getTable().size() == 4 );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 0 ], 0.5, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 1 ], 0.5, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 2 ], 0.9375, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 3 ], 0.0625, 0.01 ) );

      TESTER_ASSERT( factors[ 2 ]->getDomain().size() == 2 );
      TESTER_ASSERT( factors[ 2 ]->getDomain()[ 0 ] == (int)RAIN );
      TESTER_ASSERT( factors[ 2 ]->getDomain()[ 1 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 2 ]->getTable().size() == 4 );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 0 ], 0.71, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 1 ], 0.28, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 2 ], 0.18, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 3 ], 0.81, 0.01 ) );

      TESTER_ASSERT( factors[ 3 ]->getDomain().size() == 3 );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 0 ] == (int)WETGRASS );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 1 ] == (int)RAIN );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 2 ] == (int)SPRINKLER );
      TESTER_ASSERT( factors[ 3 ]->getTable().size() == 8 );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 0 ], 1.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 1 ], 0.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 2 ], 0.076, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 3 ], 0.923, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 4 ], 0.25, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 5 ], 0.75, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 6 ], 0.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 7 ], 1.0, 0.01 ) );
   }

   void testBnPotentialSampling()
   {
      srand( 100 );
      typedef algorithm::BayesianNetwork<algorithm::PotentialTable> BayesianNetwork;
      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();

      typedef algorithm::BayesianNetworkSampling<BayesianNetwork> Sampler;
      Sampler bnSampler;
      std::vector<Sampler::EvidenceValue> samples;
      Sampler::VectorI domainOutput;
      bnSampler.compute( *bnet, 10000, samples, domainOutput );

      // now reestimate the BN and compare to true values
      algorithm::BayesianNetworkMaximumLikelihoodParameterEstimation<BayesianNetwork> mlEstimator;
      mlEstimator.compute( domainOutput, samples, *bnet );

      std::vector<const BayesianNetwork::Factor*> factorsFound;
      algorithm::getFactors( *bnet, factorsFound );
      for ( ui32 n = 0; n < factorsFound.size(); ++n )
      {
         factorsFound[ n ]->print( std::cout );
      }

      std::auto_ptr<BayesianNetwork> bnetRef = buildSprinklerNet();
      std::vector<const BayesianNetwork::Factor*> factorsRef;
      algorithm::getFactors( *bnetRef, factorsRef );

      const double EPSILON = 1e-2;
      for ( ui32 n = 0; n < factorsRef.size(); ++n )
      {
         for ( ui32 nn = 0; nn < factorsRef[ n ]->getTable().size(); ++nn )
         {
            TESTER_ASSERT( core::equal( factorsRef[ n ]->getTable()[ nn ],
                                        factorsFound[ n ]->getTable()[ nn ],
                                        EPSILON ) );
         }
      }
   }


   static std::auto_ptr<BayesianNetworkg> buildGaussianBn1()
   {
      typedef algorithm::PotentialGaussianMoment FactorMoment;
      std::auto_ptr<BayesianNetworkg> bnet( new BayesianNetworkg() );

      //
      // cloudy
      //
      FactorMoment::VectorI domainC( 1 );
      domainC[ 0 ] = (int)CLOUDY;
      FactorMoment::Vector meanC( 1 );
      meanC[ 0 ] = 15;
      FactorMoment::Matrix covC( 1, 1 );
      covC( 0, 0 ) = 0.5;
      FactorMoment cF( meanC, covC, domainC );


      FactorMoment::VectorI domainR( 1 );
      domainR[ 0 ] = (int)RAIN;
      FactorMoment::Vector meanR( 1 );
      meanR[ 0 ] = 50;
      FactorMoment::Matrix covR( 1, 1 );
      covR( 0, 0 ) = 0.1;
      FactorMoment rF( meanR, covR, domainR );

      FactorMoment::VectorI domainS( 3 );
      domainS[ 0 ] = (int)SPRINKLER;
      domainS[ 1 ] = (int)CLOUDY;
      domainS[ 2 ] = (int)RAIN;
      FactorMoment::Vector meanS( 1 );
      meanR[ 0 ] = 30;
      FactorMoment::Matrix covS( 1, 1 );
      covR( 0, 0 ) = 3;
      FactorMoment rS( meanS, covS, domainS );

      /*
      //
      // sprinkler
      //
      Factor::VectorI domainSprinklerTable( 2 );
      domainSprinklerTable[ 0 ] = (int)SPRINKLER;
      domainSprinklerTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalitySprinkler( 2 );
      cardinalitySprinkler[ 0 ] = 2;
      cardinalitySprinkler[ 1 ] = 3;
      // note: the table is encoded by domain table order starting by FF then FT, TF, TT and so on for more domain...
      Factor sprinkler( core::make_buffer1D<double>( 0.5, 0.5, 0.7, 0.3, 0.9, 0.1 ), domainSprinklerTable, cardinalitySprinkler );

      //
      // Rain
      //
      Factor::VectorI domainRainTable( 2 );
      domainRainTable[ 0 ] = (int)RAIN;
      domainRainTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityRain( 2 );
      cardinalityRain[ 0 ] = 2;
      cardinalityRain[ 1 ] = 3;
      Factor rain( core::make_buffer1D<double>( 0.8, 0.2, 0.4, 0.6, 0.2, 0.8 ), domainRainTable, cardinalityRain );

      //
      // Wet
      //
      Factor::VectorI domainWetTable( 3 );
      domainWetTable[ 0 ] = (int)WETGRASS;
      domainWetTable[ 1 ] = (int)RAIN;
      domainWetTable[ 2 ] = (int)SPRINKLER;
      Factor::VectorI  cardinalityWet( 3 );
      cardinalityWet[ 0 ] = 2;
      cardinalityWet[ 1 ] = 2;
      cardinalityWet[ 2 ] = 2;
      Factor wet( core::make_buffer1D<double>( 1, 0, 0.1, 0.9, 0.1, 0.9, 0.01, 0.99 ), domainWetTable, cardinalityWet );

      //
      // create network
      //
      BayesianNetwork::NodeDescritor cloudyNode    = bnet->addNode( "CLOUDY",     cloudy );
      BayesianNetwork::NodeDescritor wetNode       = bnet->addNode( "WET",        wet );
      BayesianNetwork::NodeDescritor sprinklerNode = bnet->addNode( "SPRINKLER",  sprinkler );
      BayesianNetwork::NodeDescritor rainNode      = bnet->addNode( "RAIN",       rain );

      bnet->addLink( cloudyNode, sprinklerNode );
      bnet->addLink( cloudyNode, rainNode );
      bnet->addLink( sprinklerNode, wetNode );
      bnet->addLink( rainNode, wetNode );
      */
      return bnet;
   }

   void testInferenceGaussianBn()
   {
      std::auto_ptr<BayesianNetworkg> bnet = buildGaussianBn1();

      std::cout << "ok" << std::endl;
   }

   template<typename T>
   struct EmptyDeleter
   {
       void operator() (T* )
       {
          // do nothing!
       }
   };

   void testLinearGaussian1()
   {
      typedef algorithm::PotentialLinearGaussian   Potential;
      enum
      {
         S, C, R
      };

      Potential::Vector meanR = core::make_buffer1D<double>( 50 );
      Potential::Matrix covR( 1, 1 ); covR[ 0 ] = 0.1;
      Potential::VectorI idR = core::make_buffer1D<ui32>( (int)R );
      Potential potR( meanR, covR, idR );

      Potential::Vector meanC = core::make_buffer1D<double>( 15 );
      Potential::Matrix covC( 1, 1 ); covC[ 0 ] = 0.5;
      Potential::VectorI idC = core::make_buffer1D<ui32>( (int)C );
      Potential potC( meanC, covC, idC );

      Potential::Vector meanS = core::make_buffer1D<double>( 30 );
      Potential::Matrix covS( 1, 1 ); covS[ 0 ] = 3;
      Potential::VectorI idS = core::make_buffer1D<ui32>( (int)S );
      Potential::Vector wS = core::make_buffer1D<double>( 1, 2 );
      
      std::vector<Potential::Dependency> dpsS;
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potR, EmptyDeleter<Potential>() ), wS[ 0 ] ) );
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potC, EmptyDeleter<Potential>() ), wS[ 1 ] ) );
      Potential potS( meanS, covS, idS, dpsS );

      algorithm::PotentialGaussianCanonical potr = potR.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical pots = potS.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical potc = potC.toGaussianCanonical();

      algorithm::PotentialGaussianMoment result = (potr * potc * pots).toGaussianMoment();

      // validated against matlab
      TESTER_ASSERT( core::equal<double>( result.getMean()[ S ], 145 ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ C ], 15 ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ R ], 50 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, S ), 3.9, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( R, R ), 0.1, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, C ), 0.5, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, R ), 0.2, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( R, S ), 0.2, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, C ), 0.5, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, S ), 0.5, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( R, C ), 0, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, R ), 0, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getAlpha(), 1, 1e-6 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testLinearGaussian1 );
//TESTER_TEST( testInferenceGaussianBn );

TESTER_TEST( testPotentialTableMlParametersEstimation );
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST( testBnPotentialSampling );
TESTER_TEST_SUITE_END();
#endif