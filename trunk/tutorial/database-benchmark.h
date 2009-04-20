#ifndef DATABASE_BENCHMARK_H_
# define DATABASE_BENCHMARK_H_

# include <nll/nll.h>

# define NLL_DATABASE_PATH     "../../demo-nll-ocr/"
   
namespace nll
{
namespace benchmark
{
   /**
    @brief Holds a set of benchmarked database for evaluation purpose.
    */
   class BenchmarkDatabases
   {
      typedef core::Buffer1D<f64>            Point;
      typedef algorithm::Classifier<Point>   Classifier;

   public:
      typedef Classifier::Database           Database;

      /**
       @brief Represents a benchmarked database containing the database with the correct
              type (test, train, validation), the purpose of the database and the classification
              rate found for it (in the scientific litterature)
       */
      struct Benchmark
      {
         /**
          @param nameId a unique identifier
          */
         Benchmark( const std::string nameId, Database& dat, const std::string desc, double bestaccuracy )
         {
            name = nameId;
            database = dat;
            description = desc;
            accuracy = bestaccuracy;
         }

         Database       database;
         std::string    description;
         double         accuracy;
         std::string    name;
      };
      typedef std::vector<Benchmark> Benchmarks;

   public:
      static BenchmarkDatabases& instance()
      {
         static BenchmarkDatabases i;
         return i;
      }

      void add( const Benchmark benchmark )
      {
         for ( ui32 n = 0; n < _benchmarks.size(); ++n )
            assert( benchmark.name != _benchmarks[ n ].name );
         _benchmarks.push_back( benchmark );
      }

      const Benchmark& operator[]( ui32 n ) const
      {
         return _benchmarks[ n ];
      }

      const Benchmark* find( const std::string& s ) const
      {
         for ( ui32 n = 0; n < _benchmarks.size(); ++n )
            if ( _benchmarks[ n ].name == s )
               return &_benchmarks[ n ];
         return 0;
      }

   private:
      // static nature
      BenchmarkDatabases(){}
      BenchmarkDatabases& operator=( const BenchmarkDatabases );
      BenchmarkDatabases( const BenchmarkDatabases& );

   private:
      Benchmarks     _benchmarks;
   };
}
}
#endif
