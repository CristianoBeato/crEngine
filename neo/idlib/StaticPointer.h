
#ifndef __STATIC_POINTER_H__
#define __STATIC_POINTER_H__

/// 
/// crStaticPointer 
/// @brief a Smart pointer object 
template< typename _t >
class crStaticPointer
{
public:
    typedef _t value_type;
	typedef _t &reference;
	typedef const _t &const_reference;
	typedef _t *pointer;
	typedef const _t *const_pointer;
	typedef const _t*& pointer_reference;
	typedef const _t*& const_pointer_reference;

    crStaticPointer( void ) : m_data( nullptr )
    {
    }

    crStaticPointer( pointer in_ptr ) : m_data(  in_ptr  )
    {
    }

    crStaticPointer( pointer_reference in_ptr ) : m_data(  in_ptr  )
    {
    }

    ~crStaticPointer( void )
    {
        if ( m_data != nullptr )
        {
            Mem_Free( m_data );
            m_data = nullptr;
        }
    }

    	/// @brief Copy a memory source to the content 
	/// @param src memory source 
	/// @param offset position in the content 
	/// @param len size of the source
	ID_INLINE void  Memcpy( const void *in_src, const uintptr_t in_offset, const size_t in_size ) const
    {
        std::memcpy( reinterpret_cast<byte*>( m_data ) + in_offset, in_src, in_size );
    }
	
	/// @brief Compare the memory source to the content 
	/// @param src source memory 
	/// @param offset offset in the content of pointer 
	/// @param len size of the memory to compare 
	/// @return true on same 
	ID_INLINE bool  Memcmp( const void *in_src, const uintptr_t in_offset, const size_t in_size ) const
    {
        return std::memcmp( reinterpret_cast<byte*>( m_data ) + in_offset, in_src, in_size ) == 0;
    }
	
	/// @brief Set content to a value 
	/// @param c value to set
	/// @param offset offset of the buffer 
	/// @param len buffer legent 
	ID_INLINE void  Memset( const int in_val, const uintptr_t in_offset, const size_t in_size )
    {
        std::memset( reinterpret_cast<byte*>( m_data ) + in_offset, in_val, in_size );
    }
	
	/// @brief Aces to the pointer object 
	/// @param  
	/// @return 
	ID_INLINE   pointer	GetPtr( void ) const { return  m_data; }

	/// @brief Content alloced size 
	/// @param  
	/// @return pointer size
	ID_INLINE size_t			Size( void ) const
    {
        return sizeof( m_data );
    }

    ID_INLINE bool operator == ( const_pointer_reference in_ref ) const
    {
        return in_ref == m_data;
    }

	ID_INLINE bool operator != ( const_pointer_reference in_ref ) const
    {
        return in_ref != m_data;
    }


    ID_INLINE bool operator == ( const crStaticPointer<_t> & in_ref ) const
    {
        return in_ref.m_data == m_data;
    }

	ID_INLINE bool operator != ( const crStaticPointer<_t> & in_ref ) const
    {
        return in_ref.m_data != m_data;
    }

	ID_INLINE pointer operator -> ( void )
    {
        return m_data;
    }

	ID_INLINE const_pointer		operator -> ( void ) const
    {
        return m_data;
    }

	ID_INLINE pointer operator & ( void ) 
    { 
        return m_data;
    }
	
    ID_INLINE const_pointer operator & ( void ) const
    {
        return m_data;
    }
	
    ID_INLINE reference operator *(void)
    {
        return *m_data;
    }

	ID_INLINE const_reference		operator *( void ) const
    {
        return *m_data;
    }

	ID_INLINE reference operator[]( int in_index )
    {
        return m_data[in_index];
    }

	ID_INLINE const_reference operator[]( int in_index ) const
    {
        return m_data[in_index];
    }

	ID_INLINE explicit operator bool( void ) const noexcept
    {
        return m_data != nullptr;
    }

private:
    pointer m_data;
};

#endif //__STATIC_POINTER_H__