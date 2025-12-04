// Includes
#include "ModelObject.hpp"
#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec3.hpp"


using namespace rapidobj;


ModelObject::ModelObject( const char* objPath )
{

	// ACKNOLEDGEMENT
	// Code in this function is heavily inspired from Exercise G.4 loadObj.cpp
	// Specifically the function: 'SimpleMeshData load_wavefront_obj( char const* aPath );'
	// Many thanks to Markus Billeter for providing the code in that exercise.

	std::vector<std::vector<Vec3f>> vertNormalsList;

	auto result = ParseFile( objPath );

	if( result.error )
	{
		std::string errorMessage("Unable to load OBJ file '" + std::string(objPath) + "'"  + ": " + result.error.code.message());
		std::runtime_error e(errorMessage);
		throw e;
	}

	Triangulate(result);

	for( auto const& shape : result.shapes )
	{
		for( std::size_t i = 0; i < shape.mesh.indices.size(); ++i )
		{
			auto const& idx = shape.mesh.indices[i];
			mVertices.emplace_back( Vec3f{
				result.attributes.positions[idx.position_index*3+0],
				result.attributes.positions[idx.position_index*3+1],
				result.attributes.positions[idx.position_index*3+2]
			} );


			// Always triangles, so we can find the face index by dividing the vertex index by three
			auto const& mat = result.materials[shape.mesh.material_ids[i/3]];

			// Just replicate the material ambient color for each vertex...
			mVertexColours.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			} );

		}

		
	}

	vertNormalsList.resize(mVertices.size());

	for (int i = 0; i < mVertices.size()-3; i+=3) {
		Vec3f normal = CalculateNormal(mVertices[i], mVertices[i + 1], mVertices[i + 2]);

		vertNormalsList[i].emplace_back(normal);
		vertNormalsList[i + 1].emplace_back(normal);
		vertNormalsList[i + 2].emplace_back(normal);
	}

	for (int i = 0; i < vertNormalsList.size(); i++) {
		Vec3f sum = { 0.f, 0.f, 0.f };
		Vec3f denom = { 0.f, 0.f, 0.f };
		for (int j = 0; j < vertNormalsList[i].size(); j++) {

			sum += vertNormalsList[i][j];

		}

		Vec3f sqr_sum = square(sum);
		float length = sqrt(sqr_sum[0] + sqr_sum[1] + sqr_sum[2]);
		Vec3f vertNormal = { sum / length };
		mNormals.emplace_back(vertNormal);
	}

}

Vec3f ModelObject::CalculateNormal(Vec3f vertexA, Vec3f vertexB, Vec3f vertexC)
{
	//calculate vectors
	Vec3f u = vertexC - vertexA; 
	Vec3f v = vertexB - vertexA;
	//find normal of vectors
	Vec3f n = cross(v, u); 

	n = n / sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]); //normalise
	return n;
}



const std::vector<Vec3f>& ModelObject::Vertices() const
{
	return mVertices;
}


std::vector<Vec3f>& ModelObject::Vertices()
{
	return mVertices;
};


const std::vector<Vec3f>& ModelObject::Normals() const
{
	return mNormals;
}


std::vector<Vec3f>& ModelObject::Normals()
{
	return mNormals;
};


const std::vector<Vec3f>& ModelObject::VertexColours() const
{
	return mVertexColours;
}


std::vector<Vec3f>& ModelObject::VertexColours()
{
	return mVertexColours;
}
