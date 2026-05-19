
#ifndef __UNIFORM_MANAGER_HPP__
#define __UNIFORM_MANAGER_HPP__

inline constexpr uint32_t MAX_BINDING_SAMPLERS = 8192;
inline constexpr uint32_t MAX_BINDINGS = 4;
inline constexpr uint32_t BINDING_SAMP = 0; // set 0 binding 0: combined sampler array binding
inline constexpr uint32_t BINDING_MESH = 0; // set 1 binding 0: SSBO to mesh related  data
inline constexpr uint32_t BINDING_MATE = 1; // set 1 binding 1: SSBO to material related data
inline constexpr uint32_t BINDING_LIGH = 2; // set 1 binding 2: SSBO to light related data
inline constexpr uint32_t BINDING_JOIN = 3; // set 1 binding 3: SSBO to joint array 

///
/// Texture Sampler binding slot 
class vkSamplerSlot
{
public:
	vkSamplerSlot( void );
	vkSamplerSlot( const VkImageView in_imageView, const VkSampler in_sampler, const VkImageLayout in_imageLayout );
    ~vkSamplerSlot( void );
    
    void    SetIndex( const uint32_t in_index ) { m_index = in_index; }
    uint32_t GetIndex( void ) const { return m_index; }
    VkDescriptorImageInfo GetHandle( void ) const { return m_descriptorImageInfo; };

    ///
    operator uint32_t( void ) const { return m_index; }  

private:
    uint32_t 				m_index;				// logic index
	VkDescriptorImageInfo   m_descriptorImageInfo; 
};

struct alignas( 16 ) uMesh_t
{
	idVec4	viewPoint;
	float	modelMatrix[16];
	float	viewMatrix[16];
	float	MVPMatrix[16];
	float	textureMatrix[16];
};

struct alignas( 16 ) uMaterial_t
{
	idVec4 color;			// color 
	idVec4 colorAdd;		// color var
	idVec4 colorModulate;	//
	idVec4 alphaTest;		// x enable test, y test value 
};

struct alignas( 16 ) uLight_t
{
	idVec4 color;
	idVec4 position;
};

inline constexpr size_t k_MESH_UNIFORM_SIZE = sizeof( uMesh_t );
inline constexpr size_t k_MATERIAL_UNIFORM_SIZE = sizeof( uMaterial_t );
inline constexpr size_t k_LIGTH_UNIFORM_SIZE = sizeof( uLight_t );
inline constexpr size_t k_JOINT_UNIFORM_SIZE = sizeof( idJointMat );

class crUniformManager
{
public:
	static crUniformManager* Get( void );
    crUniformManager( void );
    ~crUniformManager( void );
	void				StartUp( void );
	void				ShutDown( void );
	void				SetFrame( const uint32_t in_frameID, const crCommandBufferp in_commandBuffer );
	void				SubmitOffsets( const crCommandBufferp in_commandBuffer );
	const uint32_t		CurrentFrame( void ) const { return m_frameID; }
	VkPipelineLayout	Layout( void ) const { return m_layout; }
	uMesh_t*		GetMeshUniforms( void );
	uMaterial_t*	GetMaterialUniforms( void );
	uLight_t*		GetLightUniforms( void );

	joint_cache_t	AllocJoints( const uint32_t in_count, const idJointMat* in_joints );

private:
	uint32_t									m_frameID;
	VkPipelineLayout							m_layout;
	VkDescriptorSetLayout						m_bindlessSetLayout;
	VkDescriptorSetLayout						m_storageSetLayout;
	VkDescriptorPool							m_bindlessPool;
	VkDescriptorPool							m_storagePool;
	VkDescriptorSet								m_bindlessSet;
	crMemoryPool*								m_buffersMemPool;
	idArray<VkDescriptorSet, SMP_FRAMES>		m_storageSet;
	idArray<uint32_t, MAX_BINDINGS>				m_dynamicOffsets;
	idArray<crBuffer*, MAX_BINDINGS>			m_shaderStorageBuffers;
	idArray<crMemoryPage*, MAX_BINDINGS>		m_buffersMemPages;
	idList<VkDescriptorImageInfo>				m_combinedSamplersLocations;

	void	CreateStorageBuffers( void );
	void	CreateStorageSet( void );
	void	CreateBindlessSet( void );
};

#endif //!__UNIFORM_MANAGER_HPP__