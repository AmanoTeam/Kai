#if !defined(BIGGESTINT_H)
#define BIGGESTINT_H

#include <inttypes.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
	#define HAVE_LONG_LONG 1
#endif

typedef long double bigfloat_t;

#define FORMAT_BIGGEST_FLOAT_T "Lf"

#define FORMAT_UINT_PTR_T PRIuPTR
#define FORMAT_INT_PTR_T PRIiPTR

#define strtobf strtold

#if defined(HAVE_LONG_LONG)
	typedef long long int bigint_t;
	typedef unsigned long long int biguint_t;
	
	#define FORMAT_BIGGEST_INT_T "lld"
	#define FORMAT_BIGGEST_UINT_T "llu"
	
	#define FORMAT_HEX_BIGGEST_INT_T "ll"
	#define FORMAT_HEX_BIGGEST_UINT_T "ll"
	
	#define strtobi strtoll
	#define strtobui strtoull
#else
	typedef long int bigint_t;
	typedef unsigned long int biguint_t;
	
	#define FORMAT_BIGGEST_INT_T "ld"
	#define FORMAT_BIGGEST_UINT_T "lu"
	
	#define FORMAT_HEX_BIGGEST_INT_T "l"
	#define FORMAT_HEX_BIGGEST_UINT_T "l"
	
	#define strtobi strtol
	#define strtobui strtoul
#endif

bigint_t ptobigint(const void* const pointer);
biguint_t ptobiguint(const void* const pointer);

#endif