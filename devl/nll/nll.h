#ifndef NLL_H
#define NLL_H

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NLL_EXPORTS
#define NLL_API __declspec(dllexport)
#else
#define NLL_API __declspec(dllimport)
#pragma comment(lib, "nll.lib")
#endif
#else
#define NLL_API
#endif

// define NLL_DISABLE_SSE_SUPPORT macro to disable all optimizations using SSE
// #define NLL_DISABLE_SSE_SUPPORT

// define the NLL_NOT_MULTITHREADED macro if NLL needs not to be thread safe. By default it is thread safe.
//#define NLL_NOT_MULTITHREADED

#ifndef NLL_NOT_MULTITHREADED
# include <omp.h>
#endif

/**
 @mainpage Numerical Learning Library
   The Numerical Learning Library intends to provide a wide range of machine learning
   and machine vision algorithms such as neural networks, support vector machine,
   hidden markcov model, clustering. It is a generic and efficient C++ library with
   its full integrated framework : feature creation, feature selection, feature
   transformation, preprocessing, classification and validation algorithms.
 @author Ludovic Sibille
 @version 0.12
 @date 11th June 2010
 */

/// define the version of nll
#define NLL_VERSION  "nll-0.12"


#ifdef _MSC_VER
# define NLL_ALIGN_16   __declspec(align(16))
#else
# define NLL_ALIGN_16   // TODO: find equivalent for gcc
#endif

/**
 Concept: (for future integration with C++ 0x)
   - Point : requires
         * T& operator[]( ui32 n );
         * ui32 size() const;
         * contructor( ui32 size );
         * typedef internal_point_type value_type;
   - Database: should be used as a template instead of the concrete type to allow different kind of database
   - IndexMapper1D
   - IndexMapper2D
   - IndexMapper2Dn
   - Interpolator2D
   - Convolution
   - ClassificationSample
   - Metric : requires
         * copy constructor
         * distance
   - KMeansUtility
         typedefs
         * null
         * add
         * div
   - Allocator
         * allocate
         * deallocate
         * rebind
   - matrix
 */

# include <fstream>
# include <stdexcept>
# include <cstdlib>

/**
 @defgroup core

 This group defines the core functions of this library. It is designed to provide effifient
 and flexible datatypes and algorithms. The main groups belonging to this group are:
 - buffers
 - vectors
 - matrices
 - images
 - databases
 - math
 */
# include "singleton.h"
# include "configuration.h"
# include "types.h"
# include "utility-pure.h"
# include "math-floor.h"
# include "static-assert.h"
# include "ensure.h"
# include "log.h"
# include "logger.h"
# include "unreachable.h"
# include "id-maker.h"
# include "static-vector.h"
# include "buffer1D.h"
# include "buffer1D-converter.h"
# include "math.h"
# include "matrix.h"
# include "math-distribution-gaussian.h"
# include "math-distribution-uniform.h"
# include "math-correlation.h"
# include "math-sampling.h"
# include "math-quaternion.h"
# include "matrix-basic-operations.h"
# include "matrix-decomposition.h"
# include "matrix-basic.h"
# include "matrix-covariance.h"
# include "matrix-converter.h"
# include "matrix-sort.h"
# include "matrix-svd.h"
# include "matrix-custom.h"
# include "matrix-cholesky.h"
# include "math-distribution-multinormal.h"
# include "type-traits.h"
# include "type-traits-inherited.h"
# include "type-traits-memory.h"
# include "io.h"
# include "image.h"
# include "image-interpolator.h"
# include "image-io.h"
# include "image-transformation.h"
# include "image-threshold.h"
# include "image-utility.h"
# include "image-convolution.h"
# include "image-converter.h"
# include "image-resample.h"
# include "image-binary-transformation.h"
# include "image-snr.h"
# include "image-mask.h"
# include "image-morphology.h"
# include "image-distance-transform.h"
# include "database.h"
# include "database-manipulation.h"
# include "database-input-adapter.h"
# include "utility.h"
# include "timer.h"
# include "sequence-converter.h"
# include "allocator-aligned.h"
# include "geometry.h"

/**
 @defgroup imaging

 This group defines the main algorithms for imaging such as volumes, interpolation and
 images.
 */
# include "transformation.h"
# include "lut.h"
# include "volume.h"
# include "volume-spatial.h"
# include "volume-interpolator.h"
# include "volume-resampling.h"
# include "slice.h"
# include "slice-blending.h"
# include "slice-resampling.h"
# include "multiplanar-reconstruction.h"
# include "volume-io-mf2.h"
# include "volume-io-bin.h"
# include "volume-io-txt.h"
# include "maximum-intensity-projection.h"
# include "volume-barycentre.h"

/**
 @defgroup algorithm

 This group defines the main algorithms of the library. They are mainly decomposed into
 2 sub groups:
 - generic algorithm group, that tries to keep the dependencies to a minimum.
 - feature algorithm group, where it intregrates all the generic algorithms to
   the developped framework.
 */
# include "bracketing.h"
# include "brent.h"
# include "powell.h"
# include "mlp.h"
# include "pca.h"
# include "quadratic-discriminant.h"
# include "stopping-condition.h"
# include "metric.h"
# include "genetic-algorithm.h"
# include "optimizer-parameters.h"
# include "optimizer-client.h"
# include "optimizer.h"
# include "optimizer-genetic-algorithm.h"
# include "optimizer-harmony-search.h"
# include "optimizer-harmony-search-memory.h"
# include "optimizer-grid-search.h"
# include "optimizer-powell.h"
# include "classifier-base.h"
# include "classifier.h"
# include "kmeans.h"
# include "gmm.h"
# include "svm.h"
# include "lsdbc.h"
# include "kd-tree.h"
# include "gabor-filter.h"
# include "gabor.h"
# include "markov-chain.h"
# include "hmm.h"
# include "naive-bayes.h"
# include "hmm-continuous.h"
# include "kernel-functions.h"
# include "kernel-pca.h"
# include "classifier-gmm.h"
# include "classifier-mlp.h"
# include "classifier-discriminant.h"
# include "classifier-naive-bayes.h"
# include "classifier-nearest-neighbor.h"
# include "classifier-svm.h"
# include "classifier-adaboost.h"
# include "region-growing.h"
# include "labelize.h"
# include "relief.h"
# include "normalization.h"
# include "feature-transformation.h"
# include "feature-transformation-pca.h"
# include "feature-transformation-kernel-pca.h"
# include "feature-transformation-normalization.h"
# include "feature-transformation-discriminant.h"
# include "feature-combiner.h"
# include "feature-selection.h"
# include "feature-selection-genetic-algorithm.h"
# include "feature-selection-best-first.h"
# include "feature-selection-pearson.h"
# include "feature-selection-relieff.h"
# include "regression.h"
# include "regression-mlp.h"
# include "regression-svm-nu.h"
# include "haar-features.h"

/// @defgroup utility
# include "mask-exporter.h"

/// @defgroup debug
# include "debug-io.h"
# include "debug-decompose.h"


#endif
