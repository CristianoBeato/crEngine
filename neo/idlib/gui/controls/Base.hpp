
#ifndef __CONTROLS_BASE_HPP__
#define __CONTROLS_BASE_HPP__

namespace gui
{
    namespace controls
    {
        class crBase
        {
        public:
            typedef crAutoPointer<crBase, TAG_IDLIB>    crBasePointer;
            
            crBase( void );
            crBase( const crBasePointer &in_parent, const idStr in_name );
            ~crBase( void );

            virtual const char* GetTypeName( void ) { return "crBase"; }


            virtual void DoThink( void ) {}

            /// @brief 
            void            RemoveAllChildren( void );

            /// @brief 
            /// @param in_name 
            void            SetName( const idStr &in_name ) { m_name = in_name; }
            
            /// @brief 
            /// @param in_parent 
            void            SetParent( const crBasePointer &in_parent ) { m_parent = in_parent; };

            /// @brief 
            /// @param in_dock 
            void            SetDock( const uint32_t in_dock ) { m_docking = in_dock; }

            /// @brief Hide show the control 
            /// @param in_hide true to hide control  
            void            SetHidden( const bool in_hide ) { m_hidden = in_hide; }

            /// @brief Get the control name
            /// @return 
            const idStr     Name( void ) const { return m_name; } 
            
            /// @brief Get The control parent object 
            /// @return the parent object pointer or null if base canvas
            const crBase*   GetParent( void  ) const { return m_parent; } 

            /// @brief 
            /// @return 
            const uint32_t  GetDock( void ) const { return m_docking; }
            
            /// @brief 
            /// @return 
            const bool      IsDisabled( void ) const { return m_disabled; }

            /// @brief  
            /// @return 
            const bool      IsHidden( void ) const { return m_hidden; }

            /// @brief  
            /// @return 
            const bool      IsMouseInputEnabled( void ) const {return m_hidden; }

            /// @brief  
            /// @return 
            const bool      IsKeyboardInputEnabled( void ) const { return m_keyboardInputEnabled; }

        private:
            bool                    m_disabled;
            bool                    m_hidden;
            bool                    m_mouseInputEnabled;
            bool                    m_keyboardInputEnabled;
            uint32_t                m_docking;
            Rect_t                  m_bounds;
            Rect_t                  m_renderRect;
            Rect_t                  m_clipRect;
            idStr                   m_name;
            crBasePointer           m_parent;
            idList<crBasePointer>   m_children;
        };
    };
};

#endif //!__CONTROLS_BASE_HPP__