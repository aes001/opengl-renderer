#include "fontstashbackend.hpp"
#include <cstring>
#include <print>
#include <iostream>

int internal_fsb__renderCreate(void* uptr, int width, int height)
{
	auto* gl = static_cast<PITBFonsContext*>( uptr );

	if ( gl )
	{
		if ( gl->tex != 0 )
		{
			glDeleteTextures(1, &gl->tex);
			gl->tex = 0;
		}


		gl->width = width;
		gl->height = height;

		glGenTextures( 1, &gl->tex );
		if ( gl->tex == 0 )
		{
			return 0; // bad
		}

		glBindTexture( GL_TEXTURE_2D, gl->tex );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, gl->width, gl->height, 0, GL_RED, GL_UNSIGNED_BYTE, 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );


		glBindTexture( GL_TEXTURE_2D, 0 );



		// Make sure we've got VBOs
		if ( gl->vboVerts == 0 )
		{
			glGenBuffers( 1, &gl->vboVerts );
		}

		if ( gl->vboTexCoords == 0 )
		{
			glGenBuffers( 1, &gl->vboTexCoords );
		}

		if ( gl->vboColours == 0 )
		{
			glGenBuffers( 1, &gl->vboColours );
		}



		// VAO Stuff now
		if (gl->vao == 0)
		{
			glGenVertexArrays( 1, &gl->vao );
		}

		glBindVertexArray( gl->vao );


		glBindBuffer( GL_ARRAY_BUFFER, gl->vboVerts );
		glVertexAttribPointer(
			0,
			2, GL_FLOAT, GL_FALSE, // XY
			0,
			0
		);
		glEnableVertexAttribArray( 0 );


		glBindBuffer( GL_ARRAY_BUFFER, gl->vboColours );
		glVertexAttribPointer(
			1,
			4, GL_UNSIGNED_BYTE, GL_TRUE, // XY
			sizeof(uint32_t), // Because packed rgba into an unsigned int
			0
		);
		glEnableVertexAttribArray( 1 );


		glBindBuffer( GL_ARRAY_BUFFER, gl->vboTexCoords );
		glVertexAttribPointer(
			2,
			2, GL_FLOAT, GL_FALSE, // XY
			0,
			0
		);
		glEnableVertexAttribArray( 2 );


		glBindVertexArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
}


int internal_fsb__renderResize(void* uptr, int width, int height)
{
	return internal_fsb__renderCreate( uptr, width, height );
}


void internal_fsb__renderUpdate(void* uptr, int* rect, const unsigned char* data)
{
	auto *gl = static_cast<PITBFonsContext*>( uptr );

	if ( gl )
	{
		/*
		* [0] = rectangle X start
		* [1] = rectangle Y start
		* [2] = rectangle X end
		* [3] = rectangle Y end
		*/
		int w = rect[2] - rect[0];
		int h = rect[3] - rect[1];

		glBindTexture( GL_TEXTURE_2D, gl->tex );

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glPixelStorei( GL_UNPACK_ROW_LENGTH, gl->width );
		glPixelStorei( GL_UNPACK_SKIP_PIXELS, rect[0] );
		glPixelStorei( GL_UNPACK_SKIP_ROWS, rect[1] );

		// Add new sub image
		glTexSubImage2D( GL_TEXTURE_2D, 0,
						 rect[0], rect[1], w, h,
						 GL_RED, GL_UNSIGNED_BYTE, data );

		glBindTexture( GL_TEXTURE_2D, 0 );

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	}
}


void internal_fsb__renderDraw(void* uptr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts)
{
	auto *gl = static_cast<PITBFonsContext*>(uptr);


	if ( gl )
	{
		if (gl->tex == 0 || nverts == 0)
		{
			// Nothing to do
			return;
		}


		// Correct shader program should already be selected by now
		glBindVertexArray( gl->vao );

		glBindBuffer( GL_ARRAY_BUFFER, gl->vboVerts );
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 2 * nverts, verts, GL_DYNAMIC_DRAW );

		glBindBuffer( GL_ARRAY_BUFFER, gl->vboColours );
		glBufferData( GL_ARRAY_BUFFER, sizeof(uint32_t) * nverts, colors, GL_DYNAMIC_DRAW );

		glBindBuffer( GL_ARRAY_BUFFER, gl->vboTexCoords );
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 2 * nverts, tcoords, GL_DYNAMIC_DRAW );


		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, gl->tex );
		glDrawArrays( GL_TRIANGLES, 0, nverts );

		// Clean up
		glBindTexture( GL_TEXTURE_2D, 0 );
		glBindVertexArray( 0 );
	}
}


void internal_fsb__renderDelete(void* uptr)
{
	auto* gl = static_cast<PITBFonsContext*>(uptr);

	if ( gl )
	{
		glDeleteTextures(1, &gl->tex);
		glDeleteBuffers(1, &gl->vboVerts);
		glDeleteBuffers(1, &gl->vboColours);
		glDeleteBuffers(1, &gl->vboTexCoords);
		glDeleteVertexArrays(1, &gl->vao);

		gl->tex = 0;
		gl->vboVerts = 0;
		gl->vboColours = 0;
		gl->vboTexCoords = 0;
		gl->vao = 0;

		delete gl;
	}
}


FONScontext* CreateFons(int width, int height, int flags)
{
	FONSparams params;
	PITBFonsContext* gl = new PITBFonsContext;

	std::memset(&params, 0, sizeof(params));

	params.width  = width;
	params.height = height;

	params.flags = (unsigned char) flags;

	params.renderCreate = internal_fsb__renderCreate;
	params.renderResize = internal_fsb__renderResize;
	params.renderUpdate = internal_fsb__renderUpdate;
	params.renderDraw   = internal_fsb__renderDraw;
	params.renderDelete = internal_fsb__renderDelete;

	params.userPtr = gl;


	return fonsCreateInternal( &params );
}


void DeleteFons(FONScontext* ctx)
{
	fonsDeleteInternal( ctx );
}


unsigned int FonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}
