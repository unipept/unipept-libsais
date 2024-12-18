# unipept-libsais (Forked version of libsais)

This fork of `libsais` includes modifications. The libsais library is a tool for fast linear time suffix array based on induced sorting algorithm described in the following papers: 
* Ge Nong, Sen Zhang, Wai Hong Chan *Two Efficient Algorithms for Linear Suffix Array Construction*, 2009
* Juha Karkkainen, Giovanni Manzini, Simon J. Puglisi *Permuted Longest-Common-Prefix Array*, 2009
* Nataliya Timoshevskaya, Wu-chun Feng *SAIS-OPT: On the characterization and optimization of the SA-IS algorithm for suffix array construction*, 2014
* Jing Yi Xie, Ge Nong, Bin Lao, Wentao Xu *Scalable Suffix Sorting on a Multicore Machine*, 2020

Original Copyright (c) 2021-2024 Ilya Grebnov <ilya.grebnov@gmail.com>

>The libsais is inspired by [libdivsufsort](https://github.com/y-256/libdivsufsort), [sais](https://sites.google.com/site/yuta256/sais) libraries by Yuta Mori and [msufsort](https://github.com/michaelmaniscalco/msufsort) by Michael Maniscalco.

## Changes
* Removed functionality for computing the Burrows-Wheeler transform and longest common prefix array.
* Removed OpenMP acceleration.
* Removed construction of a 32-bit suffix array.
* Added functionality for contstructing a 64-bit suffix array for a 32-bit input.

## Introduction
The libsais provides simple C99 API to construct suffix array from a given string. The algorithm runs in a linear time.

> * The libsais works with compilers from GNU, Microsoft and Intel, but I recommend Clang for best performance.
> * The libsais is sensitive to fast memory and software prefetching and might not be suitable for some workloads. Please benchmark yourself.

## License
The libsais is released under the [Apache License Version 2.0](LICENSE "Apache license")

## Versions of the libsais
* [libsais64.c](src/libsais64.c) (and corresponding [libsais64.h](include/libsais64.h)) is for suffix array construction over 8-bit inputs for inputs larger or equlas to 2GB (2147483648 bytes).
* [libsais16x64.c](src/libsais16x64.c) (and corresponding [libsais16x64.h](include/libsais16x64.h) is an independent version of the library for 16-bit inputs.
* [libsais32x64.c](src/libsais32x64.c) (and corresponding [libsais32x64.h](include/libsais32x64.h) is an independent version of the library for 32-bit inputs.

## Examples of APIs (see [libsais16x64.h](include/libsais16x64.h), see [libsais32x64.h](include/libsais32x64.h) and [libsais64.h](include/libsais64.h) for complete APIs list)
```c
    /**
    * Constructs the suffix array of a given string.
    * @param T [0..n-1] The input string.
    * @param SA [0..n-1+fs] The output array of suffixes.
    * @param n The length of the given string.
    * @param fs The extra space available at the end of SA array (0 should be enough for most cases).
    * @param freq [0..255] The output symbol frequency table (can be NULL).
    * @return 0 if no error occurred, -1 or -2 otherwise.
    */
    int64_t libsais64(const uint8_t * T, int64_t * SA, int64_t n, int64_t fs, int64_t * freq);

    /**
    * Constructs the suffix array of a given 16-bit string.
    * @param T [0..n-1] The input 16-bit string.
    * @param SA [0..n-1+fs] The output array of suffixes.
    * @param n The length of the given 16-bit string.
    * @param fs The extra space available at the end of SA array (0 should be enough for most cases).
    * @param freq [0..65535] The output 16-bit symbol frequency table (can be NULL).
    * @return 0 if no error occurred, -1 or -2 otherwise.
    */
    int64_t libsais16x64(const uint16_t * T, int64_t * SA, int64_t n, int64_t fs, int64_t * freq);

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
    int64_t libsais32x64(const uint32_t * T, int64_t * SA, int64_t n, int64_t k, int64_t fs, int64_t * freq);
```

# Algorithm description
The libsais uses the SA-IS (Suffix Array Induced Sorting) algorithm to construct both the suffix array through recursive decomposition and induced sorting:
* Initially, the algorithm classifies each position in a string as either an S-type or an L-type, based on whether the suffix starting at that position is lexicographically smaller or larger than the suffix at the adjacent right position. Positions identified as S-type, which have an adjacent left L-type position, are further categorized as LMS-type (Leftmost S-type) positions. Next, the algorithm splits the input string into LMS substrings, which start at an LMS-type position and extend up to the next adjacent LMS-type position. These LMS substrings are then lexicographically sorted through induced sorting and subsequently replaced in the input string with their corresponding sorted ranks, thus forming a new, compacted string. This compacted string reduces the problem size, enabling the algorithm to perform a recursive decomposition in which it is reapplied to construct the suffix array for the compacted string. And at the end of the recursive call, the suffix array for the input string is constructed from the suffix array of the compacted string using another round of induced sorting.
* The induced sorting is a core mechanic of the SA-IS algorithm and is employed twice during each recursive call: initially before the recursive call to establish the order of LMS substrings, and subsequently after the recursive call to finalize the order of the suffixes of the string. This process involves two sequential scans: a left-to-right scan that determines the order of L-type positions based on the LMS-type positions, followed by a right-to-left scan that establishes the order of S-type positions based on L-type positions. These scans efficiently extend the ordering from LMS-type positions to all positions in the string.

The SA-IS algorithm is quite elegant, yet implementing it efficiently presents multiple challenges. The primary challenge is that the SA-IS algorithm exhibits random memory access patterns, which can significantly decrease efficiency due to cache misses. Another significant challenge is that the SA-IS algorithm is not a lightweight construction algorithm; it requires additional memory to support positions classification, induced sorting, compacted string representations, and recursive decomposition. To circumvent this, the libsais implements careful optimizations that are worth highlighting:
* The libsais is meticulously designed from the ground up to leverage the capabilities of modern microprocessors, aiming to minimize various stalls and enhance throughput through instruction-level parallelism. The library employs sophisticated techniques such as manual loop unrolling, software prefetching, and branch elimination to achieve this goal. Moreover, it strives to minimize the number of passes over the data by combining multiple operations into a single function. A prime example of these techniques could be observed in the initialization phase of the SA-IS algorithm. In this phase, the entire logic required to classify positions, count symbols into various buckets, and segment the string into LMS substrings is executed through a single, completely branch-less loop:
```c
        for (i = m - 1, j = omp_block_start + prefetch_distance + 3; i >= j; i -= 4)
        {
            libsais_prefetchr(&T[i - 2 * prefetch_distance]);

            libsais_prefetchw(&buckets[BUCKETS_INDEX4(T[i - prefetch_distance - 0], 0)]);
            libsais_prefetchw(&buckets[BUCKETS_INDEX4(T[i - prefetch_distance - 1], 0)]);
            libsais_prefetchw(&buckets[BUCKETS_INDEX4(T[i - prefetch_distance - 2], 0)]);
            libsais_prefetchw(&buckets[BUCKETS_INDEX4(T[i - prefetch_distance - 3], 0)]);

            c1 = T[i - 0]; s = (s << 1) + (fast_uint_t)(c1 > (c0 - (fast_sint_t)(s & 1))); SA[m] = (sa_sint_t)(i + 1); m -= ((s & 3) == 1);
            buckets[BUCKETS_INDEX4((fast_uint_t)c0, s & 3)]++;

            c0 = T[i - 1]; s = (s << 1) + (fast_uint_t)(c0 > (c1 - (fast_sint_t)(s & 1))); SA[m] = (sa_sint_t)(i - 0); m -= ((s & 3) == 1);
            buckets[BUCKETS_INDEX4((fast_uint_t)c1, s & 3)]++;

            c1 = T[i - 2]; s = (s << 1) + (fast_uint_t)(c1 > (c0 - (fast_sint_t)(s & 1))); SA[m] = (sa_sint_t)(i - 1); m -= ((s & 3) == 1);
            buckets[BUCKETS_INDEX4((fast_uint_t)c0, s & 3)]++;

            c0 = T[i - 3]; s = (s << 1) + (fast_uint_t)(c0 > (c1 - (fast_sint_t)(s & 1))); SA[m] = (sa_sint_t)(i - 2); m -= ((s & 3) == 1);
            buckets[BUCKETS_INDEX4((fast_uint_t)c1, s & 3)]++;
        }
```
* To sort LMS substrings lexicographically and compute their ranks, the libsais algorithm begins by gathering LMS-type positions as they appear in the string, placing them at the end of the suffix array. The library then employs two passes of induced sorting, which concludes with these same LMS-type positions ordered lexicographically at the beginning of the suffix array. Once all LMS-type positions are sorted, the ranks of the LMS substrings are computed by inspecting each pair of adjacent positions to determine if the corresponding LMS substrings are identical. If they are the same, they receive the same rank; otherwise, the rank is incremented by one. 
  * The first challenge of induced sorting is that, during passes over the suffix array, we need to examine each value to determine if it represents a valid position or an empty space, whether this position is not the beginning of the string (and thus could induce another position), and if the induced position is going to be of the necessary type (for example, during a left-to-right scan, we are only inducing L-type positions). This process can cause branch mispredictions and corresponding microprocessor stalls. To address this challenge, libsais employs following techniques. Firstly, the library uses two pointers per induction bucket, each pointing to different sections of the suffix array depending on the type of positions these positions will be inducing next. This approach allows for the separation of LS-type (meaning S-type, which induces L-type; this is the same as LMS-type) and LL-type positions needed for the left-to-right scan from SL-type and SS-type positions needed for the right-to-left scan. Secondly, by understanding the distribution of symbols based on their position types and the types they induce (i.e., SS, SL, LS, LL), we can pre-calculate pointers for each bucket, leaving no empty spaces. And thirdly, by removing the first LMS position and all positions left of it from the initial gathering and distribution, we eliminate the need to check whether a position is not the beginning of the string. These techniques not only result in a completely branch-less loop for each induction sorting pass but also eliminate redundant scanning and the final gathering of LMS-type positions at the beginning of the suffix array.
  * The second challenge arises after induced sorting when we need to compute the ranks of LMS substrings. To accomplish this, we must first calculate and store the lengths of LMS substrings and then inspect each pair of adjacent LMS-type positions to determine if the corresponding LMS substrings are identical. This comparison starts with their lengths, and if they are the same, proceeds to compare the substrings themselves. Such operations exhibits random memory access patterns, which can significantly decrease efficiency due to cache misses. However, libsais avoids this inefficient logic by incorporating the ranking of LMS substrings as part of the induced sorting process itself. The library achieves this by marking the most significant bit (MSB) of positions in the suffix array that start new ranking groups. Each time a position is processed during induced sorting, the library checks the MSB and increments the current rank if the beginning of a new ranking group is encountered. Additionally, for each pointer in an induction bucket, the rank of the previous induced position is maintained. Whenever another position is induced, this previous rank is used to determine whether to mark the newly induced position as the beginning of a new rank group. All the logic to update the ranks and mark the beginnings of new ranking groups is implemented using bit manipulation and is completely branch-less.
```c
    for (i = omp_block_start, j = omp_block_start + omp_block_size - 2 * prefetch_distance - 1; i < j; i += 2)
    {
        libsais_prefetchr(&SA[i + 3 * prefetch_distance]);

        libsais_prefetchr(&T[SA[i + 2 * prefetch_distance + 0] & SAINT_MAX] - 1);
        libsais_prefetchr(&T[SA[i + 2 * prefetch_distance + 0] & SAINT_MAX] - 2);
        libsais_prefetchr(&T[SA[i + 2 * prefetch_distance + 1] & SAINT_MAX] - 1);
        libsais_prefetchr(&T[SA[i + 2 * prefetch_distance + 1] & SAINT_MAX] - 2);

        sa_sint_t p0 = SA[i + prefetch_distance + 0] & SAINT_MAX; sa_sint_t v0 = BUCKETS_INDEX4(T[p0 - (p0 > 0)], 0); libsais_prefetchw(&buckets[v0]);
        sa_sint_t p1 = SA[i + prefetch_distance + 1] & SAINT_MAX; sa_sint_t v1 = BUCKETS_INDEX4(T[p1 - (p1 > 0)], 0); libsais_prefetchw(&buckets[v1]);

        sa_sint_t p2 = SA[i + 0]; d += (p2 < 0); p2 &= SAINT_MAX; sa_sint_t v2 = BUCKETS_INDEX4(T[p2 - 1], T[p2 - 2] >= T[p2 - 1]);
        SA[buckets[v2]++] = (p2 - 1) | ((sa_sint_t)(buckets[2 + v2] != d) << (SAINT_BIT - 1)); buckets[2 + v2] = d;

        sa_sint_t p3 = SA[i + 1]; d += (p3 < 0); p3 &= SAINT_MAX; sa_sint_t v3 = BUCKETS_INDEX4(T[p3 - 1], T[p3 - 2] >= T[p3 - 1]);
        SA[buckets[v3]++] = (p3 - 1) | ((sa_sint_t)(buckets[2 + v3] != d) << (SAINT_BIT - 1)); buckets[2 + v3] = d;
    }
```
* In the SA-IS algorithm, after induced sorting, the ranks of LMS substrings are computed in suffix order. These ranks then need to be scattered to reorder them in string order before being gathered again to form the compacted string for recursion. At this point, some LMS substrings may be unique, meaning they don't share their rank with any other LMS substring. Being unique, these substrings are essentially already sorted, and their position relative to other LMS substrings is already determined. However, these unique LMS substrings may still be necessary for sorting other, non-unique LMS substrings during recursion-unless a unique LMS substring is immediately followed by another unique LMS substring in the string. In such cases, the rank of any subsequent unique LMS substrings becomes redundant in the compacted string, as it will not be utilized. Leveraging this insight, libsais employs a strategy to further reduce the size of the compacted string by omitting such redundant LMS substring ranks. This process involves a few steps. First, unique LMS substrings are identified by looking ahead while scanning LMS-positions in the suffix array during the ranking and scattering phase. When scattering LMS substring ranks to form the compacted string, the most significant bit (MSB) of the rank is used to mark that this rank is unique. Next, as the library scans the ranks in string order and detects tandems of unique ranks using the MSB, it then  recalculates the MSB for ranks which are redundant, thus markign them for removal from the compacted string. Subsequently, the libsais rescans the LMS-positions in suffix order to recompute the ranks, now focusing only on the ranks of the remaining LMS substrings. The library also uses MSB of first symbol of LMS substrings to mark that LMS substring is removed from the compacted string. Finally, the library builds the compacted string based on the newly recalculated ranks for the remaining LMS substrings, while also saving the final positions for the removed LMS substrings before proceeding with recursion. This reduction process not only further decreases the size of the compacted string but also reduces the alphabet size of the reduced string and creates additional free space in the suffix array, which can be utilized during recursion.
* The SA-IS algorithm, while robust for suffix array construction, is not considered lightweight due to its need for additional memory for tasks such as position classification, induced sorting, the creation of compacted string representations, and recursive decomposition. To mitigate this, libsais optimizes memory usage by not storing position classifications and striving to reuse the memory space allocated for the suffix array for induced sorting, compacted string representations, and recursive decomposition processes. Since position classifications are not stored, the library recalculates them as needed, typically involving checks of adjacent symbols for a given position. Although this approach may seem straightforward, it introduces the challenge of random memory access. Nevertheless, libsais manages these accesses in a manner that either avoids unnecessary memory fetches or minimizes cache penalties. In situations where avoiding cache penalties is unfeasible, the library leverages the most significant bit (MSB) bits for computations, as branch mispredictions on modern microprocessors generally incur lower penalties than cache misses. Memory reuse for the suffix array, despite appearing straightforward, also presents hidden challenges related to implementation complexity. In certain cases, the available space in the suffix array may not suffice for the most optimal algorithm implementation mentioned above. Although such instances are rare, the library aims to deliver optimal performance without additional memory allocation by resorting to a less efficient variant of induced sorting. To accommodate various scenarios, libsais includes four distinct implementations tailored to different breakpoints based on alphabet size (denoted by 'k'): 6k, 4k, 2k, and 1k, with each implementation optimized to ensure performance efficiency. Extensive efforts have been dedicated to refining these implementations, including significant time invested in using various sanitizers to confirm the correctness of the algorithms. Ultimately, while there are specific inputs under which libsais might require additional memory-most of which tend to be synthetic tests designed specifically to challenge the SA-IS algorithm-such instances are relatively rare. In these exceptional cases, the library is designed to allocate only the minimum necessary amount of memory while still delivering the best possible performance.
* The libsais library, initially was developed for constructing suffix arrays, but has broadened its scope to include the calculation of the longest common prefix (LCP) and both the forward and inverse Burrows-Wheeler Transform (BWT) with considerable efforts has been dedicated to refining these algorithms to ensure they deliver maximum performance and maintain the correctness. An illustrative example is the forward BWT, which performance is nearly identical to that of its suffix array construction which is achieved by integrating a modified version of the induced sorting implementation within the final stage of the SA-IS algorithm. Rather than inducing suffix positions at this stage, the library induces the Burrows-Wheeler Transform directly. This approach also supports in-place transformation, maintaining a memory usage of 5n, making it an sutable for data compression applications. Similarly, the inverse BWT is fine-tuned to operate in-place, adhering to the same memory efficiency of 5n with an additional optimization of a bi-gram LF-mapping technique, which allows for the decoding of two symbols simultaneously effectively reduces the number of cache misses during the inversion of the Burrows-Wheeler Transform.
