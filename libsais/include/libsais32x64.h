/*--

This file is a part of libsais, a library for linear time suffix array,
longest common prefix array and burrows wheeler transform construction.

   Copyright (c) 2021-2024 Ilya Grebnov <ilya.grebnov@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

Please see the file LICENSE for full copyright information.

--*/

/*--
Modifications made by team Unipept as of 18/10/2024 and ongoing.
--*/

#ifndef LIBSAIS32X64_H
#define LIBSAIS32X64_H 1

#define LIBSAIS32X64_VERSION_MAJOR   2
#define LIBSAIS32X64_VERSION_MINOR   8
#define LIBSAIS32X64_VERSION_PATCH   5
#define LIBSAIS32X64_VERSION_STRING  "2.8.5"

#ifdef _WIN32
    #ifdef LIBSAIS_SHARED
        #ifdef LIBSAIS_EXPORTS
            #define LIBSAIS32X64_API __declspec(dllexport)
        #else
            #define LIBSAIS32X64_API __declspec(dllimport)
        #endif
    #else
        #define LIBSAIS32X64_API
    #endif
#else
    #define LIBSAIS32X64_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

    #include <stdint.h>

    /**
    * Constructs the suffix array of a given 32-bit string.
    * @param T [0..n-1] The input 32-bit string.
    * @param SA [0..n-1+fs] The output array of suffixes.
    * @param n The length of the given 32-bit string.
    * @param k The alphabet size.
    * @param fs The extra space available at the end of SA array (0 should be enough for most cases).
    * @param freq [0..65535] The output 32-bit symbol frequency table (can be NULL).
    * @return 0 if no error occurred, -1 or -2 otherwise.
    */
    LIBSAIS32X64_API int64_t libsais32x64(const uint32_t * T, int64_t * SA, int64_t n, int64_t k, int64_t fs, int64_t * freq);

#ifdef __cplusplus
}
#endif

#endif
