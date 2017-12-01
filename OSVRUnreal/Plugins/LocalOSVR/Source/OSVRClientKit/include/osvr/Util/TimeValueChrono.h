/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
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

#ifndef INCLUDED_TimeValueChrono_h_GUID_19CA90DA_70CF_4CBD_2327_5B6335744E91
#define INCLUDED_TimeValueChrono_h_GUID_19CA90DA_70CF_4CBD_2327_5B6335744E91

// Internal Includes
// - none

// Library/third-party includes
#include <osvr/Util/TimeValue.h>

// Standard includes
#include <chrono>

/// Add a util::time::TimeValue and a std::chrono::duration
///
/// NB: Can't have this in the usual namespaces because TimeValue is actually a
/// typedef for the (un-namespaced) C struct, so ADL doesn't go looking in those
/// namespaces, just in std:: and std::chrono:: (because of the second
/// argument).
template <typename Rep, typename Period>
inline OSVR_TimeValue
operator+(OSVR_TimeValue const &tv,
          std::chrono::duration<Rep, Period> const &additionalTime) {
    using namespace std::chrono;
    using SecondsDuration = duration<OSVR_TimeValue_Seconds>;
    using USecondsDuration = duration<OSVR_TimeValue_Microseconds, std::micro>;
    auto ret = tv;
    auto seconds = duration_cast<SecondsDuration>(additionalTime);
    ret.seconds += seconds.count();
    ret.microseconds +=
        duration_cast<USecondsDuration>(additionalTime - seconds).count();
    osvrTimeValueNormalize(&ret);
    return ret;
}

#endif // INCLUDED_TimeValueChrono_h_GUID_19CA90DA_70CF_4CBD_2327_5B6335744E91
