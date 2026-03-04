
#ifndef __VK_RESOURCE_HPP__
#define __VK_RESOURCE_HPP__

class vkResourceState
{
public:
    enum state_t : uint8_t
    {
        RESOURCE_STATE_UNKNOW,
        RESOURCE_STATE_COPY_DESTINATION,  // resource is a destination of a copy operation 
        RESOURCE_STATE_COPY_SOURCE,       // resource is a source from a copy operation
        RESOURCE_STATE_USE_RENDER,        // resource is used in a render operation
        RESOURCE_STATE_USE_COMPUTE,       // resource is used in a compute operation
        RESOURCE_STATE_WRITE_COMPUTE,     // resource is a compute shader destination
        RESOURCE_STATE_WRITE_RENDER       // resource is a render targer
    };

    vkResourceState( void ): m_state( RESOURCE_STATE_UNKNOW )
    {
    }

    /// @brief Vulkan state transition 
    /// @param in_state 
    virtual void    StateTransition( const state_t in_state ) = 0;
    
    state_t         State( void ) const { return m_state; }

protected:
    state_t m_state;    // resource transition state
};

#endif //!__VK_RESOURCE_HPP__