
#ifndef __LINUX_CPU_INFO_HPP__
#define __LINUX_CPU_INFO_HPP__

class crLinuxCPUInfo : public crCPUInfo
{
public:
    crLinuxCPUInfo( void );
    ~crLinuxCPUInfo( void );

    virtual void	Init( void );

protected:
    void    GetProcessorName( void );

private:

};

#endif //!__LINUX_CPU_INFO_HPP__