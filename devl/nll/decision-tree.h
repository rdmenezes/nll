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

#ifndef NLL_ALGORITHM_CLASSIFICATION_TREE_H_
# define NLL_ALGORITHM_CLASSIFICATION_TREE_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Generic decision tree
    */
   template <class Database>
   class DecisionTree
   {
   public:
      typedef typename Database::Sample::Input::value_type  value_type;
      typedef typename Database::Sample::Input              Point;
      typedef typename Database::Sample::Output             Class;
      typedef TreeNodeSplit<Database>                       NodeSplit;

      struct LevelData : public core::NonCopyable
      {
         LevelData( ui32 d, const Database&   dd ) : depth( d ), data( dd )
         {}

         ui32              depth;
         const Database&   data;
      };

      /**
       @brief Control the growing strategy of a tree
       */
      class GrowingStrategy
      {
      public:
         /**
          @brief returns true if the tree should continue growing
          @note think also about cases with pure nodes...
          */
         virtual bool continueGrowth( const LevelData& data ) const = 0;

         /**
          @brief returns the class of a current node
          */
         virtual Class getNodeClass( const Database& dat, const std::vector<float>& weights ) const = 0;
      };

      /**
       @brief grow a tree out of a database
       @param dat input database
       @param nodeFactory the factory that will create each decision node
       @param growingStrategy the strategy that will control how the tree is growing
       @param weights a weights attributed to each sample. It can be empty.
       */
      template <class NodeFactory>
      void compute( const Database& dat, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy, const core::Buffer1D<float> weights = core::Buffer1D<float>() )
      {
         ensure( weights.size() == dat.size() || weights.size() == 0, "weights must have the same size as database or empty" );
         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );

         std::vector<float> w;
         if ( weights.size() == 0 )
         {
            // if we don't have weights, just weight the samples equally
            const float wval = 1.0f / learning.size();
            w = std::vector<float>( learning.size(), wval );
         } else {
             w = std::vector<float>( weights.begin(), weights.end() );
         }

         _compute( learning, w, nodeFactory, growingStrategy, 0 );
      }

      /**
       @brief get the class of this point
       */
      ui32 test( const Point& p ) const
      {
         if ( _nodes.size() == 0 )
         {
            // we are at the leaf...
            return _nodeClass;
         }

         ui32 nodeId = _split->test( p );
         return _nodes[ nodeId ].test( p );
      }

      void print( std::ostream& o ) const
      {
         if ( _nodes.size() )
         {
            o << "non-leaf node:";
            _split->print( o );
            o << core::incendl;
            for ( size_t n = 0; n < _nodes.size(); ++n )
            {
               _nodes[ n ].print( o );
               if ( ( n + 1 ) != _nodes.size() )
                  o << core::iendl;
            }
            o << core::decindent;
         } else {
            o << "leaf node,  class=" << _nodeClass;
         }
      }

      const std::vector<DecisionTree>& getNodes() const
      {
         return _nodes;
      }

      const std::shared_ptr<NodeSplit>& getSplits() const
      {
         return _split;
      }

      /**
       @brief Only valid when getNodes().size() == 0
       */
      Class getNodeClass() const
      {
         return _nodeClass;
      }


   private:
      // recursively grow the tree on the sub nodes
      template <class NodeFactory>
      void _compute( const Database& dat, const std::vector<float>& weights, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy, ui32 level )
      {
         LevelData ld( level, dat );
         const bool continueGrowth = growingStrategy.continueGrowth( ld );

         if ( !continueGrowth )
         {
            _nodeClass = growingStrategy.getNodeClass( dat, weights );
            return;
         }

         std::vector<Database> dats;
         _split = std::shared_ptr<NodeSplit>( new NodeFactory::value_type( nodeFactory.create() ) );
         std::vector< std::vector< float > > weightsOut;
         _split->compute( dat, weights, dats, weightsOut );

         _nodes = std::vector<DecisionTree>( dats.size() );
         for ( size_t n = 0; n < dats.size(); ++n )
         {
            _nodes[ n ]._compute( dats[ n ], weightsOut[ n ], nodeFactory, growingStrategy, level + 1 );
         }
      }

   private:
      std::vector<DecisionTree>     _nodes;           // the nodes at n+1 level
      std::shared_ptr<NodeSplit>    _split;           // the decision split
      Class                         _nodeClass;       // if this node is a leaf, return this class
   };

   /**
    @brief Grow a fixed depth tree
    */
   template <class Database>
   class GrowingStrategyFixedDepth : public DecisionTree<Database>::GrowingStrategy
   {
   public:
      typedef typename DecisionTree<Database>::LevelData LevelData;
      typedef typename DecisionTree<Database>::Class     Class;

   public:
      GrowingStrategyFixedDepth( ui32 maxDepth = std::numeric_limits<ui32>::max() ) : _maxDepth( maxDepth )
      {}

      /**
       @brief returns true if the tree should continue growing
       */
      virtual bool continueGrowth( const LevelData& data ) const
      {
         ensure( data.data.size(), "hugh!? empty!" );

         // first ensure the node is not pure
         Class c = data.data[ 0 ].output;
         for ( ui32 n = 1; n < data.data.size(); ++n )
         {
            if ( c != data.data[ n ].output )
            {
               // the node is impure, check we are below the allowed depth
               return data.depth < _maxDepth;
            }
         }
         return false;  // node is pure! can't get better now...
      }

      /**
       @brief returns the class of a current node
       */
      virtual Class getNodeClass( const Database& dat, const std::vector<float>& weights ) const
      {
         // get the max class to be able to count the classes
         ui32 max = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            max = std::max<ui32>( dat[ n ].output, max );
         }

         // count the classes
         std::vector<float> counts( max + 1 );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const float w = weights[ n ];
            counts[ dat[ n ].output ] += w;
         }

         // return the max count class
         std::vector<float>::const_iterator it = std::max_element( counts.begin(), counts.end() );
         return static_cast<Class>( it - counts.begin() );
      }

   private:
      ui32  _maxDepth;
   };
}
}

#endif
