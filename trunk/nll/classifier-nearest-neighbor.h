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
    
    Metric needs to provide double operator()( const Point& point )
   
    */
   template <class Point, class Metric>
   class ClassifierNearestNeighbor : public Classifier<Point>
   {
   public:
      typedef Classifier<Point>  Base;

   private:
      typedef core::DatabaseInputAdapterRead<typename Base::Database> Adapter;
      typedef KdTree<typename Adapter::Point, Metric, 5, Adapter> KdTree;

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
         return classifier;
      }

      /**
       @note the tree is not saved: it has to be recomputed when loaded
       */
      virtual void read( std::istream& i )
      {
         _dat.read( i );
		   core::read<ui32>( _k, i );
         
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
      }

	   /**
       @throw std::exception if there is no suitable database to compare to
	   */
      virtual typename Base::Class test( const Point& p ) const
      {
         assert( _dat.size() );
         if ( !_dat.size() )
             throw std::runtime_error( "ClassifierNearestNeighbor: the database is empty, can't classify" );

         typename KdTree::NearestNeighborList list = _tree.findNearestNeighbor( p, _k );

         typedef std::map<ui32, ui32>  Count;
         Count count;
         for ( typename KdTree::NearestNeighborList::const_iterator it = list.begin(); it != list.end(); ++it )
            ++count[ _dat[ it->id ].output ];

         ui32 index = 0;
         ui32 max_index = 0;
         for ( Count::const_iterator it = count.begin(); it != count.end(); ++it )
            if ( max_index < it->second )
            {
               max_index = it->second;
               index = it->first;
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
      }

   protected:
      // can't copy it
      ClassifierNearestNeighbor& operator=( const ClassifierNearestNeighbor& );

   protected:
      typename Base::Database  _dat;
      const Metric*            _metric;
	   ui32                     _k;
      KdTree                   _tree;
      Adapter*                 _adapter;
   };
}
}

#endif
