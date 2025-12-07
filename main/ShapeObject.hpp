#ifndef SHAPE_OBJECT_HPP
#define SHAPE_OBJECT_HPP

#include "ModelObject.hpp"
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"


struct ShapeMaterial
{
	Vec3f mVertexColor;
	Vec3f mSpecular;
	float mShininess;
};


ModelObject MakeCylinder( bool aCapped, size_t aSubdivs, Transform aPreTransform, ShapeMaterial aMaterial );

ModelObject MakeCone( bool aCapped, size_t aSubdivs, Transform aPreTransform, ShapeMaterial aMaterial);

ModelObject MakeCube( Transform aPreTransform, ShapeMaterial aMaterial );


template<typename T>
concept IsModelObject = std::same_as<T, ModelObject>;

template<IsModelObject First, IsModelObject... Rest>
ModelObject CombineShapeModelObjects( const First& first, const Rest&... rest )
{
	std::vector<Vec3f> pos;
	std::vector<Vec3f> normals;
	std::vector<Vec3f> colour;
	std::vector<Vec3f> specular;
	std::vector<float> shininess;

	size_t totalSize = ( first.Vertices().size() + ... + rest.Vertices().size() );

	pos.reserve( totalSize );
	normals.reserve( totalSize );
	colour.reserve( totalSize );
	specular.reserve( totalSize );
	shininess.reserve( totalSize );

	auto append = [&] ( const ModelObject& m )
	{
		pos.insert( pos.end(), m.Vertices().begin(), m.Vertices().end() );
		normals.insert( normals.end(), m.Normals().begin(), m.Normals().end() );
		colour.insert( colour.end(), m.VertexColours().begin(), m.VertexColours().end() );
		specular.insert( specular.end(), m.VertexSpecular().begin(), m.VertexSpecular().end() );
		shininess.insert( shininess.end(), m.VertexShininess().begin(), m.VertexShininess().end() );
	};

	append(first);
	(append(rest), ...);

	return ModelObject{ std::move(pos), std::move(normals), std::move(colour), std::move(specular), std::move(shininess) };
}


#endif // SHAPE_OBJECT_HPP
