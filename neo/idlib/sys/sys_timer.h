
#ifndef __TIMER_H__
#define __TIMER_H__

// a decent minimum sleep time to avoid going below the process scheduler speeds
inline constexpr uint32_t SYS_MINSLEEP = 20u;

// allow game to yield CPU time
// NOTE: due to SYS_MINSLEEP this is very bad portability karma, and should be completely removed
extern void			Sys_Sleep( const uint32_t in_msec );

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
extern uint32_t     Sys_Milliseconds( void );
extern uint64_t     Sys_Microseconds( void );

#endif //!__TIMER_H__