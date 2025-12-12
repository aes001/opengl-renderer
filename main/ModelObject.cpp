// Includes
#include "ModelObject.hpp"
#include <rapidobj/rapidobj.hpp>
#include <stb_image.h>
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec2.hpp"


using namespace rapidobj;


ModelObject::ModelObject( const char* objPath, uint32_t loadFlags /*= kLoadEverything*/ )
	: mLoadFlags( loadFlags )
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


			if ( loadFlags & kLoadTextureCoords )
			{
				mTextureCoords.emplace_back( Vec2f{
					result.attributes.texcoords[idx.texcoord_index*2+0],
					result.attributes.texcoords[idx.texcoord_index*2+1]
				} );
			}


			// Always triangles, so we can find the face index by dividing the vertex index by three
			auto const& mat = result.materials[shape.mesh.material_ids[i/3]];

			// Yes we are assuming that there is only one shape and one diffuse texture
			mDiffuseTexturePath = "";
			if (!mat.diffuse_texname.empty())
			{
				std::filesystem::path filePath( objPath );
				mDiffuseTexturePath = filePath.replace_filename(mat.diffuse_texname).string();
			}

			if ( loadFlags & kLoadVertexColour )
			{
				mVertexColours.emplace_back( Vec3f{
					mat.diffuse[0],
					mat.diffuse[1],
					mat.diffuse[2]
				} );
			}

			if ( loadFlags & kLoadVertexAmbient )
			{
				mVertexAmbient.emplace_back( Vec3f{
					mat.ambient[0],
					mat.ambient[1],
					mat.ambient[2]
				} );
			}

			if ( loadFlags & kLoadVertexSpecular )
			{
				mVertexSpecular.emplace_back( Vec3f{
					mat.specular[0],
					mat.specular[1],
					mat.specular[2]
				} );
			}


			if ( loadFlags & kLoadVertexShininess )
			{
				mVertexShininess.emplace_back( mat.shininess );
			}
		}
	}

	vertNormalsList.resize(mVertices.size());

	for (size_t i = 0; i < mVertices.size()-3; i+=3) {
		Vec3f normal = CalculateNormal(mVertices[i], mVertices[i + 1], mVertices[i + 2]);

		vertNormalsList[i].emplace_back(normal);
		vertNormalsList[i + 1].emplace_back(normal);
		vertNormalsList[i + 2].emplace_back(normal);
	}

	for( size_t i = 0; i < vertNormalsList.size(); i++ )
	{
		Vec3f sum = { 0.f, 0.f, 0.f };
		for( size_t j = 0; j < vertNormalsList[i].size(); j++ )
		{
			sum += vertNormalsList[i][j];
		}

		Vec3f sqr_sum = square(sum);
		float length = sqrt(sqr_sum[0] + sqr_sum[1] + sqr_sum[2]);
		Vec3f vertNormal = { sum / length };
		mNormals.emplace_back(vertNormal);
	}

}


