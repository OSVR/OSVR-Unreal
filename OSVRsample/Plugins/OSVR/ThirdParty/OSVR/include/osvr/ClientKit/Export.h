
#ifndef OSVR_CLIENTKIT_EXPORT_H
#define OSVR_CLIENTKIT_EXPORT_H

#ifdef OSVR_CLIENTKIT_STATIC_DEFINE
#  define OSVR_CLIENTKIT_EXPORT
#  define OSVR_CLIENTKIT_NO_EXPORT
#else
#  ifndef OSVR_CLIENTKIT_EXPORT
#    ifdef osvrClientKit_EXPORTS
        /* We are building this library */
#      define OSVR_CLIENTKIT_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define OSVR_CLIENTKIT_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef OSVR_CLIENTKIT_NO_EXPORT
#    define OSVR_CLIENTKIT_NO_EXPORT 
#  endif
#endif

#ifndef OSVR_CLIENTKIT_DEPRECATED
#  define OSVR_CLIENTKIT_DEPRECATED __declspec(deprecated)
#endif

#ifndef OSVR_CLIENTKIT_DEPRECATED_EXPORT
#  define OSVR_CLIENTKIT_DEPRECATED_EXPORT OSVR_CLIENTKIT_EXPORT OSVR_CLIENTKIT_DEPRECATED
#endif

#ifndef OSVR_CLIENTKIT_DEPRECATED_NO_EXPORT
#  define OSVR_CLIENTKIT_DEPRECATED_NO_EXPORT OSVR_CLIENTKIT_NO_EXPORT OSVR_CLIENTKIT_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define OSVR_CLIENTKIT_NO_DEPRECATED
#endif

#endif
