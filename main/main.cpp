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
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "ModelObject.hpp"
#include "ShapeObject.hpp"
#include "LookAt.hpp"
#include "AnimationTools.hpp"
#include "GeometricHelpers.hpp"
#include "Light.hpp"
#include "UIObject.hpp"
#include "UIGroup.hpp"

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
		std::vector<PointLight>* lights;
		std::vector<ShaderProgram*> progs;
		const Vec3f diffuseLight = { 0.729f, 0.808f, 0.92f }; //sky: 0.529f, 0.808f, 0.92f, warm: 0.9f, 0.9f, 0.6f
		Vec3f currentGlobalLight;
		std::vector<KeyFramedFloat>* animatedFloatsPtr;
		float dt;
		float speedMod;
		bool pressedKeys[KEY_COUNT_GLFW] = { false };

		Vec2f mousePos;
		int mouseStatus;

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
	UIGroup createUI();
	Vec2f convertCursorPos(float x, float y, float width, float height);
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
	glClearColor( 0.529f, 0.808f, 0.92f, 0.0f );
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

	ShaderProgram progUI({
		{ GL_VERTEX_SHADER, "assets/cw2/uiShader.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/uiShader.frag" }
	});

	state.progs.push_back(&prog);
	state.progs.push_back(&prog2);
	state.progs.push_back(&progUI);
	auto last = Clock::now();