ModelObject::ModelObject( std::vector<Vec3f> positions, std::vector<Vec3f> normals, std::vector<Vec3f> colours, std::vector<Vec3f> specular, std::vector<float> shininess )
	: mLoadFlags ( kLoadVertexColour | kLoadVertexSpecular | kLoadVertexShininess )
	, mVertices( std::move(positions) )
	, mVertexColours( std::move(colours) )
	, mVertexSpecular( std::move(specular) )
	, mVertexShininess( std::move(shininess) )
	, mNormals( std::move(normals) )
{
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


const std::vector<Vec3f>& ModelObject::VertexAmbient() const
{
	return mVertexAmbient;
}


std::vector<Vec3f>& ModelObject::VertexAmbient()
{
	return mVertexAmbient;
}


const std::vector<Vec3f>& ModelObject::VertexSpecular() const
{
	return mVertexSpecular;
}


std::vector<Vec3f>& ModelObject::VertexSpecular()
{
	return mVertexSpecular;
}


const std::vector<float>& ModelObject::VertexShininess() const
{
	return mVertexShininess;
}


std::vector<float>& ModelObject::VertexShininess()
{
	return mVertexShininess;
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


uint32_t ModelObject::LoadFlags() const
{
	return mLoadFlags;
}


void ModelObject::OriginToGeometry()
{
	Vec3f min{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
	Vec3f max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for( auto& v : mVertices )
	{
		min.x = std::min(v.x, min.x);
		min.y = std::min(v.y, min.y);
		min.z = std::min(v.z, min.z);

		max.x = std::max(v.x, max.x);
		max.y = std::max(v.y, max.y);
		max.z = std::max(v.z, max.z);
	}

	Vec3f center = (min + max) / 2.f;

	for( auto& v : mVertices )
	{
		v -= center;
	}
}


GLuint LoadTexture2D( char const* aPath )
{
	// ACKNOWLEDGEMENT
	// Code in this function is taken from Exercise G.6 of ExerciseG6.pdf
	// Specifically the function: 'GLuint load_texture_2d( char const* aPath );'
	// Many thanks to Markus Billeter for providing the code in that exercise.

	assert( aPath );

	// Load image first
	// This may fail (e.g., image does not exist), so there�s no point in
	// allocating OpenGL resources ahead of time.
	stbi_set_flip_vertically_on_load( true );

	int w, h, channels;
	stbi_uc* ptr = stbi_load( aPath, &w, &h, &channels, STBI_rgb_alpha );
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
	, mVboVertexAmbient(0)
	, mVboVertexSpecular(0)
	, mVboVertexShininess(0)
	, mVboNormals(0)
	, mVboTextureCoords(0)
	, mDiffuseTexture(0)
{
	CreatePositionsVBO( model );
	CreateNormalsVBO( model );


	uint32_t loadFlags = model.LoadFlags();

	if( loadFlags & kLoadVertexColour )
	{
		CreateVertexColourVBO( model );
	}

	if ( loadFlags & kLoadTextureCoords )
	{
		CreateTextureCoordsVBO( model );
	}

	if( loadFlags & kLoadVertexAmbient )
	{
		CreateVertexAmbientVBO( model );
	}

	if( loadFlags & kLoadVertexSpecular )
	{
		CreateVertexSpecularVBO( model );
	}

	if( loadFlags & kLoadVertexShininess )
	{
		CreateVertexShininessVBO( model );
	}


	// Only load textures if we have UVs
	if ( loadFlags & kLoadTextureCoords )
	{
		// !!! IMPORTANT !!!
		// Ensure that the ModelObject.DiffuseTexturePath() path is valid!
		// or else we will crash!
		CreateTexture( model );
	}


	// Clean up
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


ModelObjectGPU::~ModelObjectGPU()
{
	ReleaseBuffers();
}


ModelObjectGPU::ModelObjectGPU( ModelObjectGPU&& other ) noexcept
	: mVboPositions       ( std::exchange(other.mVboPositions, 0) )
	, mVboVertexColor     ( std::exchange(other.mVboVertexColor, 0) )
	, mVboVertexAmbient	  ( std::exchange(other.mVboVertexAmbient, 0) )
	, mVboVertexSpecular  ( std::exchange(other.mVboVertexSpecular, 0) )
	, mVboVertexShininess ( std::exchange(other.mVboVertexShininess, 0) )
	, mVboNormals         ( std::exchange(other.mVboNormals, 0) )
	, mVboTextureCoords   ( std::exchange(other.mVboTextureCoords, 0) )
	, mDiffuseTexture     ( std::exchange(other.mDiffuseTexture, 0) )
{
}


ModelObjectGPU& ModelObjectGPU::operator=( ModelObjectGPU&& other ) noexcept
{
	if( this != &other )
	{
		ReleaseBuffers();

		mVboPositions       = std::exchange( other.mVboPositions, 0 );
		mVboVertexColor     = std::exchange( other.mVboVertexColor, 0 );
		mVboVertexAmbient   = std::exchange( other.mVboVertexAmbient, 0 );
		mVboVertexSpecular  = std::exchange( other.mVboVertexSpecular, 0 );
		mVboVertexShininess = std::exchange( other.mVboVertexShininess, 0 );
		mVboNormals         = std::exchange( other.mVboNormals, 0 );
		mVboTextureCoords   = std::exchange( other.mVboTextureCoords, 0 );
		mDiffuseTexture     = std::exchange( other.mDiffuseTexture, 0 );
	}

	return *this;
}


GLuint ModelObjectGPU::BufferId(eBufferType bufferType) const
{
	GLuint ret = 0;

	switch( bufferType )
	{
		case kVboPositions:
			ret = mVboPositions;
			break;
		case kVboVertexColor:
			ret = mVboVertexColor;
			break;
		case kVboVertexAmbient:
			ret = mVboVertexAmbient;
			break;
		case kVboVertexSpecular:
			ret = mVboVertexSpecular;
			break;
		case kVboVertexShininess:
			ret = mVboVertexShininess;
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


void ModelObjectGPU::CreateVertexAmbientVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboVertexAmbient );
	glBindBuffer( GL_ARRAY_BUFFER, mVboVertexAmbient );
	glBufferData( GL_ARRAY_BUFFER, model.VertexAmbient().size() * sizeof(Vec3f), model.VertexAmbient().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateVertexSpecularVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboVertexSpecular );
	glBindBuffer( GL_ARRAY_BUFFER, mVboVertexSpecular );
	glBufferData( GL_ARRAY_BUFFER, model.VertexSpecular().size() * sizeof(Vec3f), model.VertexSpecular().data(), GL_STATIC_DRAW );
}


void ModelObjectGPU::CreateVertexShininessVBO( const ModelObject& model )
{
	glGenBuffers( 1, &mVboVertexShininess );
	glBindBuffer( GL_ARRAY_BUFFER, mVboVertexShininess );
	glBufferData( GL_ARRAY_BUFFER, model.VertexShininess().size() * sizeof(float), model.VertexShininess().data(), GL_STATIC_DRAW );
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


void ModelObjectGPU::ReleaseBuffers()
{
	glDeleteBuffers( 1, &mVboPositions );
	glDeleteBuffers( 1, &mVboVertexColor );
	glDeleteBuffers( 1, &mVboVertexAmbient );
	glDeleteBuffers( 1, &mVboVertexSpecular );
	glDeleteBuffers( 1, &mVboVertexShininess );
	glDeleteBuffers( 1, &mVboNormals );
	glDeleteBuffers( 1, &mVboTextureCoords );


	glDeleteTextures( 1, &mDiffuseTexture );


	mVboPositions       = 0;
	mVboVertexColor     = 0;
	mVboVertexAmbient   = 0;
	mVboVertexSpecular  = 0;
	mVboVertexShininess = 0;
	mVboNormals         = 0;
	mVboTextureCoords   = 0;
	mDiffuseTexture     = 0;
}


Mat44f Transform::Matrix() const
{
	Mat44f rotX = make_rotation_x( mRotation.x );
	Mat44f rotY = make_rotation_y( mRotation.y );
	Mat44f rotZ = make_rotation_z( mRotation.z );

	Mat44f rotation = rotZ * rotY * rotX;

	Mat44f translate = make_translation( mPosition );
	Mat44f scale = make_scaling( mScale.x, mScale.y, mScale.z );

	return translate * rotation * scale;
}

Mat33f Transform::NormalUpdateMatrix() const
{
	Mat44f rotX = make_rotation_x(mRotation.x);
	Mat44f rotY = make_rotation_y(mRotation.y);
	Mat44f rotZ = make_rotation_z(mRotation.z);

	Mat44f rotation = rotZ * rotY * rotX;

	Mat44f scale = make_scaling(mScale.x, mScale.y, mScale.z);

	Mat44f N4 = rotation * invert(scale);

	Mat33f N3 = { N4[0,0], N4[0,1], N4[0,2],
				  N4[1,0], N4[1,1], N4[1,2],
				  N4[2,0], N4[2,1], N4[2,2]};

	return N3;
}

ObjectInstanceGroup::ObjectInstanceGroup( ModelObjectGPU& modelObjectGPU )
	: mModelObjectGPU( modelObjectGPU )
{
}


void ObjectInstanceGroup::CreateInstance( const Transform& transform )
{
	mTransformList.push_back( transform );
}


size_t ObjectInstanceGroup::GetInstanceCount()
{
	return mTransformList.size();
}


std::vector<Mat44f> ObjectInstanceGroup::GetProjCameraWorldArray(const Mat44f& projection, const Mat44f& world2camera) const
{
	std::vector<Mat44f> ret;
	ret.reserve( mTransformList.size() );

	for( const auto& transform : mTransformList )
	{
		ret.push_back( projection * world2camera * transform.Matrix() );
	}

	return ret;
}


std::vector<std::array<float, 3>> ObjectInstanceGroup::GetTranslationArray()
{
	std::vector<std::array<float, 3>> ret;
	ret.reserve(GetInstanceCount());

	for (const auto& transform : GetTransforms())
	{
		std::array<float, 3> transTemp{
			transform.mPosition.x,
			transform.mPosition.y,
			transform.mPosition.z
		};
		ret.push_back(std::move(transTemp));
	}

	return ret;
}

std::vector<Mat33f> ObjectInstanceGroup::GetNormalUpdateArray() const
{
	std::vector<Mat33f> ret;
	ret.reserve(mTransformList.size());

	for (const auto& transform : GetTransforms())
	{
		ret.push_back(transform.NormalUpdateMatrix());
	}

	return ret;
}



const std::vector<Transform>& ObjectInstanceGroup::GetTransforms() const
{
	return mTransformList;
}


const Transform& ObjectInstanceGroup::GetTransform( size_t instanceIndex ) const
{
	return mTransformList.at( instanceIndex );
}


Transform& ObjectInstanceGroup::GetTransform( size_t instanceIndex )
{
	return mTransformList.at( instanceIndex );
}


const ModelObjectGPU& ObjectInstanceGroup::GetModel() const
{
	return mModelObjectGPU;
}


ModelObjectGPU& ObjectInstanceGroup::GetModel()
{
	return mModelObjectGPU;
}


