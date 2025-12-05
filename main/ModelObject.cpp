// Includes
#include "ModelObject.hpp"
#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec2.hpp"
#include <stb_image.h>


using namespace rapidobj;


ModelObject::ModelObject( const char* objPath )
{

	// ACKNOWLEDGEMENT
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

			mTextureCoords.emplace_back( Vec2f{
				result.attributes.texcoords[idx.texcoord_index*2+0],
				result.attributes.texcoords[idx.texcoord_index*2+1]
			} );


			// Always triangles, so we can find the face index by dividing the vertex index by three
			auto const& mat = result.materials[shape.mesh.material_ids[i/3]];

			// Yes we are assuming that there is only one shape and one diffuse texture
			std::filesystem::path filePath( objPath );
			mDiffuseTexturePath = filePath.replace_filename(mat.diffuse_texname).string();

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


const std::vector<Vec2f>& ModelObject::TextureCoords() const
{
	return mTextureCoords;
}


std::vector<Vec2f>& ModelObject::TextureCoords()
{
	return mTextureCoords;
}


std::string& ModelObject::DiffuseTexturePath()
{
	return mDiffuseTexturePath;
}


const std::string& ModelObject::DiffuseTexturePath() const
{
	return mDiffuseTexturePath;
}



GLuint LoadTexture2D( char const* aPath )
{
	// ACKNOWLEDGEMENT
	// Code in this function is taken from Exercise G.6 ExerciseG6.pdf
	// Specifically the function: 'GLuint load_texture_2d( char const* aPath );'
	// Many thanks to Markus Billeter for providing the code in that exercise.

	assert( aPath );

	// Load image first
	// This may fail (e.g., image does not exist), so there’s no point in
	// allocating OpenGL resources ahead of time.
	stbi_set_flip_vertically_on_load( true );

	int w, h, channels;
	stbi_uc* ptr = stbi_load( aPath, &w, &h, &channels, 4 );
	if( !ptr )
	{
		std::string errorMessage("Unable to load image " + std::string(aPath) + "\n");
		std::runtime_error e(errorMessage);
		throw e;
	}

	// Generate texture object and initialize texture with image
	GLuint tex = 0;
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr );

	stbi_image_free( ptr );

	// Generate mipmap hierarchy
	glGenerateMipmap( GL_TEXTURE_2D );

	// Configure texture
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 6.f );

	return tex;
}


ModelObjectGPU::ModelObjectGPU( const ModelObject& model )
	: mVboPositions(0)
	, mVboVertexColor(0)
	, mVboNormals(0)
	, mVboTextureCoords(0)
	, mDiffuseTexture(0)
{
	CreatePositionsVBO( model );
	CreateVertexColourVBO( model );
	CreateNormalsVBO( model );
	CreateTextureCoordsVBO( model );

	// !!! IMPORTANT !!!
	// Ensure that the ModelObject.DiffuseTexturePath() path is valid!
	// or else we will crash!
	CreateTexture( model );

	// Clean up
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


ModelObjectGPU::~ModelObjectGPU()
{
	glDeleteBuffers( 1, &mVboPositions );
	glDeleteBuffers( 1, &mVboVertexColor );
	glDeleteBuffers( 1, &mVboNormals );
	glDeleteBuffers( 1, &mVboTextureCoords );

	glDeleteTextures( 1, &mDiffuseTexture );
}


GLuint ModelObjectGPU::vbo(eVboTypes vboType) const
{
	GLuint ret = 0;

	switch( vboType )
	{
		case kVboPositions:
			ret = mVboPositions;
			break;
		case kVboVertexColor:
			ret = mVboVertexColor;
			break;
		case kVboNormals:
			ret = mVboNormals;
			break;
		case kVboTextureCoords:
			ret = mVboTextureCoords;
			break;
		case kDiffuseTexture:
			ret = mDiffuseTexture;
			break;
	};

	return ret;
}


void ModelObjectGPU::CreatePositionsVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboPositions );
	glBindBuffer( GL_ARRAY_BUFFER, mVboPositions );
	glBufferData( GL_ARRAY_BUFFER, model.Vertices().size() * sizeof(Vec3f), model.Vertices().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateVertexColourVBO( const ModelObject& model )
{
	glGenBuffers(1, &mVboVertexColor );
	glBindBuffer( GL_ARRAY_BUFFER, mVboVertexColor );
	glBufferData( GL_ARRAY_BUFFER, model.VertexColours().size() * sizeof(Vec3f), model.VertexColours().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateNormalsVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboNormals );
	glBindBuffer( GL_ARRAY_BUFFER, mVboNormals );
	glBufferData( GL_ARRAY_BUFFER, model.Normals().size() * sizeof(Vec3f), model.Normals().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateTextureCoordsVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboTextureCoords );
	glBindBuffer( GL_ARRAY_BUFFER, mVboTextureCoords );
	glBufferData( GL_ARRAY_BUFFER, model.TextureCoords().size() * sizeof(Vec2f), model.TextureCoords().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateTexture( const ModelObject& model )
{
	const std::string& texturePath = model.DiffuseTexturePath();
	if ( texturePath.empty() )
	{
		return;
	}

	mDiffuseTexture = LoadTexture2D(texturePath.c_str());
}



