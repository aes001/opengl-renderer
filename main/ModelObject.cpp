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

}





const std::vector<Vec3f>& ModelObject::Vertices() const
{
	return mVertices;
}





std::vector<Vec3f>& ModelObject::Vertices()
{
	return mVertices;
};





const std::vector<Vec3f>& ModelObject::VertexColours() const
{
	return mVertexColours;
}





std::vector<Vec3f>& ModelObject::VertexColours()
{
	return mVertexColours;
}




