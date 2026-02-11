/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __POINTERS_HPP__
#define __POINTERS_HPP__

/// @brief Base pointer estructure 
/// @tparam __type__ the pointer holding type
/// @tparam __tag__ Memory tag definition
template< typename __type__, memTag_t __tag__ >
class crPointer
{
public:
    typedef __type__*       pointer;
    typedef const __type__* const_pointer; 
    typedef __type__&       reference;
    typedef const __type__& const_reference;
    
    crPointer( void ) : m_data( nullptr ), m_size( 0 )
    {
    }

    virtual ~crPointer( void )
    {
    }

    /// @brief Fill the pointer with zero values.
    ID_INLINE void              SetZero( void )
    {
        std::memset( m_data, 0x00, m_size );
    }

    /// @brief Fills the pointer with values ​​defined by
    /// @param in_val the value to be set in the pointer.
    ID_INLINE void              SetValue( const int in_val )
    {
        std::memset( m_data, in_val, m_size );
    }

    /// @brief Perfor a memcpy to the pointer 
    /// @param in_src the source data buffer
    /// @param in_offset the offset in the destination pointer ( in bytes )
    /// @param in_bytes the bytes count to copy
    /// @return 
    ID_INLINE void              Memcpy( const void *in_src, const size_t in_offset, const size_t in_bytes )
    {
        /// check for overflow
        assert( ( in_offset + in_bytes ) < m_size );
        std::memcpy( reinterpret_cast<byte*>( m_data ) + in_offset, in_src, in_bytes );
    }

    ID_INLINE int               Memcmp( const void *in_src, const size_t in_offset, const size_t in_bytes )
    {
        /// check for overflow
        assert( ( in_offset + in_bytes ) < m_size );
        return std::memcmp( reinterpret_cast<byte*>( m_data ) + in_offset, in_src, in_bytes );
    }

    /// @brief The size of data allocated in the pointer.
    ID_INLINE const size_t      Size( void ) const { return m_size; }

    /// @brief Manual access to the pointer 
    ID_INLINE const_pointer     Data( void ) const { return m_data; }

    /// @brief Operator overloading -> to access members of the pointed object
    ID_INLINE pointer           operator->( void ) { return m_data; }

    /// @brief Operator overloading -> to access members of the pointed object.
    ID_INLINE const_pointer     operator->( void ) const { return m_data; }

    /// @brief Overloading the "*" operator to dereference the pointer.
    ID_INLINE reference         operator*( void ) { return *m_data; }

    /// @brief Overloading the "*" operator to dereference the pointer.
    ID_INLINE const_reference   operator*( void ) const { return *m_data; }

    /// @brief 
    /// @param i 
    /// @return 
    ID_INLINE reference         operator[]( const int &i ) { return m_data[i]; }

    /// @brief 
    /// @param i 
    /// @return 
    ID_INLINE const_reference   operator[]( const int &i ) const {  return m_data[i]; }
    
    /// @brief 
    /// @param  
    ID_INLINE operator __type__*( void ) const { return m_data; }

    /// @brief 
    /// @param  
    ID_INLINE operator bool( void ) const { return m_data != nullptr; }

protected:
    size_t      m_size;
    pointer     m_data;
};

/// @brief Base pointer estructure ( RAII patern )
/// @tparam __type__ the pointer holding type
/// @tparam __tag__ Memory tag definition
template< typename __type__, memTag_t __tag__ >
class crStaticPointer : public crPointer< __type__, __tag__ >
{
public:
    typedef __type__*       pointer;
    typedef const __type__* const_pointer; 
    typedef __type__&       reference;
    typedef const __type__& const_reference;

    crStaticPointer( void ) : crPointer<__type__, __tag__>()
    {
    }

    explicit crStaticPointer( const uint32_t in_count, const size_t in_alignament ) : crPointer<__type__, __tag__>()
    {
        this->m_data = static_cast<pointer>( Mem_Alloc16( sizeof( __type__), __tag__ ) );
        this->m_size = sizeof( this->m_data ); // TODO: align size first, and get it to allocation
    }

    ~crStaticPointer( void )
    {
        /// Release data at poiter exit
        if ( crPointer<__type__, __tag__>::m_data != nullptr )
        {
            Mem_Free16( this->m_data );
            this->m_data = nullptr;
        }

        this->m_size = 0;
    }

    ID_INLINE void Duplicate( const crStaticPointer &in_copySource )
    {
        this->m_size = in_copySource.m_size;
        this->m_data = static_cast<pointer>( Mem_Alloc16( this->m_size, __tag__ ) );
        std::memcpy( this->m_data, in_copySource.m_data, this->m_size );
    }

private:
    // Prevents copying to avoid two objects attempting to delete the same pointer.
    crStaticPointer( const crStaticPointer& ) = delete;
    crStaticPointer& operator = ( const crStaticPointer& ) = delete;
};

