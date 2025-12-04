#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>
#include <numbers>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>

#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"
#include "ModelObject.hpp"
#include "LookAt.hpp"


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};


	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.005f; // radians per pixel
	constexpr size_t KEY_COUNT_GLFW = 349;

	struct State_
	{
		ShaderProgram* prog;
		float dt;
		float speedMod;
		bool pressedKeys[KEY_COUNT_GLFW] = { false };

		struct CamCtrl_
		{
			bool cameraActive;
			Vec3f cameraPos;
			Vec3f cameraDirection;
			Vec3f cameraRight;
			Vec3f cameraUp;

			float yaw, pitch;

			float lastX, lastY;
		} camControl;
	};


	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY );

	void glfw_callback_mouse_button_(GLFWwindow* window, int button, int action, int );

	void updateCamera(State_& state);
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '{}' ({})", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };


	// Set up event handling
	// TODO: Additional event handling setup

	State_ state{};

	// Initial camera set up
	state.camControl.cameraPos = {0.f, 0.f, -10.f};
	state.camControl.cameraDirection = {0.f, 0.f, -1.f};
	//state.camControl.cameraRight = {}
	//state.camControl.cameraUp;
	//state.camControl.yaw = -90.f;
	//state.camControl.pitch = 0.f;

	glfwSetWindowUserPointer( window, &state );

	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetCursorPosCallback( window, &glfw_callback_motion_ );
	glfwSetMouseButtonCallback( window, &glfw_callback_mouse_button_ );


	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

	std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
	std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
	std::print( "VERSION {}\n", (char const*)glGetString( GL_VERSION ) );
	std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	glEnable( GL_FRAMEBUFFER_SRGB );
	glEnable( GL_CULL_FACE );
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
	glEnable( GL_DEPTH_TEST );

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
		} );

	state.prog = &prog;
	auto last = Clock::now();


	ModelObject terrain( "assets/cw2/parlahti.obj" );
	std::vector<Vec3f>& terrainVerts = terrain.Vertices();
	std::vector<Vec3f>& terrainColours = terrain.VertexColours();
	std::vector<Vec3f>& terrainNormals = terrain.Normals();
	const size_t numTerrainVerts = terrainVerts.size();

	// VBO Creations
	GLuint vboPosition = 0;
	glGenBuffers( 1, &vboPosition );
	glBindBuffer( GL_ARRAY_BUFFER, vboPosition );
	glBufferData( GL_ARRAY_BUFFER, terrainVerts.size() * sizeof(Vec3f), terrainVerts.data(), GL_STATIC_DRAW );

	GLuint vboColor = 0;
	glGenBuffers(1, &vboColor );
	glBindBuffer( GL_ARRAY_BUFFER, vboColor );
	glBufferData( GL_ARRAY_BUFFER, terrainColours.size() * sizeof(Vec3f), terrainColours.data(), GL_STATIC_DRAW );

	GLuint vboNormals = 0;
	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, terrainNormals.size() * sizeof(Vec3f), terrainNormals.data(), GL_STATIC_DRAW);

	// Create VAO
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	//positions
	glBindBuffer( GL_ARRAY_BUFFER, vboPosition );
	glVertexAttribPointer(
		0,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 0 );
	//colours
	glBindBuffer( GL_ARRAY_BUFFER, vboColor );
	glVertexAttribPointer(
		1,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 1 );
	//normals
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glVertexAttribPointer(
		2,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray(2);

	// Reset State
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &vboColor );
	glDeleteBuffers( 1, &vboPosition  );

	OGL_CHECKPOINT_ALWAYS();

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state

		auto const now = Clock::now();
		state.dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;

		updateCamera(state);


		Mat44f model2world = kIdentity44f;

		Mat44f view = MakeLookAt(state.camControl.cameraPos,
								 state.camControl.cameraDirection,
								 state.camControl.cameraUp,
								 state.camControl.cameraRight);

		//Mat44f world2camera = Rx * Ry * T;
		Mat44f world2camera = view;
		//Mat44f world2camera = make_translation( {0.f, 0.f, -10.f });
		Mat44f projection = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			fbwidth/float(fbheight),
			0.1f, 100.0f
		);

		Mat44f projCameraWorld = projection * world2camera * model2world;


		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// EXERCISE 5
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( prog.programId() );

		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);

		Vec3f lightDir = normalize(Vec3f{ -1.f, 1.f, 0.5f }); // light direction
		glUniform3fv(1, 1, &lightDir.x);

		glUniform3f(2, 0.9f, 0.9f, 0.6f); // light diffuse
		glUniform3f(3, 0.05f, 0.05f, 0.05f); // light ambient

		glBindVertexArray( vao );
		glDrawArraysInstanced( GL_TRIANGLES, 0, numTerrainVerts, 1);

		glBindVertexArray( 0 );
		glUseProgram( 0 );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
	state.prog = nullptr;

	return 0;
}
catch( std::exception const& eErr )
{
	std::print( stderr, "Top-level Exception ({}):\n", typeid(eErr).name() );
	std::print( stderr, "{}\n", eErr.what() );
	std::print( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::print( stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int mods )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}

		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if( GLFW_PRESS == aAction )
			{
				state->pressedKeys[aKey] = true;
			}
			else if( aAction == GLFW_RELEASE )
			{
				state->pressedKeys[aKey] = false;
			}
		}
	}


	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			if( state->camControl.cameraActive )
			{
				auto const dx = float(aX-state->camControl.lastX);
				auto const dy = float(aY-state->camControl.lastY);

				state->camControl.yaw += dx*kMouseSensitivity_;


				constexpr float maxPitch =  std::numbers::pi_v<float>/2.f;
				constexpr float minPitch = -std::numbers::pi_v<float>/2.f;
				float tempPitch = state->camControl.pitch + dy*kMouseSensitivity_;

				state->camControl.pitch = std::clamp(tempPitch, minPitch, maxPitch);
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}

	void glfw_callback_mouse_button_(GLFWwindow* aWindow, int aButton, int aAction, int )
	{
		auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow ));

		if ( aButton == GLFW_MOUSE_BUTTON_RIGHT && aAction == GLFW_PRESS )
		{
			state->camControl.cameraActive = !state->camControl.cameraActive;

			if( state->camControl.cameraActive )
				glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
			else
				glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		}
	}


	void updateCamera(State_& state)
	{
		State_::CamCtrl_& cam = state.camControl;

		cam.cameraRight = normalize(cross({ 0.f, 1.f, 0.f }, cam.cameraDirection));
		cam.cameraUp = cross(cam.cameraDirection, cam.cameraRight);

		cam.cameraDirection = normalize( {float(cos(cam.yaw)) * float(cos(cam.pitch)),
										  float(sin(cam.pitch)),
										  float(sin(cam.yaw)) * float(cos(cam.pitch))} );

		if (state.camControl.cameraActive)
		{
			if(state.pressedKeys[GLFW_KEY_LEFT_SHIFT])
				state.speedMod = 10;
			else if(state.pressedKeys[GLFW_KEY_LEFT_CONTROL])
				state.speedMod = 0.5;
			else
				state.speedMod = 1;

			float moveDistance = state.speedMod * kMovementPerSecond_ * state.dt;

			if (state.pressedKeys[GLFW_KEY_W])
				state.camControl.cameraPos += state.camControl.cameraDirection * moveDistance;
			if (state.pressedKeys[GLFW_KEY_S])
				state.camControl.cameraPos -= state.camControl.cameraDirection * moveDistance;
			if (state.pressedKeys[GLFW_KEY_A])
				state.camControl.cameraPos -= normalize(cross(state.camControl.cameraDirection, state.camControl.cameraUp)) * moveDistance;
			if (state.pressedKeys[GLFW_KEY_D])
				state.camControl.cameraPos += normalize(cross(state.camControl.cameraDirection, state.camControl.cameraUp)) * moveDistance;
			if (state.pressedKeys[GLFW_KEY_E])
				state.camControl.cameraPos -= state.camControl.cameraUp * moveDistance;
			if (state.pressedKeys[GLFW_KEY_Q])
				state.camControl.cameraPos += state.camControl.cameraUp * moveDistance;
		}
	}

}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}
