
#ifndef __LINUX_PATH_HPP__
#define __LINUX_PATH_HPP__

class crLinuxPaths : public crPaths
{
public:
    virtual const char*	CWD( void ) override;
    virtual const char* EXEPath( void ) override;
	//virtual const char* DefaultBasePath( void ) override;
	virtual const char*	DefaultSavePath( void ) override;
    virtual bool		IsFileWritable( const char* path ) override;

private:
};

#endif //!__LINUX_PATH_HPP__