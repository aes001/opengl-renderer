#ifndef MODEL_OBJECT_H
#define MODEL_OBJECT_H





// Includes
#include "glad/glad.h"
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"

// Standard Library Includes
#include <string>
#include <vector>




// Forward Declarations
struct Vec3f;
struct Mat44f;
struct Mat33f;




// Free functions
GLuint LoadTexture2D( char const* aPath );



enum ModelLoadFlags : uint32_t
{
	kLoadVertexColour    = 1 << 0,
	kLoadVertexAmbient   = 1 << 1,
	kLoadVertexSpecular  = 1 << 2,
	kLoadVertexShininess = 1 << 3,
	kLoadTextureCoords   = 1 << 4,

	kLoadEverything      = 0xFFFFFFFF
};



// Classes
class ModelObject
{
public:
	explicit ModelObject( const char* objPath, uint32_t loadFlags = kLoadEverything );
	explicit ModelObject( std::vector<Vec3f> positions, std::vector<Vec3f> normals, std::vector<Vec3f> colours );

	const std::vector<Vec3f>& Vertices() const;
	std::vector<Vec3f>& Vertices();

	const std::vector<Vec3f>& Normals() const;
	std::vector<Vec3f>& Normals();

	const std::vector<Vec3f>& VertexColours() const;
	std::vector<Vec3f>& VertexColours();

	const std::vector<Vec3f>& VertexAmbient() const;
	std::vector<Vec3f>& VertexAmbient();

	const std::vector<Vec3f>& VertexSpecular() const;
	std::vector<Vec3f>& VertexSpecular();

	const std::vector<float>& VertexShininess() const;
	std::vector<float>& VertexShininess();

	const std::vector<Vec2f>& TextureCoords() const;
	std::vector<Vec2f>& TextureCoords();

	const std::string& DiffuseTexturePath() const;
	// If you know the diffuse texture path is wrong, set it's
	// correct value with this.
	std::string& DiffuseTexturePath();

	uint32_t LoadFlags() const;


private:
	Vec3f CalculateNormal(Vec3f vertexA, Vec3f vertexB, Vec3f vertexC);


private:
	uint32_t           mLoadFlags;

	std::vector<Vec3f> mVertices;

	std::vector<Vec3f> mVertexColours;
	std::vector<Vec3f> mVertexAmbient;
	std::vector<Vec3f> mVertexSpecular;
	std::vector<float> mVertexShininess;

	std::vector<Vec2f> mTextureCoords;
	std::vector<Vec3f> mNormals;

	std::string mDiffuseTexturePath;
};




enum eBufferType : size_t
{
	kVboPositions = 0,
	kVboVertexColor,
	kVboVertexAmbient,
	kVboVertexSpecular,
	kVboVertexShininess,
	kVboNormals,
	kVboTextureCoords,
	kDiffuseTexture,
	kBufferTypeCount
};




/*
 *	Scope bound VBO resource management (RAII)
 *	This class will create your VBOs and will delete the buffers once this
 *	object goes out of scope. This is done by calling glDeleteBuffers() in its
 *	destructor.
 */
class ModelObjectGPU
{
public:
	ModelObjectGPU( const ModelObject& model );
	~ModelObjectGPU();

	// Non copiable
	ModelObjectGPU( const ModelObjectGPU& ) = delete;
	ModelObjectGPU& operator=( const ModelObjectGPU& ) = delete;

	ModelObjectGPU( ModelObjectGPU&& other ) noexcept;
	ModelObjectGPU& operator=( ModelObjectGPU&& other ) noexcept;


	GLuint BufferId( eBufferType bufferType ) const;


private:
	void CreatePositionsVBO( const ModelObject& model );
	void CreateVertexColourVBO( const ModelObject& model );
	void CreateNormalsVBO( const ModelObject& model );
	void CreateTextureCoordsVBO( const ModelObject& model );
	void CreateVertexAmbientVBO( const ModelObject& model );
	void CreateVertexSpecularVBO( const ModelObject& model );
	void CreateVertexShininessVBO( const ModelObject& model );

	void CreateTexture( const ModelObject& model );

	void ReleaseBuffers();


private:
	GLuint mVboPositions;
	GLuint mVboVertexColor;
	GLuint mVboVertexAmbient;
	GLuint mVboVertexSpecular;
	GLuint mVboVertexShininess;

	GLuint mVboNormals;
	GLuint mVboTextureCoords;

	GLuint mDiffuseTexture;
};





struct Transform
{
	Vec3f mPosition{ 0.f, 0.f, 0.f };
	Vec3f mRotation{ 0.f, 0.f, 0.f };
	Vec3f mScale   { 1.f, 1.f, 1.f };

	Mat44f Matrix() const;
	Mat33f NormalUpdateMatrix() const;
};


constexpr
Transform operator+( const Transform& left, const Transform& right ) noexcept
{
	return Transform( {
		.mPosition = left.mPosition + right.mPosition,
		.mRotation = left.mRotation + right.mRotation,
		.mScale    = left.mScale    + right.mScale
	} );
}


// Deprecated
//class ObjectInstance
//{
//public:
//	ObjectInstance( ModelObjectGPU& modelObjectGPU, Transform transform );
//	ObjectInstance( ModelObjectGPU& modelObjectGPU );
//
//	~ObjectInstance() = default;
//
//	ObjectInstance( const ObjectInstance& other ) = delete;
//	ObjectInstance& operator=( const ObjectInstance& other ) = delete;
//
//	ObjectInstance( ObjectInstance&& other ) = default;
//	ObjectInstance& operator=( ObjectInstance&& other ) = default;
//
//
//	const Transform& GetTransform() const;
//	Transform& GetTransform();
//
//	const ModelObjectGPU& GetModel() const;
//	ModelObjectGPU& GetModel();
//
//
//private:
//	ModelObjectGPU& mModelObjectGPU;
//	Transform mTransform;
//};





class ObjectInstanceGroup
{
public:
	ObjectInstanceGroup( ModelObjectGPU& modelObjectGPU );

	~ObjectInstanceGroup() = default;

	ObjectInstanceGroup( const ObjectInstanceGroup& other ) = delete;
	ObjectInstanceGroup& operator=( const ObjectInstanceGroup& other ) = delete;

	ObjectInstanceGroup( ObjectInstanceGroup&& other ) = default;
	ObjectInstanceGroup& operator=( ObjectInstanceGroup&& other ) = default;

	void CreateInstance( const Transform& transform );
	size_t GetInstanceCount();

	std::vector<Mat44f> GetProjCameraWorldArray(const Mat44f& projection, const Mat44f& world2camera) const;

	const std::vector<Transform>& GetTransforms() const;

	const Transform& GetTransform( size_t instanceIndex ) const;
	Transform& GetTransform( size_t instanceIndex );

	const ModelObjectGPU& GetModel() const;
	ModelObjectGPU& GetModel();


private:
	ModelObjectGPU& mModelObjectGPU;
	std::vector<Transform> mTransformList;
};

#endif // MODEL_OBJECT_H