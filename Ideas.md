# Current planed 
- PBR Rendering Pipeline.
- SDL3 Sound Egine ( OpenAL is crashing on my linux [Debian 13] )
- A Internal GUI Sistem for tools.
- Checkboard rendering for performance.
- ~~Fix vehicles (savegame crash fix; networking)~~
- 8-way texture blending using vertex colors with transition masking using height maps
- Refine LOD solution (make it a bit more performant; add support for skeletal meshes)
- Distance culling (not render meshes past certain distance) 
- Foliage shader with simple wind deformations (including using vertex color to anchor mesh and create gradient rigidity) 
- Mesh instancing for static meshes (kinda like ISM/HISM in UE4)
- AAS on meshes (to be able to have navigation built on mesh terrains)
- Save/load game multithreading (to eliminate hiccups/freezes when performing saving/loading in-game)
- Improve Lights Editor to allow spawning/deleting/moving/saving lights in-game
- DoomScript nativization (optional, to improve performance)
- Reverb zones (XAudio2 supports reverb afaik).
- Native multiplayer coop.
- Headless dedicated server.
- Improve shadowmapping (particularly cascaded shadowmaps on huge levels)
- Tesselation displacement on meshes.
- Parallax displacement on surfaces.
- Parallax-corrected cubemap reflections ( with support for equirectangle textures; or better yet reflection capture entities ).
- SSAO.
- Light shafts.
- Replace SWF renderer with a more suitable gui renderer.

# Current on the way
- Remove internal compressors, to implement custom pre compiled texture loaders ( Like VALVe Source VTF ), BTF
- VULKAN 1.3 PORT ( on the way )
    - Create "By Material" VkPipelines.
        The idea behind Vulkan pipelines is to create predefined rendering states in order to save rendering time, assuming that the drivers already have all the states and shaders defined when the command buffer is submitted. However, this creates greater complexity due to the constant changes in states caused by materials and rendering stages. To minimize these difficulties, implementing pipelines in the shaders through a manager that delivers pre-created pipelines or creates new ones according to the needs of new shaders, combined with the fact that idTech4 already loads all shaders at initialization, allows for this method, similar to that described in "https://gpuopen.com/learn/porting-detroit-1".

    - Create Pipeline Cache by device. ( DONE )
        With the implementation of new shaders, creating a very large combination of states becomes costly in terms of performance and application initialization. Therefore, it's necessary to implement methods to mitigate this difficulty, and the best way Vulkan offers this is through pipeline caching. This allows the creation of new pipelines to reuse pre-compiled bitcode from other pipelines, and this bitcode (specific to the driver and the driver version) can be serialized to disk, allowing it to be reloaded on subsequent initializations. It only needs to be generated on the first application initialization (or during driver updates).

    - SwapChain. ( DONE ).
        Unlike OpenGL, where the swapchain is managed by the driver, and we only manage it if we write to the back/front buffer, Vulkan requires that the swapchain be created and managed by the application.

    - CommandBuffers. ( DONE )
        Vulkan, in order to summarize the Driver Overhead, uses command buffer structures, where commands are registered and sent in batches to the driver and subsequently executed sequentially by the GPU (this also serves to facilitate multithread management, but for now it is not necessary).
        
    - Vertex/Index Buffers.
    - Transfer Buffers.
    - Texture Images.
    - Pixel Transfer.
    - Current Render Copy.

# May Be 
# Precache on map load
Precache material configurations and set bindless textures and samples locations on map load,
then wen render, we just point the index to element ( just like vertex arrays or index arrays )

# May Be
# Frontend "snapshot" 
Mesh properties, like orientation matrix, view positions are stored in a frontend buffer, then the backend copy the buffer to a
shader storage render buffer, like a "snapshot" of the current meshes properties, so we can just have everithing o a GPU memory wen gona use

# May Be
# Steam Audio
Made use of VALVe's opensource audio library SteamAudio to render spartial HRTF audio, for better 3d audio properties.