#pragma region ModelLoad




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
	landingPadInstances.CreateInstance( Transform( { .mPosition{-19.f,  -0.97f, 10.f} } ) );
	landingPadInstances.CreateInstance( Transform( { .mPosition{-32.5f, -0.97f, 2.f } } ) ); //og -34.7f, -0.97f, 1.f


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

	//specular reflectance
	glBindBuffer(GL_ARRAY_BUFFER, landingPadGPU.BufferId(kVboVertexSpecular));
	glVertexAttribPointer(
		3,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(3);

	//shininess
	glBindBuffer(GL_ARRAY_BUFFER, landingPadGPU.BufferId(kVboVertexShininess));
	glVertexAttribPointer(
		4,
		1, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(4);

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

	//specular reflectance
	glBindBuffer(GL_ARRAY_BUFFER, spaceShipModelGPU.BufferId(kVboVertexSpecular));
	glVertexAttribPointer(
		3,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(3);

	//shininess
	glBindBuffer(GL_ARRAY_BUFFER, spaceShipModelGPU.BufferId(kVboVertexShininess));
	glVertexAttribPointer(
		4,
		1, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(4);

#pragma endregion

#pragma region LightsInit



	//LIGHTS
	state.currentGlobalLight = state.diffuseLight;

	#define N_LIGHTS 3
	Vec4f l1InitialTransform = { -33.5f, 0.3f, 2.f, 0.f};
	Vec4f l2InitialTransform = { -32.3f, 0.6f, 2.f, 0.f};
	Vec4f l3InitialTransform = { -31.5f, -0.5f, 2.f, 0.f};
	std::vector<Vec4f> lightOriginalPositions = {l1InitialTransform, l2InitialTransform, l3InitialTransform};
	//lights: position, colour, intensity
	PointLight l1 = { l1InitialTransform, { 0.8f, 0.77f, 0.72f, 1.f}, {0.15f, 0.f, 0.f} }; //under saucer light
	PointLight l2 = { l2InitialTransform, { 0.988f, 0.1f, 0.1f, 1.f}, {0.1f, 0.f, 0.f} }; //naecell light
	PointLight l3 = { l3InitialTransform, { 0.1f, 0.1f, 0.9f, 1.f}, {0.2f, 0.f, 0.f} }; //bottom light
	std::vector<PointLight> lights(N_LIGHTS);
	lights[0] = l1;
	lights[1] = l2;
	lights[2] = l3;
	state.lights = &lights;

	GLuint uboLights;
	glGenBuffers(1, &uboLights);
	glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight)* N_LIGHTS, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//bind to material shader
	GLuint blockIndex = glGetUniformBlockIndex(prog2.programId(), "LightBlock");
	glUniformBlockBinding(prog2.programId(), blockIndex, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);

	//bind to default shader
	GLuint blockIndexdefault = glGetUniformBlockIndex(prog.programId(), "LightBlock");
	glUniformBlockBinding(prog.programId(), blockIndexdefault, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);

	// Reset State
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

#pragma endregion

	// Animating
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

	//UI initialisation
	UIGroup UI = createUI();

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

#pragma region RenderModels



		// Rendering the terrain
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( prog.programId() );
		GLint locCamPosTerrain = glGetUniformLocation(prog.programId(), "uCamPosition");
		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);

		Vec3f lightDir = normalize(Vec3f{ -1.f, 1.f, 0.5f }); // light direction
		glUniform3fv(1, 1, &lightDir.x);
		glUniform3f(2, state.currentGlobalLight[0], state.currentGlobalLight[1], state.currentGlobalLight[2] ); // light diffuse: 0.9f, 0.9f, 0.6f
		glUniform3f(3, 0.05f, 0.05f, 0.05f); // light ambient

		//camera
		glUniform3f(locCamPosTerrain, state.camControl.cameraPos[0], state.camControl.cameraPos[1], state.camControl.cameraPos[2]);

		//lights
		glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight) * lights.size(), lights.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//action
		glBindVertexArray( vao );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, terrainGPU.BufferId(kDiffuseTexture) );
		glDrawArraysInstanced( GL_TRIANGLES, 0, numTerrainVerts, 1);

		glBindTexture( GL_TEXTURE_2D, 0 );



		// Render the landing pad
		glUseProgram( prog2.programId() );

		GLint locProj     = glGetUniformLocation( prog2.programId(), "uProjCameraWorld" );
		GLint locModelTrans = glGetUniformLocation(prog2.programId(), "uModelTransform");
		GLint locNormalTrans = glGetUniformLocation(prog2.programId(), "uNormalTransform");
		GLint locLightDir = glGetUniformLocation (prog2.programId(), "uLightDir" );
		GLint locDiffuse  = glGetUniformLocation( prog2.programId(), "uLightDiffuse" );
		GLint locAmbient  = glGetUniformLocation( prog2.programId(), "uSceneAmbient" );

		GLint locCamPos = glGetUniformLocation(prog2.programId(), "uCamPosition");
		//get camera projections
		std::vector<Mat44f> projectionList = landingPadInstances.GetProjCameraWorldArray(projection, world2camera);
		glUniformMatrix4fv(locProj, (GLsizei)projectionList.size(), GL_TRUE, projectionList.data()[0].v);
		//get translations
		std::vector<std::array<float, 3>> transformList = landingPadInstances.GetTranslationArray();
		glUniform3fv(locModelTrans, (GLsizei) projectionList.size(), transformList.data()[0].data());
		//get normal updates
		std::vector<Mat33f> normalUpdates = landingPadInstances.GetNormalUpdateArray();
		glUniformMatrix3fv(locNormalTrans, (GLsizei)normalUpdates.size(), GL_TRUE, normalUpdates.data()[0].v);

		glUniform3fv(locLightDir, 1, &lightDir.x);
		glUniform3f(locDiffuse, state.currentGlobalLight[0], state.currentGlobalLight[1], state.currentGlobalLight[2]); // light diffuse
		glUniform3f(locAmbient, 0.05f, 0.05f, 0.05f); // light ambient
		glUniform3f(locCamPos, state.camControl.cameraPos[0], state.camControl.cameraPos[1], state.camControl.cameraPos[2]);
		//specular light uniforms

		//update light positions with ship animation
		float ssXOffset = spaceShipAnimatedFloats[0].Update(state.dt) - spaceShipInitialTransform.mPosition.x;
		float ssYOffset = spaceShipAnimatedFloats[1].Update(state.dt) - spaceShipInitialTransform.mPosition.y;
		float ssZOffset = spaceShipAnimatedFloats[2].Update(state.dt) - spaceShipInitialTransform.mPosition.z;

		for(size_t i = 0; i < lights.size(); i++)
		{
			Vec4f offsets{ ssXOffset,
						   ssYOffset,
						   ssZOffset,
						   0.f};
			lights[i].lPosition = lightOriginalPositions[i] + offsets;
		}

		//bind lights to uniform buffer
		glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight)* lights.size(), lights.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindVertexArray( vaoLandingPad );
		glDrawArraysInstanced( GL_TRIANGLES, 0, landingPadVertsCount, landingPadInstances.GetInstanceCount());



		// Spaceship
		// We need to use the GetProjCameraWorldArray() from the instance group so we can later animate the spaceship
		Transform& spaceShipTrans = spaceShipInstances.GetTransform(0);

		spaceShipTrans.mPosition.x = spaceShipAnimatedFloats[0].Update(state.dt);
		spaceShipTrans.mPosition.y = spaceShipAnimatedFloats[1].Update(state.dt);
		spaceShipTrans.mPosition.z = spaceShipAnimatedFloats[2].Update(state.dt);

		spaceShipTrans.mRotation.x = spaceShipAnimatedFloats[3].Update(state.dt);
		spaceShipTrans.mRotation.y = spaceShipAnimatedFloats[4].Update(state.dt);
		spaceShipTrans.mRotation.z = spaceShipAnimatedFloats[5].Update(state.dt);



		std::vector<Mat44f> projectionList2 = spaceShipInstances.GetProjCameraWorldArray(projection, world2camera);
		glUniformMatrix4fv(locProj, (GLsizei) projectionList2.size(), GL_TRUE, projectionList2.data()[0].v );
		//get ship translation
		std::vector<std::array<float, 3>> shipTransformList = spaceShipInstances.GetTranslationArray();
		glUniform3fv(locModelTrans, (GLsizei)projectionList2.size(), shipTransformList.data()[0].data());
		//get normal updates
		std::vector<Mat33f> shipNormalUpdates = spaceShipInstances.GetNormalUpdateArray();
		glUniformMatrix3fv(locNormalTrans, (GLsizei)shipNormalUpdates.size(), GL_TRUE, shipNormalUpdates.data()[0].v);

		glBindVertexArray( vaoSpaceShip );
		glDrawArraysInstanced( GL_TRIANGLES, 0, spaceShipVertsCount, spaceShipInstances.GetInstanceCount());

		// Cleanup
		glBindVertexArray( 0 );
		glUseProgram( 0 );
