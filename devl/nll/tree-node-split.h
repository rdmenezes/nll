/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_TREE_NODE_SPLIT_H_
# define NLL_ALGORITHM_TREE_NODE_SPLIT_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Defines how a node should be splitted
    */
   template <class Database>
   class TreeNodeSplit
   {
   public:
      typedef typename Database::Sample::Input::value_type  value_type;
      typedef typename Database::Sample::Input              Point;

   public:
      /**
       @param weights the weights given to each sample. it must sum to 1
       */
      virtual void compute( const Database& dat, const std::vector<float>& weights, std::vector<Database>& split_out, std::vector< std::vector<float> >& weights_out ) = 0;
      virtual ui32 test( const Point& p ) const = 0;
      virtual void print( std::ostream& o ) const = 0;

      virtual ~TreeNodeSplit()
      {}
   };

   /**
    @brief Create discrete node decision split

    For discrete values, we simply need to decide which feature will be used to split
    the samples. So for each feature, compute <Metric> and retain only this one
    */
   template <class Database, class Metric>
   class TreeNodeSplitDiscrete : public TreeNodeSplit<Database>
   {
      typedef std::map<ui32, ui32>  AttributMapper;

   public:
      virtual void compute( const Database& dat, const std::vector<float>& weights, std::vector<Database>& split_out, std::vector< std::vector<float> >& weights_out )
      {
         if ( dat.size() == 0 )
            return;
         const ui32 nbFeatures = static_cast<ui32>( dat[ 0 ].input.size() );

         ensure( dat.size() == weights.size(), "weights don't match" );

         std::vector<ui32> y( dat.size() );
         std::vector<ui32> x( dat.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            y[ n ] = dat[ n ].output;
         }

         // find the best feature
         double bestSplitMetric = -1e10;
         for ( ui32 feature = 0; feature < nbFeatures; ++feature )
         {
            Metric metric;

            for ( ui32 n = 0; n < dat.size(); ++n )
            {
               x[ n ] = static_cast<ui32>( dat[ n ].input[ feature ] );
            }

            const double splitMetric = metric.compute( x, y, weights );
            if ( splitMetric > bestSplitMetric )
            {
               bestSplitMetric = splitMetric;
               _featureId = feature;
            }
         }

         // here we are generating a mapper feature value -> bin id
         ui32 id = 0;
         AttributMapper attributMapper;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 featureValue = static_cast<ui32>( dat[ n ].input[ _featureId ] );
            AttributMapper::const_iterator it = attributMapper.find( featureValue );
            if ( it == attributMapper.end() )
            {
               attributMapper[ featureValue ] = id++;
            }
         }

         _attributMapper = attributMapper;

         // finally route the samples according to the split rule
         split_out = std::vector<Database>( _attributMapper.size() );
         weights_out = std::vector< std::vector< float > >( _attributMapper.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 branch = test( dat[ n ].input );
            split_out[ branch ].add( dat[ n ] );
            weights_out[ branch ].push_back( weights[ n ] );
         }
      }

      virtual ui32 test( const Point& p ) const
      {
         const value_type featureValue = static_cast<ui32>( p[ _featureId ] );
         AttributMapper::const_iterator it = _attributMapper.find( featureValue );
         ensure( it != _attributMapper.end(), "this feature value was never encountered during training! Can't classify" );
         return it->second;
      }

      ui32 getFeatureSplit() const
      {
         return _featureId;
      }

      virtual void print( std::ostream& o ) const
      {
         o << "feature split=" << _featureId;
      }

   private:
      ui32                 _featureId;          // the feature we are splitting
      AttributMapper       _attributMapper;     // map a feature ID to a bin
   };

   /**
    @brief Decision node for continuous attributs. A single split will be produced

    For continues values, it is a bit more complicated. The common information measure such
    as entropy, information gain can only be computed efficiently for discrete values. Consequently
    for a single feature, different splits must be evaluated using <SplittingCriteria>, and so for
    all the features.

    @param ContinuousSplittingCriteria the splitting criteria to be used for the continuous attributs
    @param Metric the metric to be used to select the best split, the higher, the better
    */
   template <class Database, class Metric, class SplittingCriteriaFactory>
   class TreeNodeSplitContinuousSingle : public TreeNodeSplit<Database>
   {
   public:
      typedef typename SplittingCriteriaFactory::value_type SplittingCriteria;
      typedef TreeNodeSplit<Database>                       Base;

      TreeNodeSplitContinuousSingle( const SplittingCriteriaFactory& scriteria ) : _splittingCriteriaFactory( scriteria ), _thresold( std::numeric_limits<value_type>::min() ), _featureId( (ui32)-1 )
      {}

      /**
       @brief Compute the best split
       @param dat the database to use
       @param 
       */
      void compute( const Database& dat, const std::vector<float>& weights, std::vector<Database>& split_out, std::vector< std::vector<float> >& weights_out )
      {
         if ( dat.size() == 0 )
            return;
         const ui32 nbFeatures = static_cast<ui32>( dat[ 0 ].input.size() );

         ensure( dat.size() == weights.size(), "weights don't match" );

         std::vector<ui32> y( dat.size() );
         std::vector<ui32> x( dat.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            y[ n ] = dat[ n ].output;
         }

         SplittingCriteria scriteria = _splittingCriteriaFactory.create();

         // select the best feature
         double bestSplitMetric = std::numeric_limits<double>::min();
         for ( ui32 feature = 0; feature < nbFeatures; ++feature )
         {
            Metric metric;

            std::vector<value_type> splits;
            scriteria.computeSplits( dat, feature, splits );

            // then select the best split
            for ( size_t split = 0; split < splits.size(); ++split )
            {
               for ( ui32 n = 0; n < dat.size(); ++n )
               {
                  x[ n ] = dat[ n ].input[ feature ] >= splits[ split ];
               }

               // check the split quality
               const double splitMetric = metric.compute( x, y, weights );
               if ( splitMetric > bestSplitMetric )
               {
                  bestSplitMetric = splitMetric;
                  _featureId = feature;
                  _thresold = splits[ split ];
               }
            }
         }

         // finally route the samples according to the split
         split_out = std::vector<Database>( 2 );
         weights_out = std::vector< std::vector< float > >( 2 );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 branch = test( dat[ n ].input );
            split_out[ branch ].add( dat[ n ] );
            weights_out[ branch ].push_back( weights[ n ] );
         }
      }

      /**
       @brief return the branch to be taken
       */
      ui32 test( const Point& p ) const
      {
         return p[ _featureId ] >= _thresold;
      }

      ui32 getFeatureSplit() const
      {
         return _featureId;
      }

      value_type getSplitThreshold() const
      {
         return _thresold;
      }

      virtual void print( std::ostream& o ) const
      {
         o << "feature split=" << _featureId << " threshold=" << _thresold;
      }

   private:
      SplittingCriteriaFactory      _splittingCriteriaFactory;    // criteria to be used for the splits
      ui32                          _featureId;    // the feature we are splitting
      value_type                    _thresold;     // the threshold used
   };
}
}

#endif