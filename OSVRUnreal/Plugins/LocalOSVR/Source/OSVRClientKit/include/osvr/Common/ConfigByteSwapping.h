/** @file
    @brief Automatically-generated configuration header - do not edit!

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
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_ConfigByteSwapping_h_GUID_A0C3729C_7D63_43A8_2D4E_6DADB6400369
#define INCLUDED_ConfigByteSwapping_h_GUID_A0C3729C_7D63_43A8_2D4E_6DADB6400369

// Header containing byteswap intrinsics in MSVC
#define OSVR_HAVE_INTRIN_H

// MSVC-style: _byteswap_ushort, _byteswap_ulong, _byteswap_uint64
/* #undef OSVR_HAVE_MSVC_BYTESWAPS */

// *nix-style byteswap header
/* #undef OSVR_HAVE_BYTESWAP_H */

// bswap16, bswap32, bswap64
/* #undef OSVR_HAVE_WORKING_BSWAP */

// bswap_16, bswap_32, bswap_64
/* #undef OSVR_HAVE_WORKING_BSWAP_UNDERSCORE */

// __bswap_16, __bswap_32, __bswap_64
/* #undef OSVR_HAVE_WORKING_UNDERSCORES_BSWAP */

#endif // INCLUDED_ConfigByteSwapping_h_GUID_A0C3729C_7D63_43A8_2D4E_6DADB6400369
