#ifndef MODEL_OBJECT_H
#define MODEL_OBJECT_H





// Includes

// Standard Library Includes
#include <vector>




// Forward Declarations
struct Vec3f;





class ModelObject
{
public:
	explicit ModelObject( const char* objPath );

	const std::vector<Vec3f>& Vertices() const;
	std::vector<Vec3f>& Vertices();

	const std::vector<Vec3f>& VertexColours() const;
	std::vector<Vec3f>& VertexColours();



private:
	std::vector<Vec3f> mVertices;
	std::vector<Vec3f> mVertexColours;

};




#endif // MODEL_OBJECT_H