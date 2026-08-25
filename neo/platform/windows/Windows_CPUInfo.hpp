
#ifndef __WINDOWS_CPU_INFO_HPP__
#define __WINDOWS_CPU_INFO_HPP__

class crWindowsCPUInfo : public crCPUInfo
{
public:
    crWindowsCPUInfo( void );
    ~crWindowsCPUInfo( void );

    virtual void	Init( void );

protected:
    void    GetProcessorName( void );

private:

};

#endif //!__WINDOWS_CPU_INFO_HPP__