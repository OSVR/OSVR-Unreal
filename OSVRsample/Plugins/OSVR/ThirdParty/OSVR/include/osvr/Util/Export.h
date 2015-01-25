
#ifndef OSVR_UTIL_EXPORT_H
#define OSVR_UTIL_EXPORT_H

#ifdef OSVR_UTIL_STATIC_DEFINE
#  define OSVR_UTIL_EXPORT
#  define OSVR_UTIL_NO_EXPORT
#else
#  ifndef OSVR_UTIL_EXPORT
#    ifdef osvrUtil_EXPORTS
        /* We are building this library */
#      define OSVR_UTIL_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define OSVR_UTIL_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef OSVR_UTIL_NO_EXPORT
#    define OSVR_UTIL_NO_EXPORT 
#  endif
#endif

#ifndef OSVR_UTIL_DEPRECATED
#  define OSVR_UTIL_DEPRECATED __declspec(deprecated)
#endif

#ifndef OSVR_UTIL_DEPRECATED_EXPORT
#  define OSVR_UTIL_DEPRECATED_EXPORT OSVR_UTIL_EXPORT OSVR_UTIL_DEPRECATED
#endif

#ifndef OSVR_UTIL_DEPRECATED_NO_EXPORT
#  define OSVR_UTIL_DEPRECATED_NO_EXPORT OSVR_UTIL_NO_EXPORT OSVR_UTIL_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define OSVR_UTIL_NO_DEPRECATED
#endif

#endif
