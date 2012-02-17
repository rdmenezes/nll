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

#ifndef NLL_CLASSIFIER_NEAREST_NEIGHBOR_H_
# define NLL_CLASSIFIER_NEAREST_NEIGHBOR_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Nearest neighbor classification algorithm. Ensure metric is alive while
           the classifier is used.
    @note as the knn classifier is built upon a kd-tree, implying constraints to
          the metric that can be used. Please refer to the <code>KdTree</code>
    
    Metric needs to provide: double distance( const Point& p1, const Point& p2 ) const;
   
    */
   template <class Point, class Metric>
   class ClassifierNearestNeighbor : public Classifier<Point>
   {
   public:
      typedef Classifier<Point>  Base;

   private:
      typedef core::DatabaseInputAdapterRead<typename Base::Database> Adapter;
      typedef KdTree<typename Adapter::Point, Metric, 5, Adapter> NnKdTree;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 30, 5, 3, 1 ) );
         return parameters;
      }

   public:
	  /**
	   @param metric the metric used to compare. This <code>metric</code> variable must be valid until
                    this algorithm is used.
	   @param k the number of examples to be used
	   */
      ClassifierNearestNeighbor( const Metric* metric ) : Base( buildParameters() ), _metric( metric ), _k( 0 ), _tree( *metric ), _adapter( 0 )
      {
         ensure( metric, "error: no metric" );
      }

      ClassifierNearestNeighbor( const std::string& f, const Metric* metric ) : Base( buildParameters() ), _metric( metric ), _tree( *metric ), _adapter( 0 )
      {
         ensure( metric, "error: no metric" );
         Base::read( f );
      }

      const Metric* getMetric() const { return _metric; }

      virtual ~ClassifierNearestNeighbor()
      {
         if ( _adapter )
            delete _adapter;
      }

      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierNearestNeighbor<Point, Metric>* classifier = new ClassifierNearestNeighbor<Point, Metric>( _metric );
         classifier->_k = _k;
         classifier->_dat = _dat;
         classifier->_crossValidationBin = this->_crossValidationBin;
         return classifier;
      }

      /**
       @note the tree is not saved: it has to be recomputed when loaded
       */
      virtual void read( std::istream& i )
      {
         _dat.read( i );
		   core::read<ui32>( _k, i );
         core::read<ui32>( _nbClasses, i );
         
         // rebuild the tree if the database is not empty
         if ( _adapter )
            delete _adapter;
         _adapter = new Adapter( _dat );
         if ( _adapter->size() )
            _tree.build( *_adapter, (*_adapter)[ 0 ].size() );

      }
      virtual void write( std::ostream& o ) const
      {
         _dat.write( o );
		   core::write<ui32>( _k, o );
         core::write<ui32>( _nbClasses, o );
      }

	   /**
       @throw std::runtime_error if there is no suitable database to compare to
	   */
      virtual typename Base::Class test( const Point& p ) const
      {
         core::Buffer1D<double> probability;
         return test( p, probability );
      }

      virtual typename Base::Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         assert( _dat.size() );
         if ( !_dat.size() )
             throw std::runtime_error( "ClassifierNearestNeighbor: the database is empty, can't classify" );
         probability = core::Buffer1D<double>( _nbClasses );


         typename NnKdTree::NearestNeighborList list = _tree.findNearestNeighbor( p, _k );

         typedef std::map<ui32, ui32>  Count;
         Count count;
         for ( typename NnKdTree::NearestNeighborList::const_iterator it = list.begin(); it != list.end(); ++it )
            ++count[ _dat[ it->id ].output ];

         ui32 index = 0;
         ui32 max_index = 0;
         for ( Count::const_iterator it = count.begin(); it != count.end(); ++it )
         {
            if ( max_index < it->second )
            {
               max_index = it->second;
               index = it->first;
            }
            probability[ it->first ] = static_cast<double>( it->second ) / list.size();
         }
         return index;
      }

      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         ensure( parameters.size() == 1, "expected size: 1" );
         ensure( parameters[ 0 ] > 0, "bad argument" );
         _k = static_cast<ui32>( parameters[ 0 ] );
         _dat = core::filterDatabase( dat, core::make_vector<ui32>( Base::Database::Sample::LEARNING ), static_cast<ui32>( Base::Database::Sample::LEARNING ) );

         if ( _adapter )
            delete _adapter;
         _adapter = new Adapter( _dat );  
         if ( _adapter->size() )
         {
            _tree.build( *_adapter, (*_adapter)[ 0 ].size() );
         }

         _nbClasses = getNumberOfClass( dat );
      }

   protected:
      // can't copy it
      ClassifierNearestNeighbor& operator=( const ClassifierNearestNeighbor& );

   protected:
      typename Base::Database  _dat;
      const Metric*            _metric;
	   ui32                     _k;
      NnKdTree                 _tree;
      Adapter*                 _adapter;
      ui32                     _nbClasses;
   };
}
}

#endif
