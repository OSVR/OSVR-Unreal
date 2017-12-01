/** @file
    @brief Configured header for internal UTF16 or Windows TCHAR to UTF8
   conversion. Using this header requires you add ${OSVR_CODECVT_LIBRARIES} to
   your PRIVATE link libraries.

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
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_WideToUTF8_h_GUID_A2C0AD0B_9DEB_4E84_5A3A_8517ED03AA32
#define INCLUDED_WideToUTF8_h_GUID_A2C0AD0B_9DEB_4E84_5A3A_8517ED03AA32

#ifndef OSVR_HAVE_CODECVT
#define OSVR_HAVE_CODECVT
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>

#ifdef OSVR_HAVE_CODECVT
#include <locale>
#include <codecvt>
#else // !OSVR_HAVE_CODECVT
#include <boost/locale/encoding_utf.hpp>
#endif // OSVR_HAVE_CODECVT

#include <string>
#include <type_traits>

namespace osvr {
namespace util {

    template <typename T> inline std::string wideToUTF8String(T input) {
#ifdef OSVR_HAVE_CODECVT
        std::wstring_convert<std::codecvt_utf8<wchar_t> > strCvt;
        return strCvt.to_bytes(input);
#else  // !OSVR_HAVE_CODECVT
        return boost::locale::conv::utf_to_utf<char>(input);
#endif // OSVR_HAVE_CODECVT
    }

    template <typename T> inline std::wstring utf8ToWideString(T input) {
#ifdef OSVR_HAVE_CODECVT
        std::wstring_convert<std::codecvt_utf8<wchar_t> > strCvt;
        return strCvt.from_bytes(input);
#else  // !OSVR_HAVE_CODECVT
        return boost::locale::conv::utf_to_utf<wchar_t>(input);
#endif // OSVR_HAVE_CODECVT
    }

#ifdef _UNICODE
    inline std::string tcharToUTF8String(TCHAR buf[]) {
        static_assert(std::is_same<TCHAR, wchar_t>::value,
                      "This code path relies on TCHAR being a wchar_t!");
        return wideToUTF8String(buf);
    }
#else  // !_UNICODE
    inline std::string tcharToUTF8String(TCHAR buf[]) {
        static_assert(std::is_same<TCHAR, char>::value,
                      "This code path relies on TCHAR being a single byte!");
        return std::string(buf);
    }
#endif // _UNICODE

} // namespace util
} // namespace osvr

#endif // _WIN32

#endif // INCLUDED_WideToUTF8_h_GUID_A2C0AD0B_9DEB_4E84_5A3A_8517ED03AA32