// BEATO HELP:
// GCC atomic
// __sync_fetch_and_add: atomically increase and return the old value ( old = x, x += n, old )
// __sync_fetch_and_sub: atomically decrement and return the old value ( old = x, x -= n, old )
// __sync_add_and_fetch atomically increase and return the new value ( x += n, x )
// __sync_sub_and_fetch atomically decrements and return the new value ( x -= n, x )

// STD C11 atomic
// atomic_fetch_add: Atomically replaces the value pointed by obj with the result of addition of arg to the old value of obj
// atomic_fetch_sub: Atomically replaces the value pointed by obj with the result of subtraction of arg from the old value of obj

/// @brief Reference countin pointer
/// @tparam __type__ the pointer holding type
/// @tparam __tag__ Memory tag definition
template< typename __type__, memTag_t __tag__ >
class crAutoPointer : public crPointer< __type__, __tag__ >
{
public:
    typedef __type__*       pointer;
    typedef const __type__* const_pointer; 
    typedef __type__&       reference;
    typedef const __type__& const_reference;

    crAutoPointer( void ) : crPointer< __type__, __tag__ >()
    {
    }

    crAutoPointer( const crAutoPointer< __type__, __tag__> &in_ref )
    {
        // if the pointer are in use, release the old reference
        if ( this->m_data != nullptr )
        {
            // decrease reference
		    if ( DecRefCount( this->m_data ) < 1 )
			    Delete( *this );
        }

        // copy pointer endress
	    this->m_data = in_ref.m_data;

        // incrase pointer reference
	    if( this->m_data != nullptr )
		    IncRefCount( this->m_data );
    }

    ~crAutoPointer( void )
    {
        // if is the last pointer reference delete the object
	    if( this->m_data != nullptr )
	    {
	    	if ( DecRefCount( this->m_data ) < 1 )
	    		Delete( *this );
	    }
    }

    /// @brief Allocate a new data array  
    /// @param in_count number of the elements
    /// @return 
    static crAutoPointer<__type__, __tag__> Malloc( const uint32_t in_count )
    {
        // new reference pointer holder 
        crAutoPointer<__type__, __tag__> newptr = crAutoPointer<__type__, __tag__>();
        
        // Allocate the atomic counter at pointer begining 
	    void* ptr = Mem_Alloc16( ( sizeof(__type__) * in_count )+ sizeof( uint32_t ), __tag__ );
        assert(ptr != nullptr); //

        // set counter to zero
        *static_cast<uint32_t*>( ptr ) = 0;
        
	    // hide atomic counter
	    newptr.m_data = reinterpret_cast<pointer>( reinterpret_cast<uintptr_t>( ptr ) + sizeof(uint32_t) );
    
        return newptr;
    }

    template<typename... Args>
    static crAutoPointer<__type__, __tag__> New( Args&&... args )
    {
        // new reference pointer holder 
        crAutoPointer<__type__, __tag__> newptr = crAutoPointer<__type__, __tag__>();
        
        // Allocate the atomic counter at pointer begining 
	    void* ptr = Mem_Alloc16( sizeof(__type__) + sizeof( uint32_t ), __tag__ );
        assert( ptr != nullptr ); //

        // set counter to zero
        *static_cast<uint32_t*>( ptr ) = 0;
        
	    // hide atomic counter
	    newptr.m_data = reinterpret_cast<pointer>( reinterpret_cast<uintptr_t>( ptr ) + sizeof(uint32_t) );
    
        // construct the class 
        new( newptr.m_data ) __type__( std::forward<Args>(args)... ); 

        return newptr;
    }

    /// @brief Delete object pointer
    /// @param ref reference to the pointer
    /// @return 
    ID_INLINE static void                   Delete(  crAutoPointer<__type__, __tag__> &in_ref )
    {
        if ( in_ref.m_data != nullptr )
        {
            /// cast the destructor
            reinterpret_cast<pointer>( in_ref.m_data )->~__type__();

            // get whole pointer
            void* ptr = reinterpret_cast<void*>( reinterpret_cast<uintptr_t>( in_ref.m_data ) + sizeof( uint32_t ) );

            // release memory 
            Mem_Free16( ptr );
            in_ref.m_data = nullptr;
        }   
    }

private:
    uint32_t IncRefCount( pointer in_ptr )
    {
	    assert( in_ptr != nullptr );
	    uint32_t* val = reinterpret_cast<uint32_t*>( reinterpret_cast<uintptr_t>( in_ptr ) - sizeof( uint32_t ) );
#ifdef USE_GCC_ATTOMIC
		return atomic_fetch_add( val, 1 ) + 1;
#else
		return __sync_fetch_and_add( val , 1 ) + 1;
#endif
    }

	uint32_t DecRefCount( pointer in_ptr )
    {
        assert( in_ptr != nullptr );
        uint32_t* val = reinterpret_cast<uint32_t*>( reinterpret_cast<uintptr_t>( in_ptr ) - sizeof( uint32_t ) );
#ifdef USE_GCC_ATTOMIC
        return atomic_fetch_sub( val, 1 ) - 1;
#else
		return __sync_sub_and_fetch( val, 1 ) - 1;
#endif
    }
};

#endif //!__POINTERS_HPP__