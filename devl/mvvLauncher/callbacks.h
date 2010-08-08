#ifndef MVV_LAUNCHER_CALLBACKS_H_
# define MVV_LAUNCHER_CALLBACKS_H_

class Callbacks
{
public:
   Callbacks( CompilerFrontEnd& compiler ) : _compiler( compiler ), _oldCallbacks( 0 )
   {}

   void run()
   {
      const RuntimeValue* callbacksPtr = 0;
      RuntimeValues* current = 0;

      try
      {
         callbacksPtr = &_compiler.getVariable( mvv::Symbol::create( "callbacks" ) );
         current = const_cast<RuntimeValues*>( callbacksPtr->vals.getDataPtr() );
      }
      catch(...)
      {
         // can't find a callback
         return;
      }

      ensure( callbacksPtr && current, "must not be null!" );
      const RuntimeValue& callbacks = *callbacksPtr;

      if ( _oldCallbacks != current )
      {
         _oldCallbacks = current;
         _callbacks.clear();

         if ( callbacks.type != RuntimeValue::TYPE )
            throw std::exception( "error: 'callbacks' must be a type" );
         if ( (*callbacks.vals).size() != 2 )
            throw std::exception( "error: 'callbacks' must be defined as 2 arrays of function callbacks and strings" );
         if ( (*callbacks.vals)[ 0 ].type != RuntimeValue::TYPE )
            throw std::exception( "error: 'callbacks' must contains 2 types only" );

         ui32 size = (ui32)(*(*callbacks.vals)[ 0 ].vals).size();
         if ( (*(*callbacks.vals)[ 1 ].vals).size() != size )
            throw std::exception( "error: 'callbacks' must contain the same number of key & function/member pointers" );
         RuntimeValues& keys = const_cast<RuntimeValues&>( ( *(*callbacks.vals)[ 1 ].vals ) );

         // build the list of keys, call back to execute
         for ( ui32 n = 0; n < size; ++n )
         {
            if ( keys[ n ].type != RuntimeValue::STRING )
               throw std::exception( "error: 'callbacks' keys must only be of string type" );
            if ( keys[ n ].stringval.size() == 1 )
            {
               // simple key
               int key = keys[ n ].stringval[ 0 ];
               _callbacks[ key ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F1" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F1 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F2" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F2 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F3" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F3 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F4" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F4 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F5" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F5 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F6" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F6 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F7" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F7 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F8" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F8 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F9" )
            {
               _callbacks[ mvv::platform::EventKeyboard::KEY_F9 ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
         }
      }
   }

   bool handleKey( int key )
   {
      std::map<int, std::string>::iterator it = _callbacks.find( key );
      if ( it != _callbacks.end() )
      {
         try
         {
            _compiler.run( it->second );
         } catch (...)
         {
            _compiler.getStdOut() << "error: unable to launch the callback";
         }
         return true;
      }
      return false;
   }

private:
   CompilerFrontEnd&             _compiler;
   RuntimeValues*                _oldCallbacks;
   std::map<int, std::string>    _callbacks;
};

#endif