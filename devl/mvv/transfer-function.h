#ifndef MVV_TRANSFER_FUNCTION_H_
# define MVV_TRANSFER_FUNCTION_H_

namespace mvv
{
   /**
    @ingroup mvv
    @brief Transform a real value to a RGB value
    */
   class TransferFunction
   {
   public:
      virtual ~TransferFunction()
      {}

      /**
       @brief Transform a real value to a RGB value
       @param inValue the input value
       @param outValue must be allocated (4 * ui8)
       */
      virtual const ui8* transform( float inValue ) = 0;
   };
}

#endif
