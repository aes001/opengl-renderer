#ifndef SHAPE_OBJECT_HPP
#define SHAPE_OBJECT_HPP

#include "ModelObject.hpp"
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"


ModelObject MakeCylinder( bool aCapped, size_t aSubdivs, Vec3f aColor, Transform aPreTransform );

ModelObject MakeCone( bool aCapped, size_t aSubdivs, Vec3f aColor, Transform aPreTransform );

ModelObject MakeCube( Vec3f aColor, Transform aPreTransform );


template<typename T>
concept IsModelObject = std::same_as<T, ModelObject>;

template<IsModelObject First, IsModelObject... Rest>
ModelObject CombineShapeModelObjects( const First& first, const Rest&... rest )
{
	std::vector<Vec3f> pos;
	std::vector<Vec3f> normals;
	std::vector<Vec3f> col;

	size_t totalSize = ( first.Vertices().size() + ... + rest.Vertices().size() );

	pos.reserve( totalSize );
	normals.reserve( totalSize );
	col.reserve( totalSize );

	auto append = [&] ( const ModelObject& m )
	{
		pos.insert( pos.end(), m.Vertices().begin(), m.Vertices().end() );
		normals.insert( normals.end(), m.Normals().begin(), m.Normals().end() );
		col.insert( col.end(), m.VertexColours().begin(), m.VertexColours().end() );
	};

	append(first);
	(append(rest), ...);

	return ModelObject{ std::move(pos), std::move(normals), std::move(col) };
}


#endif // SHAPE_OBJECT_HPP
