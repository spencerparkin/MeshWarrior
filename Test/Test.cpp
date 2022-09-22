#include "MeshFormats/OBJFormat.h"
#include "Mesh.h"

int main()
{
	using namespace MeshWarrior;

	OBJFormat objFormat;

	Mesh* mesh = objFormat.Load("Teapot.OBJ");
	if (mesh)
	{
		objFormat.Save("TeapotCopy.OBJ", *mesh);
		delete mesh;
	}

	return 0;
}