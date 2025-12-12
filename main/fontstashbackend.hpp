#ifndef FONT_STASH_BACKEND_HPP
#define FONT_STASH_BACKEND_HPP

#include "fontstash.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"


struct PITBFonsContext
{
	GLuint tex{0};
	int width{0};
	int height{0};

	GLuint vboVerts{0};
	GLuint vboTexCoords{0};
	GLuint vboColours{0};
	GLuint vao{0};
};

FONScontext* CreateFons(int width, int height, int flags);
void DeleteFons(FONScontext* ctx);
unsigned int FonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif // FONT_STASH_BACKEND_HPP