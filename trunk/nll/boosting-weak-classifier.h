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

#ifndef NLL_ALGORITHM_BOOSTING_WEAK_CLASSIFIER_H_
# define NLL_ALGORITHM_BOOSTING_WEAK_CLASSIFIER_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Generic interface for weak classifiers, designed to be used with <code>Adaboost</code>
    */
   template <class DatabaseT>
   class WeakClassifier
   {
   public:
      typedef DatabaseT    Database;
      typedef float        value_type;

      /**
       @brief Learn the weighted database.
       @param database it is assumed all data in <code>database</code> are training data. It must contain exactly two classes
       @param weights the weights associated to each data sample. It is assumed sum(weights) = 1
       */
      virtual value_type learn( const Database& database, const core::Buffer1D<value_type> weights ) = 0;
      virtual size_t test( const typename Database::Sample::Input& input ) const = 0;

      virtual ~WeakClassifier()
      {}
   };
}
}

#endif