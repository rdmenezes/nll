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

#ifndef NLL_H
#define NLL_H

#pragma warning(disable:4996) // we don't care about this warning!

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

// define this in debug mode to find memory leaks
//#define NLL_FIND_MEMORY_LEAK

#ifdef NLL_FIND_MEMORY_LEAK
# ifdef _MSC_VER
#  ifdef _DEBUG
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#   define DEBUG_NEW     new(_NORMAL_BLOCK, __FILE__, __LINE__)
#   define new           DEBUG_NEW
#   define malloc(s)     _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#  else
  NOT HANDLED!
#  endif
# else
  NOT HANDLED!
#endif
#endif

#define PLACEMENT_NEW   new

#define NLL_INSTRUCTION_SET_BASIC      0
#define NLL_INSTRUCTION_SET_SSE        1
#define NLL_INSTRUCTION_SET_SSE2       2
#define NLL_INSTRUCTION_SET_SSE3       3
#define NLL_INSTRUCTION_SET_SSSE3      4
#define NLL_INSTRUCTION_SET_SSE41      5
#define NLL_INSTRUCTION_SET_SSE42      6
#define NLL_INSTRUCTION_SET_AVX        7
#define NLL_INSTRUCTION_SET_AVX2       8

/**
 @brief Defines the current processor instruction set supported
 */
# define NLL_INSTRUCTION_SET NLL_INSTRUCTION_SET_SSE42

# if NLL_INSTRUCTION_SET >= 1
#  include <xmmintrin.h>
#  include <emmintrin.h>
#endif

// define the NLL_NOT_MULTITHREADED macro if NLL needs not to be thread safe. By default it is thread safe.
// #define NLL_NOT_MULTITHREADED

// if defined, extra checks will be performed to check preconditions/postconditions
#define NLL_SECURE

// if not defined, use LIBFFTW3 as the implementation of the FFT
// NOTE1 that if this flag is used, you must comply with the FFTW3 licensing terms
// NOTE2 if not defined, by default no FFT library will be binded
// #define NLL_DONT_USE_LIBFFTW3

#ifndef NLL_NOT_MULTITHREADED
# include <omp.h>
#endif

// define this constant to disable all multithreading in quick loops. This is a workaround for the problematic implementation
// of OpenMP in VS2010 with spin lock.
// See http://social.msdn.microsoft.com/Forums/en-AU/parallelcppnative/thread/528479c8-fb70-4b05-83ce-7a552fd49895
#define NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS

/**
 @mainpage Numerical Learning Library
   The Numerical Learning Library intends to provide a wide range of machine learning
   and machine vision algorithms such as neural networks, support vector machine,
   hidden markcov model, clustering. It is a generic and efficient C++ library with
   its full integrated framework : feature creation, feature selection, feature
   transformation, preprocessing, classification and validation algorithms.
 @author Ludovic Sibille
 @version 0.16
 @date 21th August 2012
 */

/// define the version of nll as a string
#define NLL_VERSION     "nll-0.16"

/// define the version of nll as a plain number
#define NLL_VERSION_ID  0x016

#ifdef _MSC_VER
# define NLL_ALIGN_16   __declspec(align(16))
#else
# define NLL_ALIGN_16   __attribute__((aligned(16)))
#endif

// we want an accurate floating point model
// DDF/RBF transformations are sensitive to this parameter!
/*
#pragma float_control( precise, on )
#pragma float_control( except, off )
#pragma fp_contract(on)
#pragma fenv_access(on)
*/

/**
 Concept: (for future integration with C++ 0x)
   - Point : requires
         * T& operator[]( size_t n );
         * size_t size() const;
         * contructor( size_t size );
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

# include <assert.h>
# include <fstream>
# include <stdexcept>
# include <cstdlib>
# include <memory>
# include <map>
# include <list>
# include <stack>
# include <vector>
# include <queue>
# include <numeric>
# include <string>
# include <limits>
# include <iostream>
# include <sstream>
# include <typeinfo>
# include <utility>
# include <cstring>

# include "types.h"

/**
 @defgroup vectorized

 Low level primitive blocs helping with vectorization
 Only available if at least SSE2 is supported
 */
