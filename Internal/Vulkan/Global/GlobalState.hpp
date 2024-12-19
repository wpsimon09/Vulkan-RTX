//
// Created by wpsimon09 on 22/09/24.
//

#ifndef GLOBALSTATE_HPP
#define GLOBALSTATE_HPP

namespace GlobalState
{
    inline bool ValidationLayersEnabled = true;
    inline bool LoggingEnabled = true;
    inline bool Verbose = false;
    inline bool VerboseInRendering = false;
    inline bool AutoCommandBufferFlags = true;

    inline void EnableLogging()
    {
        LoggingEnabled = true;
    }

    inline void DisableLogging()
    {
        LoggingEnabled = false;
    }
}

#endif //GLOBALSTATE_HPP
