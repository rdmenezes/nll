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

#ifndef NLL_DATABASE_MANIPULATION_H_
# define NLL_DATABASE_MANIPULATION_H_

#pragma warning( push )
#pragma warning( disable:4127 ) // constant expression

namespace nll
{
namespace core
{
   namespace impl
   {
      template <class I, class O, class TypeSample>
      struct _IsClassificationSampleTraits
      {
         enum { value = Equal< ClassificationSample<I, O>, TypeSample >::value };
      };

      template <class Database, int hasClassTypes>
      struct _getNumberOfClass
      {
         ui32 getNumberOfClass( const Database& dat )
         {
            ensure( 0, "must never be reached" ); // unimplemented, implement one filter if database Samples has class information
            return 0;
         }
      };

      template <class Database>
      struct _getNumberOfClass<Database, 1>
      {
         ui32 getNumberOfClass( const Database& dat )
         {
            std::set<ui32> cl;
            ui32 max = 0;
            for ( ui32 n = 0; n < dat.size(); ++n )
            {
               max = std::max<ui32>( dat[ n ].output, max );
               cl.insert( dat[ n ].output );
            }
            assert( ( max + 1 ) == cl.size() ); // assert there is no "hole" in the class label
            return static_cast<ui32>( cl.size() );
         }
      };
   }

   /**
    @ingroup core
    @brief Returns the number of classes in the database. The database type <b>must</b> be a classification database
           (ie using <code>ClassificationSample</code>).
    */
   template <class Database>
   inline ui32 getNumberOfClass( const Database& dat )
   {
      return impl::_getNumberOfClass<Database, impl::_IsClassificationSampleTraits<typename Database::Sample::Input, typename Database::Sample::Output, typename Database::Sample>::value >().getNumberOfClass( dat );
   }

   /**
    @ingroup core
    @brief Computes the class distribution of the database for TESTING|LEARNING|VALIDATION
    */
   template <class Database>
   Buffer1D<ui32> getClassificationDatabaseClassNumber( const Database& d )
   {
      enum {VAL = impl::_IsClassificationSampleTraits<typename Database::Sample::Input, typename Database::Sample::Output, typename Database::Sample>::value };
      STATIC_ASSERT( VAL );

      ui32 nbClass = getNumberOfClass( d );
      ensure( nbClass, "empty database?" );
      Buffer1D<ui32> nbs( nbClass );
      for ( ui32 n = 0; n < d.size(); ++n )
      {
         ++nbs[ d[ n ].output ];
      }
      return nbs;
   }

   /**
    @ingroup core
    @brief Filters a database according to the type of the samples. The database type <b>must</b> be a classification database
           (ie using <code>ClassificationSample</code>).
    @param dat a <b>classification</b> database
    @param types the types to be selected
    @param newType the selected types will have <code>newType</code> type in the new database.
    */
   template <class Database>
   inline Database filterDatabase( const Database& dat, const std::vector<ui32> types, ui32 newType )
   {
      // only CLASSIFICATION database could filtered
      enum
      {
         VAL = impl::_IsClassificationSampleTraits<   typename Database::Sample::Input,
                                                      typename Database::Sample::Output,
                                                      typename Database::Sample  >::value
      };
      STATIC_ASSERT( VAL );
      Database ndat;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         for ( ui32 nn = 0; nn < types.size(); ++nn )
         {
            if ( static_cast<ui32>( dat[ n ].type ) == types[ nn ] )
            {
               ndat.add( dat[ n ] );
               ndat[ ndat.size() - 1 ].type = static_cast<typename Database::Sample::Type>( newType );
            }
         }
      }
      return ndat;
   }


   /**
    @ingroup core
    @brief Select features in a database. Assumes that all sample's input have the same size.
    @param dat the database to filter
    @param select an array bool specifying if this feature is kept or 
    */
   template <class Database>
   inline Database filterDatabase( const Database& dat, const Buffer1D<bool>& select )
   {
      // only CLASSIFICATION database could filtered
      enum
      {
         VAL = impl::_IsClassificationSampleTraits<   typename Database::Sample::Input,
                                                      typename Database::Sample::Output,
                                                      typename Database::Sample           >::value
      };
      STATIC_ASSERT( VAL );
      Database ndat;
      if ( !dat.size() )
         return ndat;
      ui32 sizeInput = dat[ 0 ].input.size();
      ensure( select.size() == sizeInput, "bad size" );
      for ( ui32 n = 1 ; n < dat.size(); ++n )
         assert( dat[ n ].input.size() == sizeInput );

      // build the index
      ui32 selectSize = 0;
      for ( ui32 n = 0; n < sizeInput; ++n )
         if ( select[ n ] )
            ++selectSize;
      Buffer1D<int> index( selectSize );
      ui32 ii = 0;
      for ( ui32 n = 0; n < sizeInput; ++n )
         if ( select[ n ] )
         {
            index[ ii ] = n;
            ++ii;
         }

      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         typedef typename Database::Sample::Input InputV;
         typename Database::Sample s;
         s.input = InputV( selectSize );
         s.output = dat[ n ].output;
         s.debug = dat[ n ].debug;
         s.type = dat[ n ].type;
         for ( ui32 nn = 0; nn < selectSize; ++nn )
            s.input[ nn ] = dat[ n ].input[ index[ nn ] ];
         ndat.add( s );
      }
      return ndat;
   }
}
}

#pragma warning( pop )

#endif
