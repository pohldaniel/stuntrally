/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#include "model.h"
#include "joeserialize.h"

#include <fstream>
#include <string>
#include <limits>

static const std::string file_magic = "OGLVARRAYV01";

Model::Model() :
	generatedmetrics(false)
{
	// Constructor.
}

Model::Model(const std::string & filepath, std::ostream & error_output) :
	generatedmetrics(false)
{
	if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".ova")
		ReadFromFile(filepath, error_output);
	else
		Load(filepath, error_output);
}

Model::~Model()
{
	Clear();
}

bool Model::CanSave() const
{
	return false;
}

bool Model::Save(const std::string & /*strFileName*/, std::ostream & /*error_output*/) const
{
	return false;
}

bool Model::Load(const std::string & /*strFileName*/, std::ostream & /*error_output*/)
{
	return false;
}

bool Model::Load(const VertexArray & nvarray, std::ostream & /*error_output*/)
{
	Clear();

	varray = nvarray;

	GenMeshMetrics();

	return true;
}

bool Model::WriteToFile(const std::string & filepath)
{
	std::ofstream fileout(filepath.c_str());
	if (!fileout)
		return false;

	fileout.write(file_magic.c_str(), file_magic.size());
	joeserialize::BinaryOutputSerializer s(fileout);
	return Serialize(s);
}

bool Model::ReadFromFile(const std::string & filepath, std::ostream & error_output)
{
	std::ifstream filein(filepath.c_str(), std::ios_base::binary);
	if (!filein)
	{
		error_output << "Can't find file: " << filepath << std::endl;
		return false;
	}

	std::vector<char> fmagic(file_magic.size() + 1, 0);
	filein.read(fmagic.data(), file_magic.size());
	if (!filein)
	{
		error_output << "File magic read error: " << filepath << std::endl;
		return false;
	}

	if (!file_magic.compare(fmagic.data()))
	{
		error_output << "File magic is incorrect: \"" << file_magic << "\" != \"" << fmagic.data() << "\" in " << filepath << std::endl;
		return false;
	}

	joeserialize::BinaryInputSerializer s(filein);
	if (!Serialize(s))
	{
		error_output << "Serialization error: " << filepath << std::endl;
		Clear();
		return false;
	}

	GenMeshMetrics();

	return true;
}

void Model::GenMeshMetrics()
{
	const float fmax = std::numeric_limits<float>::max();
	float maxv[3] = {-fmax, -fmax, -fmax};
	float minv[3] = {+fmax, +fmax, +fmax};

	const float * verts;
	unsigned int vnum3;
	varray.GetVertices(verts, vnum3);
	assert(vnum3);

	for (unsigned int n = 0; n < vnum3; n += 3)
	{
		const float * v = verts + n;
		if (v[0] > maxv[0]) maxv[0] = v[0];
		if (v[1] > maxv[1]) maxv[1] = v[1];
		if (v[2] > maxv[2]) maxv[2] = v[2];
		if (v[0] < minv[0]) minv[0] = v[0];
		if (v[1] < minv[1]) minv[1] = v[1];
		if (v[2] < minv[2]) minv[2] = v[2];
	}

	Vec3 min(minv[0], minv[1], minv[2]);
	Vec3 max(maxv[0], maxv[1], maxv[2]);

	aabb = Aabb<float>(min, max);
	generatedmetrics = true;
}

void Model::Clear()
{
	ClearMeshData();
	generatedmetrics = false;
}

bool Model::Loaded() const
{
	return (varray.GetNumIndices() > 0);
}

void Model::ClearMeshData()
{
	varray.Clear();
}
