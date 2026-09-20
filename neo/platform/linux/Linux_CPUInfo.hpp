
#ifndef __LINUX_CPU_INFO_HPP__
#define __LINUX_CPU_INFO_HPP__

class crLinuxCPUInfo : public crCPUInfo
{
public:
    crLinuxCPUInfo( void );
    virtual ~crLinuxCPUInfo( void );

    virtual void	Init( void ) override;

protected:
    void    GetProcessorName( void );
};

#endif //!__LINUX_CPU_INFO_HPP__