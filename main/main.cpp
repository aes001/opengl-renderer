#if defined(_WIN32) // alternative: ”#if defined(_MSC_VER)”
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1; // untested
	// See https://stackoverflow.com/questions/17458803/amd-equivalent-to-nvoptimusenablement
}
#endif

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
#include "ShapeObject.hpp"
#include "LookAt.hpp"
#include "AnimationTools.hpp"
#include "GeometricHelpers.hpp"


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
		std::vector<ShaderProgram*> progs;
		std::vector<KeyFramedFloat>* animatedFloatsPtr;
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

	ModelObject create_ship();
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

	ShaderProgram prog2( {
		{GL_VERTEX_SHADER, "assets/cw2/materialColour.vert"},
		{GL_FRAGMENT_SHADER, "assets/cw2/materialColour.frag"}
	} );

	state.progs.push_back(&prog);
	state.progs.push_back(&prog2);
	auto last = Clock::now();


	uint32_t terrainLoadFlags = kLoadTextureCoords | kLoadVertexColour;
	ModelObject terrain( "assets/cw2/parlahti.obj", terrainLoadFlags );
	std::vector<Vec3f>& terrainVerts = terrain.Vertices();
	const GLsizei numTerrainVerts = static_cast<GLsizei>( terrainVerts.size() );

	// Load model into VBOs
	ModelObjectGPU terrainGPU( terrain );

	// Create VAO
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	//positions
	glBindBuffer( GL_ARRAY_BUFFER, terrainGPU.BufferId(kVboPositions) );
	glVertexAttribPointer(
		0,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 0 );
	//colours
	glBindBuffer( GL_ARRAY_BUFFER, terrainGPU.BufferId(kVboVertexColor) );
	glVertexAttribPointer(
		1,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 1 );
	//normals
	glBindBuffer(GL_ARRAY_BUFFER, terrainGPU.BufferId(kVboNormals));
	glVertexAttribPointer(
		2,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray(2);

	//Texture
	glBindBuffer(GL_ARRAY_BUFFER, terrainGPU.BufferId(kVboTextureCoords));
	glVertexAttribPointer(
		3,
		2, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(3);


	// Second Model
	uint32_t landingPadLoadFlags = kLoadVertexColour
								 | kLoadVertexAmbient
								 | kLoadVertexSpecular
								 | kLoadVertexShininess;
	ModelObject landingPad( "assets/cw2/landingpad.obj", landingPadLoadFlags );
	ModelObjectGPU landingPadGPU( landingPad );
	const GLsizei landingPadVertsCount = static_cast<GLsizei>( landingPad.Vertices().size() );

	ObjectInstanceGroup landingPadInstances( landingPadGPU );
	landingPadInstances.CreateInstance( { .mPosition{-19.f,  -0.97f, 10.f} } ); // Near spawn
	landingPadInstances.CreateInstance( { .mPosition{-34.7f, -0.97f, 1.f } } ); // Bay


	GLuint vaoLandingPad = 0;
	glGenVertexArrays( 1, &vaoLandingPad );
	glBindVertexArray( vaoLandingPad );
	//positions
	glBindBuffer( GL_ARRAY_BUFFER, landingPadGPU.BufferId(kVboPositions) );
	glVertexAttribPointer(
		0,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 0 );
	//colours
	glBindBuffer( GL_ARRAY_BUFFER, landingPadGPU.BufferId(kVboVertexColor) );
	glVertexAttribPointer(
		1,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);

	glEnableVertexAttribArray( 1 );
	//normals
	glBindBuffer(GL_ARRAY_BUFFER, landingPadGPU.BufferId(kVboNormals));
	glVertexAttribPointer(
		2,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);


	glEnableVertexAttribArray(2);


	// Space Ship

	// Combine the two model objects
	ModelObject spaceShipModel = create_ship();
	const GLsizei spaceShipVertsCount = spaceShipModel.Vertices().size();

	// Creaete the vbos for the model object
	ModelObjectGPU spaceShipModelGPU( spaceShipModel );

	// Create an instance of the model object
	// Makes the model object have a position that we can later modify

	Transform spaceShipInitialTransform{
		.mPosition{ -34.7f, -0.97f, 1.f },
		.mRotation{ 0.f, 0.f, 0.f },
		.mScale{ 1.f, 1.f, 1.f }
	};
	ObjectInstanceGroup spaceShipInstances( spaceShipModelGPU );
	spaceShipInstances.CreateInstance( spaceShipInitialTransform );

	// Space ship animation
	std::vector<KeyFramedFloat> spaceShipAnimatedFloats =
		[&] ()
		{
			std::vector<KeyFramedFloat> ret;
			// Need to do this otherwise the references are invalid because
			// vector gets resized after emplace_back();
			ret.reserve(6);

			KeyFramedFloat& spaceShipXKF = ret.emplace_back();
			KeyFramedFloat& spaceShipYKF = ret.emplace_back();
			KeyFramedFloat& spaceShipZKF = ret.emplace_back();

			KeyFramedFloat& spaceShipXRotKF = ret.emplace_back();
			KeyFramedFloat& spaceShipYRotKF = ret.emplace_back();
			KeyFramedFloat& spaceShipZRotKF = ret.emplace_back();


			// Initial Transforms
			spaceShipXKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.x,
				0.f,
				ShapingFunctions::None // First shaping function is unused
			});

			spaceShipYKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.y,
				0.f,
				ShapingFunctions::None // First shaping function is unused
			});

			spaceShipZKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.z,
				0.f,
				ShapingFunctions::None // First shaping function is unused
			});

			spaceShipYRotKF.InsertKeyframe({
				spaceShipInitialTransform.mRotation.y,
				0.f,
				ShapingFunctions::None
			});


			// Go Up
			spaceShipXKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.x,
				7.f,
				ShapingFunctions::Smoothstep
				});

			float newSpaceShipY = spaceShipInitialTransform.mPosition.y + 30.f;
			spaceShipYKF.InsertKeyframe({
				newSpaceShipY,
				7.f,
				ShapingFunctions::Smoothstep
			});

			spaceShipZKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.z,
				7.f,
				ShapingFunctions::Smoothstep
			});

			float newSpaceShipRotY = spaceShipInitialTransform.mRotation.y + 100.0_deg;
			spaceShipYRotKF.InsertKeyframe({
				newSpaceShipRotY,
				7.f,
				ShapingFunctions::PolynomialEaseOut<4>
			});


			// Wait
			spaceShipXKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.x,
				0.1f,
				ShapingFunctions::None
			});

			spaceShipYKF.InsertKeyframe({
				newSpaceShipY,
				0.1f,
				ShapingFunctions::None
			});

			spaceShipZKF.InsertKeyframe({
				spaceShipInitialTransform.mPosition.z,
				0.1f,
				ShapingFunctions::None
			});


			// Warp Calculations
			// Transform the whole ship
			Vec3f spaceShipPositionAfterLiftOff{
				spaceShipInitialTransform.mPosition.x,
				newSpaceShipY,
				spaceShipInitialTransform.mPosition.z
			};

			Vec3f spaceShipForward{
				cosf(spaceShipInitialTransform.mRotation.x) * sinf(newSpaceShipRotY),
				sinf(spaceShipInitialTransform.mRotation.x),
				cosf(spaceShipInitialTransform.mRotation.x) * cosf(newSpaceShipRotY)
			};

			spaceShipForward = Vec4ToVec3(make_rotation_y(-90.0_deg) * Vec3ToVec4(normalize(spaceShipForward)));

			Vec3f newSpaceShipPosition  = (spaceShipForward * 1000.f) + spaceShipPositionAfterLiftOff;


			// Warp
			spaceShipXKF.InsertKeyframe({
				newSpaceShipPosition.x,
				3.f,
				ShapingFunctions::Polynomial<6>
			});

			spaceShipYKF.InsertKeyframe({
				newSpaceShipPosition.y,
				3.f,
				ShapingFunctions::Polynomial<6>
			});

			spaceShipZKF.InsertKeyframe({
				newSpaceShipPosition.z,
				3.f,
				ShapingFunctions::Polynomial<6>
			});


			// Disappear
			spaceShipXKF.InsertKeyframe({
				newSpaceShipPosition.x + 9999.f,
				0.f,
				ShapingFunctions::Instant
			});

			spaceShipYKF.InsertKeyframe({
				newSpaceShipPosition.y + 9999.f,
				0.f,
				ShapingFunctions::Instant
			});

			spaceShipZKF.InsertKeyframe({
				newSpaceShipPosition.z + 9999.f,
				0.f,
				ShapingFunctions::Instant
			});


			return ret;
		} ();

	state.animatedFloatsPtr = &spaceShipAnimatedFloats;




	GLuint vaoSpaceShip = 0;
	glGenVertexArrays( 1, &vaoSpaceShip );
	glBindVertexArray( vaoSpaceShip );
	//positions
	glBindBuffer( GL_ARRAY_BUFFER, spaceShipModelGPU.BufferId(kVboPositions) );
	glVertexAttribPointer(
		0,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray( 0 );

	//colours
	glBindBuffer( GL_ARRAY_BUFFER, spaceShipModelGPU.BufferId(kVboVertexColor) );
	glVertexAttribPointer(
		1,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray( 1 );

	//normals
	glBindBuffer(GL_ARRAY_BUFFER, spaceShipModelGPU.BufferId(kVboNormals));
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

		Mat44f world2camera = view;
		Mat44f projection = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			fbwidth/float(fbheight),
			0.1f, 200.0f
		);

		Mat44f projCameraWorld = projection * world2camera * model2world;


		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// Rendering the terrain
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( prog.programId() );

		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);

		Vec3f lightDir = normalize(Vec3f{ -1.f, 1.f, 0.5f }); // light direction
		glUniform3fv(1, 1, &lightDir.x);
		glUniform3f(2, 0.9f, 0.9f, 0.6f); // light diffuse
		glUniform3f(3, 0.05f, 0.05f, 0.05f); // light ambient

		glBindVertexArray( vao );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, terrainGPU.BufferId(kDiffuseTexture) );
		glDrawArraysInstanced( GL_TRIANGLES, 0, numTerrainVerts, 1);

		glBindTexture( GL_TEXTURE_2D, 0 );



		// Render the landing pad
		glUseProgram( prog2.programId() );

		GLint locProj     = glGetUniformLocation( prog2.programId(), "uProjCameraWorld" );
		GLint locLightDir = glGetUniformLocation (prog2.programId(), "uLightDir" );
		GLint locDiffuse  = glGetUniformLocation( prog2.programId(), "uLightDiffuse" );
		GLint locAmbient  = glGetUniformLocation( prog2.programId(), "uSceneAmbient" );

		std::vector<Mat44f> projectionList = landingPadInstances.GetProjCameraWorldArray(projection, world2camera);
		glUniformMatrix4fv(locProj, (GLsizei) projectionList.size(), GL_TRUE, projectionList.data()[0].v );
		glUniform3fv(locLightDir, 1, &lightDir.x);
		glUniform3f(locDiffuse, 0.9f, 0.9f, 0.6f); // light diffuse
		glUniform3f(locAmbient, 0.05f, 0.05f, 0.05f); // light ambient
		glBindVertexArray( vaoLandingPad );
		glDrawArraysInstanced( GL_TRIANGLES, 0, landingPadVertsCount, landingPadInstances.GetInstanceCount());



		// Spaceship
		// We need to use the GetProjCameraWorldArray() from the instance group so we can later animate the spaceship
		Transform& spaceShipOTrans = spaceShipInstances.GetTransform(0);

		spaceShipOTrans.mPosition.x = spaceShipAnimatedFloats[0].Update(state.dt);
		spaceShipOTrans.mPosition.y = spaceShipAnimatedFloats[1].Update(state.dt);
		spaceShipOTrans.mPosition.z = spaceShipAnimatedFloats[2].Update(state.dt);

		spaceShipOTrans.mRotation.x = spaceShipAnimatedFloats[3].Update(state.dt);
		spaceShipOTrans.mRotation.y = spaceShipAnimatedFloats[4].Update(state.dt);
		spaceShipOTrans.mRotation.z = spaceShipAnimatedFloats[5].Update(state.dt);

		std::vector<Mat44f> projectionList2 = spaceShipInstances.GetProjCameraWorldArray(projection, world2camera);
		glUniformMatrix4fv(locProj, (GLsizei) projectionList2.size(), GL_TRUE, projectionList2.data()[0].v );
		glBindVertexArray( vaoSpaceShip );
		glDrawArraysInstanced( GL_TRIANGLES, 0, spaceShipVertsCount, spaceShipInstances.GetInstanceCount());


		// Cleanup
		glBindVertexArray( 0 );
		glUseProgram( 0 );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	for( auto& prog : state.progs )
	{
		prog = nullptr;
	}

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

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
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

			if( GLFW_KEY_F == aKey && GLFW_PRESS == aAction )
			{
				for ( auto& anim : *(state->animatedFloatsPtr) )
				{
					anim.Toggle();
				}
			}

			if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				for ( auto& anim : *(state->animatedFloatsPtr) )
				{
					anim.Stop();
				}
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