#pragma endregion

		//Render UI
		glDisable(GL_DEPTH_TEST);

		glUseProgram(progUI.programId());
		//give projection matrix to uniform

		glEnable(GL_BLEND); //enter blending mode to use transparency
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		UI.checkMouseInterractions(state.mousePos, state.mouseStatus);
		//draw each UI element
		for (int i = 0; i < UI.getElementCount(); i++) 
		{
			glBindVertexArray(UI.getElementGPU(i).ArrayId());

			GLint Colour = glGetUniformLocation(progUI.programId(), "inColour");
			Vec4f element_colour = UI.getElement(i).getColour();
			glUniform4fv(Colour, 1, &element_colour.x);
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(UI.getElement(i).Vertices().size()));

		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);


		// Cleanup
		glBindVertexArray(0);
		glUseProgram(0);


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

			//key actions
			if (GLFW_KEY_1 == aKey && GLFW_PRESS == aAction)
				(state->lights)->at(0).lColour.w = (state->lights)->at(0).lColour.w == 1.f ? 0.f : 1.f; //toggle light on/off
			if (GLFW_KEY_2 == aKey && GLFW_PRESS == aAction)
				(state->lights)->at(1).lColour.w = (state->lights)->at(1).lColour.w == 1.f ? 0.f : 1.f;
			if (GLFW_KEY_3 == aKey && GLFW_PRESS == aAction)
				(state->lights)->at(2).lColour.w = (state->lights)->at(2).lColour.w == 1.f ? 0.f : 1.f;
			if (GLFW_KEY_4 == aKey && GLFW_PRESS == aAction)
				state->currentGlobalLight = state->currentGlobalLight == Vec3f{0.f, 0.f, 0.f} ? state->diffuseLight : Vec3f{0.f, 0.f, 0.f};
		}
	}


	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			//get mouse position
			int width, height;
			glfwGetWindowSize(aWindow, &width, &height);
			state->mousePos = convertCursorPos(float(aX), float(aY), float(width), float(height));

			//apply camera changes
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
		if (aButton == GLFW_MOUSE_BUTTON_LEFT) 
		{
			state->mouseStatus = aAction;
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
				state.speedMod = 0.2;
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
		ShapeMaterial hullPlating
		{
			.mVertexColor = {0.4f, 0.4f, 0.4f},
			.mSpecular = {0.846f, 0.846f, 0.846f},
			.mShininess = 50.f
		};

		ShapeMaterial radarDish
		{
			.mVertexColor = {0.722f, 0.451f, 0.20f},
			.mSpecular = {0.946f, 0.846f, 0.846f},
			.mShininess = 300.f
		};

		Vec3f base_colour = { 0.7f, 0.7f, 0.7f };
		Vec3f red = { 0.8f, 0.1f, 0.1f };

		Transform bodyTransform{
			.mPosition{1.6f, 0.9f, 1.f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{1.7f, 0.2f, 0.2f}
		};
		ModelObject body = MakeCylinder(true, 32, bodyTransform, hullPlating);

		Transform leftNacelTransform{
			.mPosition{2.5f, 1.6f, 0.2f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{2.2f, 0.1f, 0.1f}
		};
		ModelObject leftNacel = MakeCylinder(true, 32, leftNacelTransform, hullPlating);

		Transform rightNacelTransform{
			.mPosition{2.5f, 1.6f, 1.8f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{2.2f, 0.1f, 0.1f}
		};
		ModelObject rightNacel = MakeCylinder(true, 32, rightNacelTransform, hullPlating);

		Transform saucerTransform{
			.mPosition{1.2f, 1.5f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> / 2},
			.mScale{0.1f, 1.f, 1.f}
		};
		ModelObject saucerSection = MakeCylinder(true, 32, saucerTransform, hullPlating);

		Transform leftArmTransform{
			.mPosition{3.f, 1.2f, 1.4f},
			.mRotation{0.8f, 0.f, 0.f},
			.mScale{0.1f, 0.5f, 0.05f}
		};
		ModelObject leftArm = MakeCube(leftArmTransform, hullPlating);

		Transform rightArmTransform{
			.mPosition{3.f, 1.2f, 0.6f},
			.mRotation{-0.8f, 0.f, 0.f},
			.mScale{0.1f, 0.5f, 0.05f}
		};
		ModelObject rightArm = MakeCube(rightArmTransform, hullPlating);

		Transform neckTransform{
			.mPosition{1.8f, 1.2f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> *0.2f},
			.mScale{0.15f, 0.4f, 0.075f}
		};
		ModelObject neck = MakeCube(neckTransform, hullPlating);

		Transform topSaucerTransform{
			.mPosition{1.2f, 1.6f, 1.f},
			.mRotation{0.f, 0.f, std::numbers::pi_v<float> / 2},
			.mScale{0.2f, 0.75f, 0.75f}
		};

		ModelObject topSaucer = MakeCone(false, 32, topSaucerTransform, hullPlating);

		Transform bottomSaucerTransform{
			.mPosition{1.2f, 1.5f, 1.f},
			.mRotation{0.f, 0.f, -std::numbers::pi_v<float> / 2},
			.mScale{0.2f, 0.5f, 0.5f}
		};
		ModelObject bottomSaucer = MakeCone(false, 32, bottomSaucerTransform, hullPlating);

		Transform rearDishTransform{
			.mPosition{1.5f, 0.9f, 1.f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{0.1f, 0.19f, 0.19f}
		};
		ModelObject rearDish = MakeCone(false, 32, rearDishTransform, radarDish);

		Transform frontDishTransform{
			.mPosition{1.5f, 0.9f, 1.f},
			.mRotation{0.f, std::numbers::pi_v<float>, 0.f},
			.mScale{-0.1f, 0.19f, 0.19f}
		};
		ModelObject frontDish = MakeCone(false, 32, frontDishTransform, radarDish);

		Transform radarAntennaTransform{
			.mPosition{1.5f, 0.9f, 1.f},
			.mRotation{0.f, 0.f, 0.f},
			.mScale{0.075f, 0.01f, 0.01f}
		};
		ModelObject radarAntenna = MakeCylinder(true, 32, radarAntennaTransform, radarDish);

		// Combine the two model objects
		ModelObject combined = CombineShapeModelObjects(body, saucerSection, topSaucer, bottomSaucer, leftNacel, rightNacel, neck, leftArm, rightArm, rearDish, frontDish, radarAntenna);

		return combined;
	}

	UIGroup createUI() 
	{	
		std::vector<UIElement> elements;

		UIElementProperties test_prop
		{
			.uiColour = {0.722f, 0.151f, 0.1f, 0.5f},
			.uiPosition = {0.7f, -0.9},
			.uiWidth = 0.2f,
			.uiHeight = 0.2f,
			.uiBorderWidth = 0.01f
		};

		UIElement test_element = UIElement(test_prop);
		elements.push_back(test_element);

		UIElementProperties test_prop2
		{
			.uiColour = {0.22f, 0.151f, 0.9f, 0.5f},
			.uiPosition = {0.45f, -0.9},
			.uiWidth = 0.2f,
			.uiHeight = 0.2f,
			.uiBorderWidth = 0.05f
		};

		UIElement test_element2 = UIElement(test_prop2);
		elements.push_back(test_element2);

		UIElementProperties test_prop3
		{
			.uiColour = {1.f, 0.f, 0.f, 0.5f},
			.uiPosition = {0.5f, 0.5f},
			.uiWidth = 0.2f,
			.uiHeight = 0.2f,
			.uiBorderWidth = 0.f
		};

		UIElement test_element3(test_prop3);
		elements.push_back(test_element3);

		return UIGroup(elements);
	}

	Vec2f convertCursorPos(float x, float y, float width, float height) 
	{
		Vec2f R;
		R.x = (x / width) - 0.5f;
		R.y = -(y / height) + 0.5f;

		return R * 2.f;
	}
}
