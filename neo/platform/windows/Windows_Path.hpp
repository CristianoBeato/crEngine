
#ifndef __WINDOWS_PATH_HPP__
#define __WINDOWS_PATH_HPP__

class crWindowsPaths : public crPaths
{
public:
    virtual const char* EXEPath( void ) override;
    virtual const char*	CWD( void ) override;
    virtual const char* DefaultBasePath( void ) override;
    virtual const char*	DefaultSavePath( void ) override;
    virtual bool		IsFileWritable( const char* path ) override;
};

#endif //!__WINDOWS_PATH_HPP__