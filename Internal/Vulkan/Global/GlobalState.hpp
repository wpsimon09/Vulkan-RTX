//
// Created by wpsimon09 on 22/09/24.
//

#ifndef GLOBALSTATE_HPP
#define GLOBALSTATE_HPP

#define MB = 1024 * 1024

namespace GlobalState {

inline bool ValidationLayersEnabled = true;
inline bool LoggingEnabled          = true;
inline bool Verbose                 = false;
inline bool VerboseInRendering      = false;
inline bool AutoCommandBufferFlags  = true;
inline bool MSAA                    = true;

inline void EnableLogging()
{
    LoggingEnabled = true;
}

inline void DisableLogging()
{
    LoggingEnabled = false;
}

inline int LogLimit = 700;  // entires in console

#ifdef NDEBUG
inline bool InDebugMode = false;
#else
inline bool InDebugMode = true;
#endif


}  // namespace GlobalState

#endif  //GLOBALSTATE_HPP
