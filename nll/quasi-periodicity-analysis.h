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

#ifndef NLL_ALGORITHM_QUASI_PERIODICITY_ANALYSIS_H_
# define NLL_ALGORITHM_QUASI_PERIODICITY_ANALYSIS_H_

namespace nll
{
namespace algorithm
{
   /**
      @brief Determine the period of a quasi periodic time serie
      @note Direct implementation of "Visual Quasi-Periodicity", 2008 E. Pogalin, A.W.M. Smeulders A.H.C. Thean
      @param Frames can in fact be any collection of fixed size data, not only "images"
      */
   template <class Frames>
   class QuasiPeriodicityAnalysis
   {
   public:
      typedef float                                                   value_type;
      typedef typename Frames::value_type                             Frame;
      typedef core::Matrix<double, core::IndexMapperRowMajorFlat2D>   Matrix;

      /**
         @brief Result summarizing the main steps of the algorithm that could be reused...
         */
      struct Result
      {
         Result()
         {
            periodicityStrength = 0;
            fundamentalFrequency = -1;
         }

         double                              periodicityStrength;
         double                              fundamentalFrequency;
         core::Buffer1D<double>              weightedSpectrum;
         algorithm::PcaSparseSimple<Frames>  pcaDecomposition;
      };

   public:
      /**
         @brief Analyze a set of frames to find its degree of periodicity and its period
         @param frames must be greyscale frames. The frames must be cropped and centred on the region of interest
         @param frequencyResolution is used as the tolerance for the peak matching
               it is defined by the sampling rate fs in hertz and the number of frames so that frequencyResolution = fs / nbSamples,
               see http://electronics.stackexchange.com/questions/12407/what-is-the-relation-between-fft-length-and-frequency-resolution
         @param varianceToRetain the variance to retain during the PCA step
         */
      Result analyse( const Frames& frames, double frequencyResolution, double varianceToRetain = 0.75 )
      {
         // sanity checks
         if ( frames.size() == 0 )
            return Result();
         const size_t frameSize = frames[ 0 ].size();
         const size_t nbFrames = static_cast<size_t>( frames.size() );

         {
            std::stringstream ss;
            ss << "started QuasiPeriodicityAnalysis::analyse..." << std::endl
               << "nbFrames=" << frames.size() << std::endl
               << "frameSize=" << frameSize << std::endl
               << "frequencyResolution=" << frequencyResolution << std::endl
               << "varianceToRetain=" << varianceToRetain;

            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         for ( size_t n = 0; n < nbFrames; ++n )
         {
            ensure( frameSize == frames[ n ].size(), "wrong size" );
         }

         // first do a PCA decomposition. The eigen vectors are representing the main "step" of the period
         //algorithm::PrincipalComponentAnalysis<Frames> pca;
         algorithm::PcaSparseSimple<Frames> pca;
         const bool success = pca.computeByVarianceRetained( frames, varianceToRetain );
         ensure( success, "PCA decomposition failed!" );
            
         // simply project each frame on the retained orthogonal basis, this will give us the reponse of a frame for each basis over time
         const size_t nbBasis = pca.getNbVectors();
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "PCA eigen vectors retained=" + core::val2str( nbBasis ) );

         // each column represent a projection of one image on all eigen vectors retained
         std::vector<core::Buffer1D<double>> responses( pca.getNbVectors() );
         double sumVariance = 0;
         for ( size_t n = 0; n < pca.getNbVectors(); ++n )
         {
            responses[ n ] = core::Buffer1D<double>( nbFrames, false );
            sumVariance += fabs( pca.getEigenValues()[ pca.getPairs()[ n ].second ] );
         }

         for ( size_t n = 0; n < frames.size(); ++n )
         {
            core::Buffer1D<float> projection = pca.process( core::Buffer1D<float>( frames[ n ] ) );
            assert( projection.size() == pca.getNbVectors() ); // "wrong size"
            for ( size_t nn = 0; nn < projection.size(); ++nn )
            {
               responses[ nn ][ n ] = projection[ nn ];
            }
         }

         //
         // now do spectrum analysis
         //
         std::vector<core::Buffer1D<double>> periodogramResponses( pca.getNbVectors() );
         for ( size_t n = 0; n < pca.getNbVectors(); ++n )
         {
            Periodogram periodogram;
            HanningWindow window;
            periodogramResponses[ n ] = periodogram.compute( responses[ n ], core::make_buffer1D<double>( 0.1, 0.2, 0.4, 0.2, 0.1 ), window );
         }

         // compute the weighted spectrum
         ensure( periodogramResponses.size(), "No component!" );
         const size_t spectrumSize = periodogramResponses[ 0 ].size();
         core::Buffer1D<double> weightedSpectrum( spectrumSize, false );
         for ( size_t n = 0; n < spectrumSize; ++n )
         {
            double sum = 0;
            for ( size_t id = 0; id < periodogramResponses.size(); ++id )
            {
               const double weight = pca.getEigenValues()[ pca.getPairs()[ id ].second ] / sumVariance;
               sum += weight * periodogramResponses[ id ][ n ];
            }
            weightedSpectrum[ n ] = sum;
         }

         {
            std::stringstream ss;
            ss << "weighted spectrum=" << std::endl;
            weightedSpectrum.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         double fundamentalFrequency = -1;
         double periodicityStrength = 0;
         _findFundamentalFrequency( (size_t)frames.size(), weightedSpectrum, frequencyResolution, fundamentalFrequency, periodicityStrength );

         {
            std::stringstream ss;
            ss << "fundamentalFrequency=" << fundamentalFrequency << std::endl
               << "periodicityStrength=" << periodicityStrength;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Result result;
         result.fundamentalFrequency = fundamentalFrequency;
         result.periodicityStrength = periodicityStrength;
         result.weightedSpectrum = weightedSpectrum;
         result.pcaDecomposition = pca;
         return result;
      }

   private:
      // here we are assuming the weightedSpectrum is smooth enough so that we can take the local minima to fin the frequency peaks
      static void _findFundamentalFrequency( size_t nbData, const core::Buffer1D<double>& weightedSpectrum, double frequencyResolution, double& fundamentalFrequency_out, double& periodicityStrength_out )
      {
         // find all the peaks
         std::vector<size_t> peaks;
         for ( size_t n = 0; n < weightedSpectrum.size(); ++n )
         {
            const double frequency = static_cast<double>( n ) / ( nbData );
            if ( frequency < frequencyResolution )
            {
               continue; 
            }

            const double valn = weightedSpectrum[ n ];
            if ( valn > 0.1 && n > 0 && n + 1 < weightedSpectrum.size() )
            {
               if ( valn > weightedSpectrum[ n - 1 ] && valn > weightedSpectrum[ n + 1 ] )
               {
                  peaks.push_back( n );
               }
            }
         }

         // computes the peak's left and right support. A peak support is the index until when a peak reaches zero
         // in practice, we use a threhold low enough
         std::vector<size_t> leftSupport( peaks.size() );
         std::vector<size_t> rightSupport( peaks.size() );
         for ( size_t n = 0; n < peaks.size(); ++n )
         {
            const double zero = 0.05 * peaks[ n ];
            size_t index = 0;
            for ( index = peaks[ n ] - 1; index != 0; --index )
            {
               if ( weightedSpectrum[ index ] <= zero )
                  break;
            }
            leftSupport[ n ] = index;
            if ( n > 0 && index < peaks[ n - 1 ] )
            {
               // extra check: we can't be 2 peaks apart..
               leftSupport[ n ] = peaks[ n - 1 ] + 1;
            }

            for ( index = peaks[ n ] + 1; index < weightedSpectrum.size(); ++index )
            {
               if ( weightedSpectrum[ index ] <= zero )
                  break;
            }
            rightSupport[ n ] = std::min<size_t>( index, weightedSpectrum.size() - 1 );
            if ( n + 1 < peaks.size() && rightSupport[ n ] > peaks[ n + 1 ] )
            {
               rightSupport[ n ] = peaks[ n + 1 ] - 1;
            }
         }


         // check support consistency: the right suppost may be found interleaved with the left support of the next peak,
         // so jzust split the support at the middle
         for ( int n = 0; n < (int)peaks.size() - 1; ++n )
         {
            size_t& right = rightSupport[ n ];
            size_t& left = leftSupport[ n + 1 ];
            if ( right > left )
            {
               const size_t middle = ( right + left ) / 2;
               right = middle;
               left = middle;
            }
         }

         // computes the energy of each peak
         std::vector<double> peakEnergy( peaks.size() );
         for ( size_t n = 0; n < peaks.size(); ++n )
         {
            const double e = std::accumulate( weightedSpectrum.begin() + leftSupport[ n ], weightedSpectrum.begin() + rightSupport[ n ] + 1, 0.0 );
            peakEnergy[ n ] = e;
         }

         const double totalEnergy = std::accumulate( weightedSpectrum.begin(), weightedSpectrum.end(), 0.0 );

         {
            std::stringstream ss;
            ss << "nbPeaks=" << peaks.size() << " spectrumTotalEnergy=" << totalEnergy << std::endl;
            for ( size_t n = 0; n < peaks.size(); ++n )
            {
               const double peakFrequency = static_cast<double>( peaks[ n ] ) / ( nbData );
               ss << "peak[ " << n << " ]=" << peaks[ n ] << " peakEnergy=" << peakEnergy[ n ] << " peakFrequency=" << peakFrequency << " leftSupport=" << leftSupport[ n ] << " rightSupport=" << rightSupport[ n ] << std::endl;
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // finally computes the (fundamental, harmonics) frequency maximizing the spectrum energy.
         // the fundamental must be of higher ernergy and its harmonics must be a multiple of the fundamental frequency
         // as several fundamental frequencies exist, we take the one that maximizes the spectrum energy
         // the tolerance used to match the harmonics is the frequency resolution fs / N
         double bestEnergy = 0;
         std::vector<double> bestFrequencyMatches;
         for ( size_t n = 0; n < peaks.size(); ++n )
         {
            std::vector<double> frequencyMatches;
            double energy = peakEnergy[ n ];
            const double fundamentalFrequency = static_cast<double>( peaks[ n ] ) / ( nbData );
            frequencyMatches.push_back( fundamentalFrequency );
            for ( size_t harmonic = n + 1; harmonic < peaks.size(); ++harmonic )
            {
               const double harmonicFrequency = static_cast<double>( peaks[ harmonic ] ) / ( nbData );
               const bool isMatching = core::isMultipleOf( fundamentalFrequency, harmonicFrequency, frequencyResolution );
               if ( isMatching )
               {
                  energy += peakEnergy[ harmonic ];
                  frequencyMatches.push_back( harmonicFrequency );
               }
            }

            if ( energy > bestEnergy )
            {
               bestEnergy = energy;
               bestFrequencyMatches = frequencyMatches;
            }

            {
               std::stringstream ss;
               ss << "matched peaks: total energy=" << energy << std::endl;
               for ( size_t n = 0; n < frequencyMatches.size(); ++n )
               {
                  ss << " peak frequency=" << frequencyMatches[ n ] << std::endl;
               }
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }
         }

         if ( bestFrequencyMatches.size() )
         {
            fundamentalFrequency_out = bestFrequencyMatches[ 0 ];
            periodicityStrength_out = bestEnergy / totalEnergy;

            // sometimes we take the left and right support of different peak to be the same value (however they should be very small!)
            // consequently we can sometimes be slightly over 1 in periodicity, so we perform extra check to "nomrlaize"
            periodicityStrength_out = std::min<double>( 1, periodicityStrength_out );
         } else {
            fundamentalFrequency_out = -1;
            periodicityStrength_out = 0;
         }
      }
   };
}
}

#endif