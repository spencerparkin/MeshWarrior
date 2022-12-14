#pragma once

#if defined MESH_WARRIOR_EXPORTS
#	define MESH_WARRIOR_API		__declspec(dllexport)
#elif defined MESH_WARRIOR_IMPORTS
#	define MESH_WARRIOR_API		__declspec(dllimport)
#else
#	define MESH_WARRIOR_API
#endif

#define MW_EPS					1e-5
#define MW_PI					3.1415926536
#define MW_TWO_PI				(2.0 * MW_PI)
#define MW_MIN(a, b)			((a) < (b) ? (a) : (b))
#define MW_MAX(a, b)			((a) > (b) ? (a) : (b))
#define MW_CLAMP(x, a, b)		MW_MIN(MW_MAX(x, a), b)
#define MW_SQUARED(x)			((x) * (x))
#define MW_CUBED(x)				((x) * (x) * (x))
#define MW_SIGN(x)				(((x) < 0.0) ? -1.0 : 1.0)
#define MW_SWAP(a, b) \
								do { \
									a ^= b; \
									b ^= a; \
									a ^= b; \
								} while(0)

#if defined MW_DEBUG
#	define MW_ASSERT(cond)		do { \
									assert(cond); \
								} while(0)
#else
#	define MW_ASSERT(cond)
#endif