/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_COLLECTION_WRAPPER_H_
# define NLL_COLLECTION_WRAPPER_H_

namespace nll
{
namespace core
{
   /**
    @brief This class will wrap a subset of a collection, without copying any data. This is designed to mimic std::vector interface

    The <Collection> must define size(), operator[] operations

    @note Typical use case is when multithreading is involved with ref counted object. We really don't want to internally increase the shared ref count,
          hence we only keep a const reference.
    */
   template <class Collection>
   class ConstCollectionWrapper
   {
   public:
      typedef typename Collection::value_type   value_type;
      typedef ui32                              index_type;

   public:
      ConstCollectionWrapper( const Collection& collection ) : _collection( collection )
      {}

      /**
       @brief Returns the size of the subset currently held
       */
      index_type size() const
      {
         return static_cast<index_type>( _indexes.size() );
      }

      /**
       @brief Insert an element to the <ConstCollectionWrapper> referencing the <index> element in <_collection>
       */
      void insertRef( index_type index )
      {
         _indexes.push_back( index );
      }

      /**
       @brief Returns the <index> element held in <ConstCollectionWrapper>
       */
      const value_type& operator[]( index_type index ) const
      {
         return _collection[ _indexes[ index ] ];
      }

      /**
       @brief Preallocate the size of the subset
       */
      void reserve( index_type nbElements )
      {
         _indexes.reserve( nbElements );
      }

   private:
      // no copy constructible
      ConstCollectionWrapper& operator=( const ConstCollectionWrapper& );
      ConstCollectionWrapper( const ConstCollectionWrapper& );

   private:
      const Collection&          _collection;
      std::vector<index_type>    _indexes;
   };
}
}

#endif
