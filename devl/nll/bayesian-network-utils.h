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

#ifndef NLL_ALGORITHM_BAYESIAN_NETWORK_UTILS_H_
# define NLL_ALGORITHM_BAYESIAN_NETWORK_UTILS_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      template <class BNetwork>
      class UtilsHolder
      {
      public:
         typedef BNetwork                                Bn;
         typedef typename Bn::Graph                      Graph;
         typedef typename BNetwork::Factor               Factor;

         template <class Functor>
         class ConstBnVisitor : public core::GraphVisitorBfs<Graph>
         {
         public:
            typedef typename Graph::const_vertex_iterator   const_vertex_iterator;

         public:
            ConstBnVisitor( const Bn& bn, Functor& fun ) : _bn( bn ), _func( fun )
            {}

            void run()
            {
               visit( _bn.getNetwork() );
            }

         private:
            virtual void discoverVertex( const const_vertex_iterator& it, const Graph& )
            {
               const Factor& f = _bn.getFactors()[ it ];
               _func( f );
            }

         private:
            // no copy allowed
            ConstBnVisitor& operator=( const ConstBnVisitor& );
            ConstBnVisitor( const ConstBnVisitor& );

         private:
            const Bn&   _bn;
            Functor&    _func;
         };

         template <class Functor>
         class BnVisitor : public core::GraphVisitorBfs<Graph>
         {
         public:
            typedef typename Graph::const_vertex_iterator   const_vertex_iterator;

         public:
            BnVisitor( Bn& bn, Functor& fun ) : _bn( bn ), _func( fun )
            {}

            void run()
            {
               visit( _bn.getNetwork() );
            }

         private:
            virtual void discoverVertex( const const_vertex_iterator& it, const Graph& )
            {
               Factor& f = _bn.getFactors()[ it ];
               _func( f );
            }

         private:
            // no copy allowed
            BnVisitor& operator=( const BnVisitor& );
            BnVisitor( const BnVisitor& );

         private:
            Bn&         _bn;
            Functor&    _func;
         };

         class GetConstFactorsFunctor
         {
         public:
            void clear()
            {
               _factors.clear();
            }

            void operator()( const Factor& f )
            {
               _factors.push_back( &f );
            }

            const std::vector<const Factor*>& getFactors() const
            {
               return _factors;
            }

         private:
            std::vector<const Factor*> _factors;
         };

         class GetFactorsFunctor
         {
         public:
            void clear()
            {
               _factors.clear();
            }

            void operator()( Factor& f )
            {
               _factors.push_back( &f );
            }

            const std::vector<Factor*>& getFactors() const
            {
               return _factors;
            }

         private:
            std::vector<Factor*> _factors;
         };
      };
   }

   /**
    @ingroup algorithm
    @brief Returns a pointer for each factor belonging to a bayesian network
    */
   template <class Factor>
   void getFactors( const BayesianNetwork<Factor>& bnet, std::vector<const Factor*>& factors_out )
   {
      typedef BayesianNetwork<Factor> Network;
      typedef impl::UtilsHolder<Network>::GetConstFactorsFunctor Functor;
      typedef impl::UtilsHolder<Network>::ConstBnVisitor<Functor> Visitor;

      Functor factorFunctor;
      Visitor visitorGetFactors( bnet, factorFunctor );
      visitorGetFactors.run();

      factors_out = factorFunctor.getFactors();
   }

   /**
    @ingroup algorithm
    @brief Returns a pointer for each factor belonging to a bayesian network
    */
   template <class Factor>
   void getFactors( BayesianNetwork<Factor>& bnet, std::vector<Factor*>& factors_out )
   {
      typedef BayesianNetwork<Factor> Network;
      typedef impl::UtilsHolder<Network>::GetFactorsFunctor Functor;
      typedef impl::UtilsHolder<Network>::BnVisitor<Functor> Visitor;

      Functor factorFunctor;
      Visitor visitorGetFactors( bnet, factorFunctor );
      visitorGetFactors.run();

      factors_out = factorFunctor.getFactors();
   }
}
}

#endif