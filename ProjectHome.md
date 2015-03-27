# Numerical learning library #

NLL is a multi-platform open source project entirely written in C++. Its goal is to propose generic and efficient algorithms for machine learning and more specifically computer vision. It is intended to be very easy to integrate and it is mainly composed of header files with no dependency on any library but the STL.

# Architecture #

NLL implements generic algorithms using template metaprogramming and a minimalist interface. Several layers are used:
  * core: the very basic structures and operations
  * algorithm\_impl: generic algorithms with very limited dependencies and interface
  * algorithm: algorithms taking advantage of the NLL framework
  * imaging: algorithms related to imaging techniques such as volumes, slices, blending, lut tables, multi-planar reconstruction or maximum intensity projection.


# Details #

Here is an overview of some algorithms implemented in NLL:
  * classifiers (k-nearest neighbour, multi-layered neural networks, support vector machines, boosting, gaussian mixture model, quadratic discriminant, radial basis function, naive bayes)
  * feature selection (best-first, wrapper using genetic algorithm, relief-f, pearson)
  * feature transformation (PCA, kernel PCA, ICA)
  * optimizers (grid search, harmony search, genetic algorithms, powell)
  * math library (matrix, vector, linear algebra, distributions)
  * image library (resampling, morphology, transformations, convolutions, region growing, labeling, SURF)
  * visualization of high-dimensional data (locally linear embedding, Sammon's mapping)
  * volume library (resampling, maximum intensity projection, multi-planar reconstruction)
  * clustering (k-means, LSDBC)
  * kd-trees, gabor filters, haar features
  * markov chain, hidden markov model
  * RANSAC estimator
  * ... and much more soon!

# Example #

Here is a typical use of the framework:
```
/**
 In this test a neural network will be optimized using a harmony search algorithm.
 */
void test()
{
   typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
   typedef nll::algorithm::Classifier<Input>                            Classifier;

   // find the cancer1.dt benchmark
   const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
   ensure( benchmark, "can't find benchmark" );
   Classifier::Database dat = benchmark->database;

   // use a multi layered perceptron as a classifier
   typedef algorithm::ClassifierMlp<Input> ClassifierImpl;
   ClassifierImpl classifier;

   // optimize the parameters of the classifier on the original dataset
   // we will use a harmony search algorithm.
   // For each point, the classifier is evaluated: a 10-fold cross validation is
   // run on the learning database
   Classifier::OptimizerClientClassifier classifierOptimizer = classifier.createOptimizer( dat );

   // configure the optimizer options
   nll::algorithm::StopConditionIteration stop( 10 );
   nll::algorithm::MetricEuclidian<nll::algorithm::OptimizerHarmonySearchMemory::TMetric::value_type> metric;
   nll::algorithm::OptimizerHarmonySearchMemory parametersOptimizer( 5, 0.8, 0.1, 1, &stop, 0.01, &metric );

   // run the optimizer on the default constrained classifier parameters
   // if the default values don't fit, other constraint parameters should be given
   std::vector<double> params = parametersOptimizer.optimize( classifierOptimizer, ClassifierImpl::buildParameters() );
   
   // learn the LEARNING and VALIDATION database with the optimized parameters, and test the classifier
   // on the TESTING database
   classifier.learnTrainingDatabase( dat, nll::core::make_buffer1D( params ) );
   Classifier::Result rr = classifier.test( dat );

   TESTER_ASSERT( rr.testingError < 0.025 );
}

```


# Tutorials #

Here for a list of tutorials and samples of code: [Tutorials](Tutorials.md)