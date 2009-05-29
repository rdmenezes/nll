#ifndef NLL_DEBUG_IO_H_
# define NLL_DEBUG_IO_H_

namespace nll
{
namespace debug
{
   /**
    @ingroup debug
    @brief write a 2D buffer to a file
    */
   template <class T>
   void writeDebug( const std::string& file, T* buf, ui32 sizex, ui32 sizey )
   {
      std::ofstream f( file.c_str() );
      for ( ui32 y = 0; y < sizey; ++y )
      {
         for ( ui32 x = 0; x < sizex; ++x )
         {
            core::write<ui8>( (ui8)( ( buf[ x + sizex * y ] % 10 ) + '0' ), f );
         }
         core::write<ui8>( '\n', f );
      }
      f.close();
   }

   /**
    @ingroup debug
    @brief write a 2D buffer, full value to a file
    */
   template <class T, class ViewedAsType>
   void writeDebugValue( const std::string& file, T* buf, ui32 sizex, ui32 sizey )
   {
      std::ofstream f( file.c_str() );
      for ( ui32 y = 0; y < sizey; ++y )
      {
         for ( ui32 x = 0; x < sizex; ++x )
         {
            f << static_cast<ViewedAsType>( buf[ x + sizex * y ] ) << " ";
         }
         f << std::endl;
      }
      f.close();
   }
}
}

#endif
