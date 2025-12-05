#ifndef MODEL_OBJECT_H
#define MODEL_OBJECT_H





// Includes
#include "../vmlib/vec2.hpp"
#include "glad/glad.h"

// Standard Library Includes
#include <vector>
#include <string>




// Forward Declarations
struct Vec3f;



// Free functions
GLuint LoadTexture2D( char const* aPath );




// Classes
class ModelObject
{
public:
	explicit ModelObject( const char* objPath );

	const std::vector<Vec3f>& Vertices() const;
	std::vector<Vec3f>& Vertices();

	const std::vector<Vec3f>& Normals() const;
	std::vector<Vec3f>& Normals();

	const std::vector<Vec3f>& VertexColours() const;
	std::vector<Vec3f>& VertexColours();

	const std::vector<Vec2f>& TextureCoords() const;
	std::vector<Vec2f>& TextureCoords();

	const std::string& DiffuseTexturePath() const;
	// If you know the diffuse texture path is wrong, set it's
	// correct value with this.
	std::string& DiffuseTexturePath();


private:
	Vec3f CalculateNormal(Vec3f vertexA, Vec3f vertexB, Vec3f vertexC);


private:
	std::vector<Vec3f> mVertices;
	std::vector<Vec3f> mVertexColours;
	std::vector<Vec2f> mTextureCoords;
	std::vector<Vec3f> mNormals;

	std::string mDiffuseTexturePath;
};




enum eVboTypes : size_t
{
	kVboPositions = 0,
	kVboVertexColor,
	kVboNormals,
	kVboTextureCoords,
	kDiffuseTexture,
	kVboTypesCount
};





class ModelObjectGPU
{
public:
	ModelObjectGPU( const ModelObject& model );
	~ModelObjectGPU();

	// Non copiable
	ModelObjectGPU( const ModelObjectGPU& ) = delete;
	ModelObjectGPU& operator=( const ModelObjectGPU& ) = delete;

	// We should define the move constructors but I can't be bothered
	// we probably won't use it anyway. If you are reading this then
	// it wasn't needed.


	GLuint vbo(eVboTypes vboType) const;


private:
	void CreatePositionsVBO( const ModelObject& model );
	void CreateVertexColourVBO( const ModelObject& model );
	void CreateNormalsVBO( const ModelObject& model );
	void CreateTextureCoordsVBO( const ModelObject& model );
	void CreateTexture( const ModelObject& model );


private:
	GLuint mVboPositions;
	GLuint mVboVertexColor;
	GLuint mVboNormals;
	GLuint mVboTextureCoords;

	GLuint mDiffuseTexture;
};



#endif // MODEL_OBJECT_H