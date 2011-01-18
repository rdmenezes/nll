#ifndef NLL_CORE_CONVERTER_H_
# define NLL_CORE_CONVERTER_H_

//
// The purpose is to provide flexible automatic structure conversion
//
namespace nll
{
namespace core
{
   namespace impl
   {
      // 3 paramemeters: input, output, if input == output
      template <class Input, class Output, int EQUAL>
      struct Converter
      {
         static void convert( const Input& input, Output& output )
         {
            throw std::runtime_error( "conversion is not handled" );
         }
      };

      template <class Input>
      struct Converter<Input, Input, 1>
      {
         static void convert( const Input& input, Input& output )
         {
            output = input;
         }
      };

      template <class Input, class X>
      struct Converter< Input, std::vector<X>, 0 >
      {
         static void convert( const Input& input, std::vector<X>& output )
         {
            output = std::vector<X>( input.size() );
            for ( ui32 n = 0; n < input.size(); ++n )
               output[ n ] = static_cast<X>( input[ n ] );
         }
      };

      template <class Input, class X>
      struct Converter< Input, Buffer1D<X>, 0 >
      {
         static void convert( const Input& input, Buffer1D<X>& output )
         {
            output = Buffer1D<X>( static_cast<ui32>( input.size() ), false );
            for ( ui32 n = 0; n < input.size(); ++n )
               output[ n ] = static_cast<X>( input[ n ] );
         }
      };
   }

   template <class Input, class X>
   void convert( const Input& i, std::vector<X>& o )
   {
      impl::Converter<Input, std::vector<X>, Equal< Input, std::vector<X> >::value >::convert( i, o );
   }

   template <class Input, class X>
   void convert( const Input& i, Buffer1D<X>& o )
   {
      impl::Converter<Input, Buffer1D<X>, Equal< Input, Buffer1D<X> >::value >::convert( i, o );
   }
}
}

#endif