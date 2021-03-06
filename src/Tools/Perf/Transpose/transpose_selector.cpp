#include <iostream>
#include <mipp.h>

#include "Tools/Exception/exception.hpp"

#ifdef __AVX2__
#include "transpose_AVX.h"
#elif defined(__SSE4_1__)
#include "transpose_SSE.h"
#endif
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include "transpose_NEON.h"
#endif

#include "transpose_selector.h"

bool aff3ct::tools::char_transpose(const signed char *src, signed char *dst, int n)
{
	bool is_transposed = false;
#if defined(__MIC__) || defined(__KNCNI__) || defined(__AVX512__) || defined(__AVX512F__)
	// is_transposed = false;
#elif defined(__AVX2__)
	if (n >= 256)
	{
		if (((uintptr_t)src) % (256 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (256 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_transpose_avx((__m256i*) src, (__m256i*) dst, n);
		is_transposed = true;
	}
#elif defined(__SSE4_1__)
	if (n >= 128)
	{
		if (((uintptr_t)src) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_transpose_sse((__m128i*) src, (__m128i*) dst, n);
		is_transposed = true;
	}
#elif (defined(__ARM_NEON__) || defined(__ARM_NEON))
	if (n >= 128)
	{
		if (((uintptr_t)src) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_transpose_neon((trans_TYPE*) src, (trans_TYPE*) dst, n);
		is_transposed = true;
	}
#else
	throw runtime_error(__FILE__, __LINE__, __func__, "Transposition does not support this architecture "
	                                                  "(supported architectures are: NEON, NEONv2, SSE4.1 and AVX2).");
#endif

	return is_transposed;
}

bool aff3ct::tools::char_itranspose(const signed char *src, signed char *dst, int n)
{
	bool is_itransposed = false;
#if defined(__MIC__) || defined(__KNCNI__) || defined(__AVX512__) || defined(__AVX512F__)
	// is_itransposed = false;
#elif defined(__AVX2__)
	if (n >= 256)
	{
		if (((uintptr_t)src) % (256 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (256 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_itranspose_avx((__m256i*) src, (__m256i*) dst, n / 8);
		is_itransposed = true;
	}
#elif defined(__SSE4_1__)
	if (n >= 128)
	{
		if (((uintptr_t)src) % (128))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_itranspose_sse((__m128i*) src, (__m128i*) dst, n / 8);
		is_itransposed = true;
	}
#elif (defined(__ARM_NEON__) || defined(__ARM_NEON))
	if (n >= 128)
	{
		if (((uintptr_t)src) % (128))
			throw runtime_error(__FILE__, __LINE__, __func__, "'src' is unaligned memory.");
		if (((uintptr_t)dst) % (128 / 8))
			throw runtime_error(__FILE__, __LINE__, __func__, "'dst' is unaligned memory.");

		uchar_itranspose_neon((trans_TYPE*) src, (trans_TYPE*) dst, n / 8);
		is_itransposed = true;
	}
#else
	throw runtime_error(__FILE__, __LINE__, __func__, "Transposition inverse does not support this architecture "
	                                                  "(supported architectures are: NEON, NEONv2, SSE4.1 and AVX2).");
#endif

	return is_itransposed;
}
