#ifndef MVV_LAUNCHER_CALLBACKS_H_
# define MVV_LAUNCHER_CALLBACKS_H_

class Callbacks
{
public:
   struct Key
   {
      enum Modifier
      {
         NORMAL,
         CTRL,
         SHIFT,
         ALT
      };

      Key( int c, Modifier m ) : key( c ), modifier( m )
      {}

      int      key;
      Modifier modifier;

      bool operator<( const Key& c ) const
      {
         const ui32 indexs = key + modifier * 1024;
         const ui32 indexr = c.key + c.modifier * 1024;
         return indexs < indexr;
      }
   };

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
         if ( (*callbacks.vals).size() != 4 )
            throw std::exception( "error: 'callbacks' must be defined as 3 arrays and one int: function callbacks, key strings and associated key modifier" );
         if ( (*callbacks.vals)[ 0 ].vals.getDataPtr() == 0 )
            return;  // no callbacks!
         //if ( (*callbacks.vals)[ 0 ].type != RuntimeValue::TYPE || (*callbacks.vals)[ 0 ].type != RuntimeValue::EMPTY )
         //   throw std::exception( "error: function 'callbacks' must be a type" );

         ui32 size = (ui32)(*(*callbacks.vals)[ 0 ].vals).size();
         if ( (*(*callbacks.vals)[ 1 ].vals).size() != size )
            throw std::exception( "error: 'callbacks' must contain the same number of key & function/member pointers" );
         RuntimeValues& keys      = const_cast<RuntimeValues&>( ( *(*callbacks.vals)[ 1 ].vals ) );
         RuntimeValues& modifiers = const_cast<RuntimeValues&>( ( *(*callbacks.vals)[ 2 ].vals ) );

         // build the list of keys, call back to execute
         for ( ui32 n = 0; n < size; ++n )
         {
            if ( keys[ n ].type != RuntimeValue::STRING )
               throw std::exception( "error: 'callbacks' keys must only be of string type" );
            if ( keys[ n ].stringval.size() == 1 )
            {
               // simple key
               int key = keys[ n ].stringval[ 0 ];
               _callbacks[ Key( key, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "ESC" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_ESC, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F1" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F1, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F2" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F2, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F3" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F3, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F4" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F4, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F5" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F5, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F6" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F6, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F7" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F7, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F8" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F8, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
            if ( keys[ n ].stringval == "F9" )
            {
               _callbacks[ Key( mvv::platform::EventKeyboard::KEY_F9, (Key::Modifier)modifiers[ n ].intval ) ] = "callbacks.callbacks[" + nll::core::val2str( n ) + "]();";
               continue;
            }
         }
      }
   }

   bool handleKey( int key, Key::Modifier modifier )
   {
      std::map<Key, std::string>::iterator it = _callbacks.find( Key( key, modifier ) );
      if ( it != _callbacks.end() )
      {
         try
         {
            //std::ostream* saveout = &_compiler.getStdOut();
            std::stringstream ss;
            _compiler.setStdOut( &ss );
            _compiler.run( it->second );
            _compiler.setStdOut( &std::cout );

            // get the context
            platform::ContextGlobal* global = (*_compiler.getContextExtension()).get<platform::ContextGlobal>();
            if ( !global )
            {
               throw RuntimeException( "mvv global context has not been initialized" );
            }

            Pane* p = (*global->layout).find( mvv::Symbol::create( "mvv::platform::LayoutCommandLine" ) );
            LayoutCommandLine* cmd = dynamic_cast<LayoutCommandLine*>( p );
            if ( !cmd )
            {
               throw RuntimeException( "invalid class for mvv::platform::LayoutCommandLine ID");
            }

            while ( !ss.eof() )
            {
               std::string s;
               std::getline( ss, s );
               if ( s != "" )
                  cmd->sendMessage( s, nll::core::vector3uc( 255, 255, 255 ) );
            }
         } catch ( std::exception e )
         {
            _compiler.getStdOut() << "error: unable to launch the callback:" << _compiler.getLastErrorMesage() << std::endl;
            _compiler.getStdOut() << "exception=" << e.what() << std::endl;
         }
         return true;
      }
      return false;
   }

private:
   // disabled
   Callbacks( const Callbacks& );
   Callbacks& operator=( const Callbacks& );


private:
   CompilerFrontEnd&             _compiler;
   RuntimeValues*                _oldCallbacks;
   std::map<Key, std::string>    _callbacks;
};

#endif