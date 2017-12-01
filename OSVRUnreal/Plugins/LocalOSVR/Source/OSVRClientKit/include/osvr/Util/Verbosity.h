/** @file
    @brief Internal, configured header file for verbosity macros.

    If this filename ends in `.h`, don't edit it: your edits will
    be lost next time this file is regenerated!

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_Verbosity_h_GUID_92D43527_D3AC_4BB4_FA39_91252412C1FF
#define INCLUDED_Verbosity_h_GUID_92D43527_D3AC_4BB4_FA39_91252412C1FF

#include <osvr/Util/MacroToolsC.h>

#define OSVR_UTIL_DEV_VERBOSE

/** @def OSVR_DEV_VERBOSE_DISABLE
    @brief Define at the top of a file, before including Verbosity.h,
    to disable verbose messages for that file only.
*/

#if defined(OSVR_UTIL_DEV_VERBOSE) && !defined(OSVR_DEV_VERBOSE_DISABLE)

#include <osvr/Util/Log.h>
#include <osvr/Util/Logger.h>

#define OSVR_TRACE(...)                                                        \
    OSVR_UTIL_MULTILINE_BEGIN::osvr::util::log::make_logger("OSVR")->trace()   \
        << __FILE__ << ":" << __LINE__ << ": " << __VA_ARGS__;                 \
    OSVR_UTIL_MULTILINE_END
#define OSVR_DEBUG(...)                                                        \
    OSVR_UTIL_MULTILINE_BEGIN::osvr::util::log::make_logger("OSVR")->debug()   \
        << __FILE__ << ":" << __LINE__ << ": " << __VA_ARGS__;                 \
    OSVR_UTIL_MULTILINE_END
#define OSVR_DEV_VERBOSE(...)                                                  \
    OSVR_UTIL_MULTILINE_BEGIN::osvr::util::log::make_logger("OSVR")->info()    \
        << __FILE__ << ":" << __LINE__ << ": " << __VA_ARGS__;                 \
    OSVR_UTIL_MULTILINE_END

#else

#define OSVR_TRACE(...) OSVR_UTIL_MULTILINE_BEGIN OSVR_UTIL_MULTILINE_END
#define OSVR_DEBUG(...) OSVR_UTIL_MULTILINE_BEGIN OSVR_UTIL_MULTILINE_END
#define OSVR_DEV_VERBOSE(...) OSVR_UTIL_MULTILINE_BEGIN OSVR_UTIL_MULTILINE_END

#endif // OSVR_UTIL_DEV_VERBOSE && !OSVR_DEV_VERBOSE_DISABLE

#endif // INCLUDED_Verbosity_h_GUID_92D43527_D3AC_4BB4_FA39_91252412C1FF
