/** @file
    @brief Header wrapping std::align or a backported version - whichever we
    have is imported into the osvr namespace.

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_StdAlignWrapper_h_GUID_9F942A35_6F5B_4895_EE43_C3E2C5C453D8
#define INCLUDED_StdAlignWrapper_h_GUID_9F942A35_6F5B_4895_EE43_C3E2C5C453D8

#define OSVR_HAVE_STDALIGN 1

#ifdef OSVR_HAVE_STDALIGN
#include <memory>
#else
#include <libcxx_backports/stdalign.h> // the backported header snipped from libc++
#endif

namespace osvr {
#ifdef OSVR_HAVE_STDALIGN
using std::align;
#else
using libcxx_backports::align;
#endif
} // namespace osvr

#endif // INCLUDED_StdAlignWrapper_h_GUID_9F942A35_6F5B_4895_EE43_C3E2C5C453D8