namespace
{
	ModelObject create_ship()
	{
		Vec3f base_colour = { 0.7f, 0.7f, 0.7f };
		Vec3f red = { 0.8f, 0.1f, 0.1f };

		// Create a transform for a cube
		/*Transform cubeTransform{
			.mPosition{5.f, 0.f, 3.f},
			.mRotation{0.785398f, 0.f, 0.f},
			.mScale{0.5f, 0.5f, 0.5f}
		};
		ModelObject cubeTest = MakeCube({ 0.8f, 0.8f, 0.8f }, cubeTransform);

		// Create a transform for a cylinder
		Transform cylinderTransform{
			.mPosition{5.f, 7.f, 3.f},
			.mRotation{2.f, 0.f, 0.f},
			.mScale{2.f, 2.f, 2.f}
		};
		ModelObject cylinderTest = MakeCylinder(true, 10, { 0.7f, 0.7f, 0.7f }, cylinderTransform);
		*/

		Transform bodyTransform{
			.mPosition{1.6f, 0.9f, 1.f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{1.7f, 0.2f, 0.2f}
		};
		ModelObject body = MakeCylinder(true, 32, base_colour, bodyTransform);

		Transform leftNacelTransform{
			.mPosition{2.5f, 1.6f, 0.2f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{2.2f, 0.1f, 0.1f}
		};
		ModelObject leftNacel = MakeCylinder(true, 32, base_colour, leftNacelTransform);

		Transform rightNacelTransform{
			.mPosition{2.5f, 1.6f, 1.8f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{2.2f, 0.1f, 0.1f}
		};
		ModelObject rightNacel = MakeCylinder(true, 16, base_colour, rightNacelTransform);

		Transform saucerTransform{
			.mPosition{1.f, 1.5f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> / 2},
			.mScale{0.1f, 1.f, 1.f}
		};
		ModelObject saucerSection = MakeCylinder(true, 32, base_colour, saucerTransform);

		Transform leftArmTransform{
			.mPosition{3.f, 1.2f, 1.4f},
			.mRotation{0.8f, 0.f, 0.f},
			.mScale{0.1f, 0.5f, 0.05f}
		};
		ModelObject leftArm = MakeCube(base_colour, leftArmTransform);

		Transform rightArmTransform{
			.mPosition{3.f, 1.2f, 0.6f},
			.mRotation{-0.8f, 0.f, 0.f},
			.mScale{0.1f, 0.5f, 0.05f}
		};
		ModelObject rightArm = MakeCube(base_colour, rightArmTransform);

		Transform neckTransform{
			.mPosition{1.6f, 1.2f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> * 0.2f},
			.mScale{0.15f, 0.4f, 0.075f}
		};
		ModelObject neck = MakeCube(base_colour, neckTransform);

		Transform topSaucerTransform{
			.mPosition{1.f, 1.6f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> / 2},
			.mScale{0.2f, 0.75f, 0.75f}
		};

		ModelObject topSaucer = MakeCone(false, 32, base_colour, topSaucerTransform);

		Transform bottomSaucerTransform{
			.mPosition{1.f, 1.5f, 1.f},
			.mRotation{0.f, 0.f, -std::numbers::pi_v<float> / 2},
			.mScale{0.2f, 0.5f, 0.5f}
		};
		ModelObject bottomSaucer = MakeCone(false, 32, base_colour, bottomSaucerTransform);

		// Combine the two model objects
		ModelObject combined = CombineShapeModelObjects(body, saucerSection, topSaucer, bottomSaucer, leftNacel, rightNacel, neck, leftArm, rightArm);

		return combined;
	}


}
