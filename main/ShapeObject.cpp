#include "ShapeObject.hpp"

#include <numbers>
#include <vector>
#include "ModelObject.hpp"
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"


Vec3f CalculateNormal(Vec3f vertexA, Vec3f vertexB, Vec3f vertexC)
{
	//calculate vectors
	Vec3f u = vertexC - vertexA;
	Vec3f v = vertexB - vertexA;
	//find normal of vectors
	Vec3f n = cross(v, u);

	n = n / sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]); //normalise
	return n;
}


ModelObject MakeCylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Transform aPreTransform )
{
	std::vector<Vec3f> pos;
	std::vector<Vec3f> normals;


	float prevY = std::cos( 0.f ); // 1
	float prevZ = std::sin( 0.f ); // 0
	for( std::size_t i = 0; i < aSubdivs; ++i )
	{
		float const angle = (i+1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
		float y = std::cos( angle );
		float z = std::sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		if( aCapped )
		{
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
			pos.emplace_back( Vec3f{ 0.f, 0.f, 0.f } );
			pos.emplace_back( Vec3f{ 0.f, y, z } );

			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0.f, 0.f } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );
		}

		prevY = y;
		prevZ = z;
	}

	Mat44f preTransform = aPreTransform.Matrix();
	for( auto& p : pos )
	{
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = preTransform * p4;
		t /= t.w;

		p = Vec3f{ t.x, t.y, t.z };
	}

	std::vector<std::vector<Vec3f>> vertNormalsList;
	vertNormalsList.resize(pos.size());

	for (int i = 0; i < pos.size(); i+=3) {
		Vec3f normal = CalculateNormal(pos[i], pos[i + 1], pos[i + 2]);

		vertNormalsList[i].emplace_back(normal);
		vertNormalsList[i + 1].emplace_back(normal);
		vertNormalsList[i + 2].emplace_back(normal);
	}

	for( int i = 0; i < vertNormalsList.size(); i++ )
	{
		Vec3f sum = { 0.f, 0.f, 0.f };
		Vec3f denom = { 0.f, 0.f, 0.f };
		for( int j = 0; j < vertNormalsList[i].size(); j++ )
		{
			sum += vertNormalsList[i][j];
		}

		Vec3f sqr_sum = square(sum);
		float length = sqrt(sqr_sum[0] + sqr_sum[1] + sqr_sum[2]);
		Vec3f vertNormal = { sum / length };
		normals.emplace_back(vertNormal);
	}

	std::vector col{pos.size(), aColor};

	return ModelObject{ std::move(pos), std::move(normals), std::move(col) };
}


ModelObject MakeCone( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Transform aPreTransform )
{
	std::vector<Vec3f> pos;
	std::vector<Vec3f> normals;
	float prevY = std::cos( 0.f ); // 1
	float prevZ = std::sin( 0.f ); // 0

	for( std::size_t i = 0; i < aSubdivs; ++i )
	{
		float const angle = (i+1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
		float y = std::cos( angle );
		float z = std::sin( angle );

		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 1.f, 0.f, 0.f } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );

		if( aCapped )
		{
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0.f, 0.f } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		}

		prevY = y;
		prevZ = z;
	}


	Mat44f preTransform = aPreTransform.Matrix();
	for( auto& p : pos )
	{
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = preTransform * p4;
		t /= t.w;

		p = Vec3f{ t.x, t.y, t.z };
	}


	std::vector<std::vector<Vec3f>> vertNormalsList;
	vertNormalsList.resize(pos.size());

	for (int i = 0; i < pos.size(); i+=3) {
		Vec3f normal = CalculateNormal(pos[i], pos[i + 1], pos[i + 2]);

		vertNormalsList[i].emplace_back(normal);
		vertNormalsList[i + 1].emplace_back(normal);
		vertNormalsList[i + 2].emplace_back(normal);
	}

	for( int i = 0; i < vertNormalsList.size(); i++ )
	{
		Vec3f sum = { 0.f, 0.f, 0.f };
		Vec3f denom = { 0.f, 0.f, 0.f };
		for( int j = 0; j < vertNormalsList[i].size(); j++ )
		{
			sum += vertNormalsList[i][j];
		}

		Vec3f sqr_sum = square(sum);
		float length = sqrt(sqr_sum[0] + sqr_sum[1] + sqr_sum[2]);
		Vec3f vertNormal = { sum / length };
		normals.emplace_back(vertNormal);
	}

	std::vector col{pos.size(), aColor};

	return ModelObject{ std::move(pos), std::move(normals), std::move(col) };
}


ModelObject MakeCube( Vec3f aColor, Transform aPreTransform )
{
	std::vector<Vec3f> pos;
	std::vector<Vec3f> normals;

	constexpr float const kCubePositions[] = {
		+1.f, +1.f, -1.f,
		-1.f, +1.f, -1.f,
		-1.f, +1.f, +1.f,
		+1.f, +1.f, -1.f,
		-1.f, +1.f, +1.f,
		+1.f, +1.f, +1.f,

		+1.f, -1.f, +1.f,
		+1.f, +1.f, +1.f,
		-1.f, +1.f, +1.f,
		+1.f, -1.f, +1.f,
		-1.f, +1.f, +1.f,
		-1.f, -1.f, +1.f,

		-1.f, -1.f, +1.f,
		-1.f, +1.f, +1.f,
		-1.f, +1.f, -1.f,
		-1.f, -1.f, +1.f,
		-1.f, +1.f, -1.f,
		-1.f, -1.f, -1.f,

		-1.f, -1.f, -1.f,
		+1.f, -1.f, -1.f,
		+1.f, -1.f, +1.f,
		-1.f, -1.f, -1.f,
		+1.f, -1.f, +1.f,
		-1.f, -1.f, +1.f,

		+1.f, -1.f, -1.f,
		+1.f, +1.f, -1.f,
		+1.f, +1.f, +1.f,
		+1.f, -1.f, -1.f,
		+1.f, +1.f, +1.f,
		+1.f, -1.f, +1.f,

		-1.f, -1.f, -1.f,
		-1.f, +1.f, -1.f,
		+1.f, +1.f, -1.f,
		-1.f, -1.f, -1.f,
		+1.f, +1.f, -1.f,
		+1.f, -1.f, -1.f,
	};


	Mat44f preTransform = aPreTransform.Matrix();
	for( size_t i = 0; i < (sizeof(kCubePositions) / sizeof(*kCubePositions)); i+= 3)
	{
		Vec4f p4{ kCubePositions[i+0], kCubePositions[i+1], kCubePositions[i+2], 1.f };
		Vec4f t = preTransform * p4;
		t /= t.w;

		pos.emplace_back( t.x, t.y, t.z );
	}


	std::vector<std::vector<Vec3f>> vertNormalsList;
	vertNormalsList.resize(pos.size());

	for (int i = 0; i < pos.size(); i+=3) {
		Vec3f normal = CalculateNormal(pos[i], pos[i + 1], pos[i + 2]);

		normals.emplace_back(normal);
		normals.emplace_back(normal);
		normals.emplace_back(normal);
	}

	std::vector col{pos.size(), aColor};

	return ModelObject{ std::move(pos), std::move(normals), std::move(col) };
}

