#ifndef __C_AUTOM8_EXPORT_HPP__
#define __C_AUTOM8_EXPORT_HPP__

#ifdef WIN32
    #ifdef AUTOM8_EXPORT
        #define dll_export __declspec(dllexport)
    #else
        #define dll_export __declspec(dllimport)
    #endif
#else
    #define dll_export
#endif

#endif