#pragma once

#if defined MESH_WARRIOR_EXPORTS
#	define MESH_WARRIOR_API		__declspec(dllexport)
#elif defined MESH_WARRIOR_IMPORTS
#	define MESH_WARRIOR_API		__declspec(dllimport)
#else
#	define MESH_WARRIOR_API
#endif

#define MW_MIN(a, b)			((a) < (b) ? (a) : (b))
#define MW_MAX(a, b)			((a) > (b) ? (a) : (b))
#define MW_CLAMP(x, a, b)		MW_MIN(MW_MAX(x, a), b)