# if NLL_INSTRUCTION_SET >= 2
#  include "constant.h"
#  include "vec4i.h"
#  include "vec4fb.h"
#  include "vec4f.h"
#endif

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
# include "indent.h"
# include "singleton.h"
# include "configuration.h"
# include "collection-wrapper.h"
# include "utility-pure.h"
# include "utility-matlab.h"
# include "math-floor.h"
# include "static-assert.h"
# include "ensure.h"
# include "histogram.h"
# include "context.h"
# include "log.h"
# include "logger.h"
# include "unreachable.h"
# include "id-maker.h"
# include "static-vector.h"
# include "type-traits.h"
# include "type-traits-inherited.h"
# include "type-traits-memory.h"
# include "buffer1D.h"
# include "buffer1D-convolve.h"
# include "Buffer1D-basic-op.h"
# include "math.h"
# include "matrix.h"
# include "math-distribution-gaussian.h"
# include "math-distribution-uniform.h"
# include "math-distribution-logistic.h"
# include "math-pdf-gaussian.h"
# include "math-statistical-independence.h"
# include "math-correlation.h"
# include "math-sampling.h"
# include "math-kurtosis.h"
# include "math-quaternion.h"
# include "math-gram-schmidt-ortho.h"
# include "math-noise-awgn.h"
# include "math-cumulative-gaussian-function.h"
# include "matrix-basic-operations.h"
# include "matrix-decomposition.h"
# include "matrix-basic.h"
# include "matrix-covariance.h"
# include "matrix-converter.h"
# include "matrix-sort.h"
# include "matrix-svd.h"
# include "matrix-custom.h"
# include "matrix-cholesky.h"
# include "matrix-pseudo-inverse.h"
# include "matrix-affine-decomposition.h"
# include "math-distribution-multinormal.h"
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
# include "geometry.h"
# include "matrix-affine-transformation.h"
# include "image-distance-transform.h"
# include "image-spatial.h"
# include "transformation-rbf.h"
# include "transformation-ddf-2d.h"
# include "image-spatial-transformation-mapper-ddf.h"
# include "image-spatial-transformation-mapper.h"
# include "image-spatial-resampling.h"
# include "database.h"
# include "database-manipulation.h"
# include "database-input-adapter.h"
# include "utility.h"
# include "timer.h"
# include "sequence-converter.h"
# include "allocator-aligned.h"
# include "converter.h"
# include "graph-adjency-list.h"
# include "graph-visitor.h"
# include "graph-traits.h"
# include "graph-algorithms.h"

// must be included before...
# include "metric.h"
# include "kmeans.h"
# include "gmm.h"
# include "histogram-fitting.h"
# include "data-compression-count.h"

/**
 @defgroup imaging

 This group defines the main algorithms for imaging such as volumes, interpolation and
 images.

 Often the library will use the concept of source and target volumes associated with affine transformation
 The "source" volume is a fixed volume, the "target" volume is the moving one. It is "moved" by an associated
 transformation (see multiplanar reconstruction). This associated transformation is ALWAYS given in the form source->target
 because this is how we display the volumes and it may not be possible to invert a transformation (or not efficiently, think
 about deformable tfm).
 */
# include "lut.h"
# include "transformation.h"
# include "volume.h"
# include "volume-spatial.h"
# include "volume-interpolator.h"
# include "volume-transformation-mapper.h"
# include "transformation-ddf.h"
# include "volume-transformation-mapper-ddf.h"
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
# include "volume-discretizer.h"
# include "volume-io-mf3.h"
# include "volume-distance-transform.h"
# include "ddf-overlay.h"

