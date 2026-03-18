
# Precache on map load

Precache material configurations and set bindless textures and samples locations on map load,
then wen render, we just point the index to element ( just like vertex arrays or index arrays )

# Frontend "snapshot" 

Mesh properties, like orientation matrix, view positions are stored in a frontend buffer, then the backend copy the buffer to a
shader storage render buffer, like a "snapshot" of the current meshes properties, so we can just have everithing o a GPU memory wen gona use


