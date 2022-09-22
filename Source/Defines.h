#pragma once

#if defined MESH_WARRIOR_EXPORTS
#	define MESH_WARRIOR_API		__declspec(dllexport)
#elif defined MESH_WARRIOR_IMPORTS
#	define MESH_WARRIOR_API		__declspec(dllimport)
#else
#	define MESH_WARRIOR_API
#endif