/**
 @defgroup algorithm

 This group defines the main algorithms of the library. They are mainly decomposed into
 2 sub groups:
 - generic algorithm group, that tries to keep the dependencies to a minimum.
 - feature algorithm group, where it intregrates all the generic algorithms to
   the developped framework.
 */
# include "fft.h"
# include "periodogram.h"
# include "function.h"
# include "gradient-descent.h"
# include "linear-regression.h"
# include "bracketing.h"
# include "brent.h"
# include "powell.h"
# include "mlp.h"
# include "normalization.h"
# include "pca.h"
# include "pca-sparse.h"
# include "quasi-periodicity-analysis.h"
# include "estimator-affine2D-transformation.h"
# include "estimator-affine-transformation.h"
# include "ransac.h"
# include "sammon-projection.h"
# include "ica.h"
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

# include "integral-image-3d.h"
# include "registration-algorithm.h"
# include "fast-volume-pyramid.h"
# include "joint-histogram.h"
# include "joint-histogram-similarity.h"
# include "registration-transformation-creator.h"
# include "registration-transformation-evaluator.h"
# include "registration-gradient-evaluator.h"
# include "registration-transformation-evaluator-similarity.h"
# include "registration-preprocessing-volume.h"
# include "registration-algorithm-intensity-affine.h"

# include "classifier-base.h"
# include "classifier.h"
# include "kmeans.h"
# include "gmm.h"
# include "histogram-fitting.h"
# include "rbf-network.h"
# include "svm.h"
# include "svm-linear.h"
# include "lsdbc.h"
# include "kd-tree.h"
# include "locally-linear-embedding.h"
# include "gabor-filter.h"
# include "gabor.h"
# include "markov-chain.h"
# include "hmm.h"
# include "naive-bayes.h"
# include "hmm-continuous.h"
# include "kernel-functions.h"
# include "kernel-pca.h"
# include "kernel-pca-preimage-mds.h"
# include "classifier-gmm.h"
# include "classifier-mlp.h"
# include "classifier-discriminant.h"
# include "classifier-naive-bayes.h"
# include "classifier-nearest-neighbor.h"
# include "classifier-svm.h"
# include "classifier-svm-linear.h"
# include "classifier-adaboost.h"
# include "classifier-rbf.h"
# include "region-growing.h"
# include "labelize.h"
# include "relief.h"
# include "feature-transformation.h"
# include "feature-transformation-pca.h"
# include "feature-transformation-ica.h"
# include "feature-transformation-kernel-pca.h"
# include "feature-transformation-normalization.h"
# include "feature-transformation-discriminant.h"
# include "feature-transformation-lle.h"
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
# include "pyramid-hessian-2d.h"
# include "surf-2d.h"
# include "point-based-registration-2d.h"
# include "haar-features-3d.h"
# include "pyramid-hessian-3d.h"
# include "surf-3d.h"
# include "point-based-registration-3d.h"
# include "registration-ct-ct-planar.h"
# include "perceptron-margin.h"

// Trees
# include "tree-continuous-splitting-criteria.h"
# include "information-measure.h"
# include "tree-node-split.h"
# include "decision-tree.h"

// Bayesian network related files
# include "potential-gaussian-moment.h"
# include "potential-gaussian-canonical.h"
# include "potential-table.h"
# include "bayesian-network.h"
# include "bayesian-network-utils.h"
# include "bayesian-network-ml-param-estimation.h"
# include "bayesian-network-sampling.h"
# include "bayesian-inference-naive.h"
# include "bayesian-inference-elimination-variable.h"

// Boosting
# include "boosting-weak-classifier.h"
# include "boosting-weak-classifier-stump.h"
# include "boosting-weak-classifier-mlp.h"
# include "boosting-weak-classifier-perceptron.h"
# include "boosting-weak-classifier-decision-tree.h"
# include "boosting-adaboost-basic.h"


/// @defgroup utility
# include "mask-exporter.h"
# include "statistics.h"

/// @defgroup debug
# include "debug-io.h"
# include "debug-decompose.h"


#endif
