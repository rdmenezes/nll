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

#ifndef NLL_FEATURE_SELECTION_H_
# define NLL_FEATURE_SELECTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a base to factorize the work of a feature selection task

    Point needs to provide: size(), operator[], constructor(size)
    Only handle the transformation of the database according to the feature descriptor 
   */
   template <class Point>
   class FeatureSelectionBase : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>   Base;

      // don't override these
      using Base::read;
      using Base::write;
      using Base::process;

      virtual ~FeatureSelectionBase(){}

      FeatureSelectionBase(){}

      FeatureSelectionBase( const core::Buffer1D<bool>& selectedFeatures ) : _selectedFeatures( selectedFeatures )
      {
         _nbSelectedFeatures = getNumberOfSelectedFeatures();
      }

   public:
      void read( std::istream& f )
      {
         core::read<size_t>( _nbSelectedFeatures, f );
         this->_selectedFeatures.read( f );
      }

      void write( std::ostream& f ) const
      {
         core::write<size_t>( _nbSelectedFeatures, f );
         this->_selectedFeatures.write( f );
      }

      /**
        @brief transform a Point with full feature to a new one with only selected features
        */
      Point process( const Point& point ) const
      {
         assert( point.size() == _selectedFeatures.size() );   // error : feature set doesn't match
         assert( _nbSelectedFeatures );                        // no selected feature
         Point newPoint( _nbSelectedFeatures );
         size_t n = 0;
         size_t i = 0;
         while ( n != _nbSelectedFeatures )
         {
            assert( n < point.size() );
            if ( _selectedFeatures[ i ] )
            {
               newPoint[ n ] = point[ i ];
               ++n;
            }
            ++i;
         }
         return newPoint;
      }

      /**
       @return the number of features that are currently selected
       */
      size_t getNumberOfSelectedFeatures()
      {
         size_t n = 0;
         for ( size_t i = 0; i < _selectedFeatures.size(); ++i )
            n += ( _selectedFeatures[ i ] == true );
         return n;
      }

      /**
       @brief Returns the curently selected features
       */
      const core::Buffer1D<bool>& getSelectedFeatures() const
      {
         return _selectedFeatures;
      }

   protected:
      /**
       @brief Set the features to be selected. Must be called by the derived class.
       */
      void _setSelectedFeatures( core::Buffer1D<bool>& f )
      {
         _selectedFeatures = f;
         _nbSelectedFeatures = getNumberOfSelectedFeatures();
      }

   private:
      core::Buffer1D<bool> _selectedFeatures;
      size_t                 _nbSelectedFeatures;
   };

   /**
    @ingroup algorithm
    @brief Actual base class for feature selection using a wrapper approach

    The Classifier is directly used to mark the feature set, this can be real classifier, or regression
    */
   template <class Point, class ClassifierBase = ClassifierBase<Point, size_t> >
   class FeatureSelectionWrapper : public FeatureSelectionBase<Point>
   {
   public:
      typedef FeatureSelectionBase<Point>       Base;
      typedef ClassifierBase                    Classifier;
      typedef typename ClassifierBase::Database Database;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionWrapper()
      {
         enum {RESULT = core::Equal<Point, typename ClassifierBase::Sample::Input>::value };
         STATIC_ASSERT( RESULT ); // the input & database must match!
      }
      virtual ~FeatureSelectionWrapper(){}

      FeatureSelectionWrapper( const core::Buffer1D<bool>& selectedFeatures ) : Base( selectedFeatures )
      {}

      // parameters : parameters of the classifier
      const core::Buffer1D<bool>& compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat )
      {
         core::Buffer1D<bool> f = _compute( classifier, parameters, dat );
         Base::_setSelectedFeatures( f );
         return Base::getSelectedFeatures();
      }

   protected:
      // parameters : parameters of the classifier
      // compute the set of selected features
      virtual core::Buffer1D<bool> _compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat ) = 0;
   };

   /**
    @ingroup algorithm
    @brief Actual base class for feature selection using a filter approach

    The class information & input only are used to select the feature subset
    */
   template <class Point>
   class FeatureSelectionFilter : public FeatureSelectionBase<Point>
   {
   public:
      typedef FeatureSelectionBase<Point>                      Base;
      typedef typename Classifier<Point>::Database             Database;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionFilter(){}
      virtual ~FeatureSelectionFilter(){}

      FeatureSelectionFilter( const core::Buffer1D<bool>& selectedFeatures ) : Base( selectedFeatures )
      {}

      const core::Buffer1D<bool>& compute( const Database& dat )
      {
         core::Buffer1D<bool> f = _compute( dat );
         Base::_setSelectedFeatures( f );
         return Base::getSelectedFeatures();
      }

   protected:
      // compute the set of selected features
      virtual core::Buffer1D<bool> _compute( const Database& dat ) = 0;
   };
}
}

#endif
