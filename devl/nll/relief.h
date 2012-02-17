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

#ifndef NLL_RELIEF_H_
# define NLL_RELIEF_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Implementation of RELIEF-F (multi class) feature selection algorithm,
           suitable for a very high dimensionality space (http://sci2s.ugr.es/publications/ficheros/0782.pdf)
           (pythia.inf.brad.ac.uk/paper/FSKD05.ppt)
           full analysis here: www.tsi.enst.fr/~campedel/Biblio/FeatureSelection/robnik03-mlj_RReliefF.pdf
           It is implemented using a kd-tree.
    */
   template <class Point>
   class Relieff
   {
   public:
      typedef typename Classifier<Point>::Database Database;
      typedef core::Buffer1D<double>               FeatureRank;

   public:
      /**
       @param dat the database containing the sample
       @param nbFeatures select the best subset features from the initial feature set.
       @param k the number of neighbours considered
       */      
      FeatureRank process( const Database& dat, ui32 steps = 1000, ui32 k = 10 )
      {
         typedef core::Buffer1D<typename Database::Sample::Input>          Points;
         typedef KdTree<Point, MetricEuclidian<Point>, 5, Points>          KdTree;
         typedef core::Buffer1D<typename Database::Sample::Input>          Inputs;
         typedef core::Buffer1D<Inputs>                                    SortedSamples;
         assert( dat.size() );

         const ui32 nbclass = core::getNumberOfClass( dat );
         const ui32 nbFeatures = dat[ 0 ].input.size();
         assert( nbFeatures );

         // sort instances by class, build a kd-tree for each class
         core::Buffer1D< core::Buffer1D<ui32> > indexes( nbclass );
         core::Buffer1D<ui32> nbSampleByClass( nbclass );
         SortedSamples sortedSamples( nbclass, true );
         for ( ui32 n = 0; n < dat.size(); ++n )
            ++nbSampleByClass[ dat[ n ].output ];
         for ( ui32 n = 0; n < nbclass; ++n )
            assert( nbSampleByClass[ n ] );
         for ( ui32 n = 0; n < nbclass; ++n )
         {
            sortedSamples[ n ] = Inputs( nbSampleByClass[ n ] );
            indexes[ n ] = core::Buffer1D<ui32>( nbSampleByClass[ n ] );
         }

         // compute min and max for each feature
         core::Buffer1D<f64> minFeature( nbFeatures );
         core::Buffer1D<f64> maxFeature( nbFeatures );
         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            minFeature[ n ] = std::numeric_limits<f64>::max();
            minFeature[ n ] = std::numeric_limits<f64>::min();
         }
         for ( ui32 n = 0; n < dat.size(); ++n )
            for ( ui32 m = 0; m < nbFeatures; ++m )
            {
               f64 val = dat[ n ].input[ m ];
               if ( minFeature[ m ] > val )
                  minFeature[ m ] = val;
               if ( maxFeature[ m ] < val )
                  maxFeature[ m ] = val;
            }

         // compute the probability to find a specific class
         core::Buffer1D<f64> probability( nbclass );
         for ( ui32 n = 0; n < nbclass; ++n )
            probability[ n ] = static_cast<f64>( nbSampleByClass[ n ] ) / dat.size();

         // build indexes and inverse
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            ui32 c = dat[ n ].output;
            --nbSampleByClass[ c ];
            sortedSamples[ dat[ n ].output ][ nbSampleByClass[ c ] ] = dat[ n ].input;
            indexes[ c ][ nbSampleByClass[ c ] ] = n;  // build the inversed index: from the sorted samples, find the original index in the database
         }
         nbSampleByClass.unref(); // do not use it anymore
         std::vector<KdTree> kdtrees( nbclass ); // TODO here a bug if core::Buffer1D<KdTree> instead -> memset(0) wich destroy data of the STL containers
         for ( ui32 n = 0; n < nbclass; ++n )
            kdtrees[ n ].build( sortedSamples[ n ], nbFeatures );
         
         // run the relieff algorithm
         FeatureRank featureRank( nbFeatures );
         typename KdTree::NearestNeighborList* neighbours = new typename KdTree::NearestNeighborList[ nbclass ];
         for ( ui32 n = 0; n < steps; ++n )
         {
            // select a sample
            ui32 c = rand() % nbclass; // TODO: check // probability is changed: instead of randomly choosing a sample, choose a class and then a sample from this class
            ui32 instance = rand() % sortedSamples[ c ].size();
            const ui32 indexOrig = indexes[ c ][ instance ];

            // compute the class misses and hit

            // TODO CHECK INDEX!!
            for ( ui32 nn = 0; nn < nbclass; ++nn )
               neighbours[ nn ] = kdtrees[ nn ].findNearestNeighbor( dat[ indexOrig ].input, k );

            // update the weight
            for ( ui32 nn = 0; nn < nbFeatures; ++ nn )
            {
               f64 pnc = 0;
               for ( ui32 mm = 0; mm < nbclass; ++mm )
               {
                  f64 diff = 0;
                  for ( typename KdTree::NearestNeighborList::iterator it = neighbours[ mm ].begin(); it != neighbours[ mm ].end(); ++it )
                     diff += core::absolute( static_cast<f64>( dat[ indexes[ mm ][ it->id ] ].input[ nn ] - dat[ indexes[ c ][ instance ] ].input[ nn ] ) / ( minFeature[ nn ] - maxFeature[ nn ] ) );
                  diff /= ( steps * k );

                  if ( mm == c )
                     pnc -= diff;
                  else
                     pnc += probability[ mm ] / ( 1 - probability[ c ] ) * diff;
               }
               featureRank[ nn ] += pnc;
            }
         }
         delete[] neighbours;
         return featureRank;
      }
   };
}
}

#endif
