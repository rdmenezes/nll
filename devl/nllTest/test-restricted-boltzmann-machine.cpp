#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace algorithm
{
   template <class Value>
   class Unit
   {
   public:
      virtual void activation( double activation );
      virtual Value getValue();
      virtual ~Unit(){}
   };

   template <class Value>
   class UnitBinaryLogistic : public Unit<Value>
   {
   public:
      virtual void activation( double activation )
      {
         double r = rand() / RAND_MAX;
         double t = 1 / ( 1 + exp( - activation ) );
         _value = ( r >= t ) ? 1 : 0;
      }

      virtual Value getValue()
      {
         return _value;
      }

   protected:
      Value    _value;
   };

   template <class Point, class UnitType = UnitBinaryLogistic<Point> >
   class BoltzmannMachineRestrictedBinaryGeneration
   {
      typedef core::Matrix<double>        Weight;  // define a weights matrix. the line N corresponds to a weight of the N nth unit to the next layer.
      typedef std::vector<UnitType>       Units;   // a set of units
      typedef std::pair<Units, Weight>    Layer;   // a layer is a set of input units and a weight matrix
      typedef std::vector<Layer>          Layers;  // last layer has an empty matrix

   public:
      BoltzmannMachineRestrictedBinaryGeneration( const std::vector<ui32>& layers, double alpha, ui32 iter ) :
         _alpha( alpha ), _iter( iter )
      {
         assert( layers.size() >= 2 );
         for ( ui32 n = 0; n < layers.size() - 1; ++n )
         {
            Matrix m( layers[ n ], layers[ n + 1 ] );
            for ( ui32 nn = 0; nn < m.size(); ++nn )
               m[ nn ] = core::generateUniformDistribution( -0.1, 0.1 );
            _layers.push_back( std::make_pair( Units(), m ) );
         }
         _layers.push_back( std::make_pair( Units(), Matrix( 0, 0 ) ) );
      }

      double forwardActivation( ui32 layer, ui32 unit )
      {
         assert( layer > 0 && layer < _layers.size() );
         double activation = 0;
         for ( ui32 n = 0; n < _layers[ layer - 1 ]; ++n )
            activation += _layers[ layer - 1 ].first[ n ] * _layers[ layer - 1 ].second( layer - 1, n );
         return activation;
      }

      double backwardActivation( ui32 layer, ui32 unit )
      {
         assert( layer > 1 && layer < _layers.size() - 1 );
         double activation = 0;
         for ( ui32 n = 0; n < _layers[ layer + 1 ]; ++n )
            activation += _layers[ layer + 1 ].first[ n ] * _layers[ layer + 1 ].second( layer + 1, n );
         return activation;
      }

   protected:
      Layers      _layers;
      double      _alpha;
      ui32        _iter;
   };
}
}

class TestBoltzmannMachine
{
public:
   void testBinaryRestrictedBoltzmannMachineGeneration()
   {
      std::cout << "HAHA" << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBoltzmannMachine);
TESTER_TEST(testBinaryRestrictedBoltzmannMachineGeneration);
TESTER_TEST_SUITE_END();
#endif
