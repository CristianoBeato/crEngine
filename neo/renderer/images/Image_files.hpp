
#ifndef __IMAGE_FILES_HPP__
#define __IMAGE_FILES_HPP__

// data is RGBA
extern void	R_WriteTGA( const char* filename, const byte* data, int width, int height, bool flipVertical = false, const char* basePath = "fs_savepath" );
extern void	R_WritePNG( const char* filename, const byte* data, int width, int height, bool flipVertical = false, const char* basePath = "fs_savepath" );
extern void R_LoadImage( const char* cname, byte** pic, int* width, int* height, ID_TIME_T* timestamp, bool makePowerOf2 );
// data is in top-to-bottom raster order unless flipVertical is set

#endif //!__IMAGE_FILES_HPP__