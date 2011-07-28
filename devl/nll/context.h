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

#ifndef NLL_CORE_CONTEXT_H_
# define NLL_CORE_CONTEXT_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief a Tag to specify the derived type is a context
    */
   class ContextInstance
   {
   public:
      virtual ~ContextInstance()
      {}
   };

   /**
    @ingroup core
    @brief Hold a set of <code>ContextInstance</code>.
    @note ContextInstance must be allocated and it will automatically be deallocated when the context instance is destroyed
    */
   class Context
   {
      typedef std::map<std::string, ContextInstance*>   ContextContainer;

   public:
      /**
       @brief deallocates all the resources
       */
      ~Context()
      {
         clear();
      }

      /**
       @brief Stores a context. If one already existed, it is destroyed.
       @param context the context, which must have been allocated. It will be deallocated
              when the context is destroyed.
       */
      template <class T> void add( T* context )
      {
         ensure( context, "must not be empty" );
         const std::string s = typeid( T ).name();
         ContextContainer::iterator it = _contexts.find( s );
         if ( it != _contexts.end() )
            delete it->second;
         _contexts[ s ] = context;
      }

      /**
       @brief returns the specified context
       */
      template <class T> T* get() const
      {
         const std::string s = typeid( T ).name();
         ContextContainer::const_iterator it = _contexts.find( s );
         if ( it == _contexts.end() )
            return 0;
         return dynamic_cast<T*>( it->second );
      }

      /**
       @brief returns the specified context
       */
      template <class T> void get( T*& out ) const
      {
         out = get<T>();
      }

      /**
       @brief clear the context
       */
      void clear()
      {
         for ( ContextContainer::iterator it = _contexts.begin(); it != _contexts.end(); ++it )
         {
            delete it->second;
         }
         _contexts.clear();
      }

   private:
      ContextContainer     _contexts;
   };
}
}

#endif