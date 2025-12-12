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
#include "Particle.hpp"

#include "PITBFont.hpp"

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

	struct CamCtrl
	{
		bool cameraActive{ false };
		Vec3f cameraPos{ 0.f, 0.f, 0.f };
		Vec3f cameraDirection{ 0.f, 0.f, 1.f };
		Vec3f cameraRight{ 1.f, 0.f, 0.f };
		Vec3f cameraUp{ 0.f, 1.f, 0.f };

		float yaw{ 0.f };
		float pitch{ 0.f };

		float lastX{ 0.f };
		float lastY{ 0.f };
	};

	struct State_
	{
		std::vector<PointLight>* lights;
		std::vector<ShaderProgram*> progs;
		const Vec3f diffuseLight = { 0.729f, 0.808f, 0.92f }; //sky: 0.529f, 0.808f, 0.92f, warm: 0.9f, 0.9f, 0.6f, blue: 0.729f, 0.808f, 0.92f
		Vec3f currentGlobalLight;

		std::vector<KeyFramedFloat>* animatedFloatsPtr;
		float dt;
		float speedMod;
		bool pressedKeys[KEY_COUNT_GLFW] = { false };
		float fbwidth{ 1280 };
		float fbheight{ 720 };

		Vec2f mousePos;
		int mouseStatus;

		size_t selectedCamera_topScreen{ 0 };
		size_t selectedCamera_bottomScreen{ 0 };
		std::vector<CamCtrl*> camControl;

		ObjectInstanceGroup* spaceShipInstPtr;
		ObjectInstanceGroup* landingPadInstPtr;
		ModelObjectGPU* terrainGPU;

		GLsizei numTerrainVerts;
		GLsizei numSpaceShipVerts;
		GLsizei numLandingPadVerts;

		const Transform spaceShipInitialTransform{
			.mPosition{ -32.5f, 0.3f, 2.f },
			.mRotation{ 0.f, 0.f, 0.f },
			.mScale{ 1.f, 1.f, 1.f }
		};
		const Vec3f shipCamLocalOffset{ -6.5f, -3.3f, 0.f };
		const Vec3f shipCamOriginalPos = Vec3f{ 32.5f, -0.3f, -2.f } + (shipCamLocalOffset);

		bool isSplitScreen;

		GLuint terrainVAO{0};
		GLuint shipVAO{0};
		GLuint landingPadVAO{0};

		GLuint lightsUBO{0};

		std::vector<Vec4f>* lightOriginalPositions;

		UIGroup* UI;
		ParticleSource* pSource;

		//Uniform ID vectors
		std::vector<GLuint> prog2UniformIds;
		std::vector<GLuint> progUniformIds;
		std::vector<GLuint> progParticleUniformIds;
	};


	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY );

	void glfw_callback_mouse_button_(GLFWwindow* window, int button, int action, int );

	void updateCamera(State_& state);

	ModelObject create_ship();
	UIGroup createUI( GLFWwindow* aWindow );
	Vec2f convertCursorPos(float x, float y, float width, float height);


	enum eSelectedCamera : size_t
	{
		kFreeCam = 0,
		kGroundCam,
		kShipCam
	};

	void RenderScene( const CamCtrl& aCamCtrl, GLFWwindow* aWindow );
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
	CamCtrl freeCam;
	state.camControl.push_back(&freeCam);
	freeCam.cameraPos = {0.f, 0.f, -10.f};

	CamCtrl groundCam;
	state.camControl.push_back(&groundCam);
	groundCam.cameraPos = { -21.0772552f, -3.f, -1.44215655f };


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
	glClearColor( 0.05f, 0.05f, 0.1f, 0.0f ); //sky: 0.529f, 0.808f, 0.92f, 0.0f
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

	ShaderProgram progParticle({
		{ GL_VERTEX_SHADER, "assets/cw2/particleShader.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/particleShader.frag" }
	});

	ShaderProgram progFont({
		{ GL_VERTEX_SHADER, "assets/cw2/fontShader.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/fontShader.frag" }
	});

	state.progs.push_back(&prog);
	state.progs.push_back(&prog2);
	state.progs.push_back(&progUI);
	state.progs.push_back(&progParticle);
	state.progs.push_back(&progFont);

	//The following is a hackey method to avoid having to call glGetUniformLocation() during the render loop, we call them all now and store the values for later
	std::vector<GLuint> progUIUniformIds;
	progUIUniformIds.push_back(glGetUniformLocation(progUI.programId(), "inColour"));

	std::vector<GLuint> progUniformIds;
	progUniformIds.push_back(glGetUniformLocation(prog.programId(), "uCamPosition"));
	state.progUniformIds = progUniformIds;

	std::vector<GLuint> prog2UniformIds;
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uProjCameraWorld"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uModelTransform"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uNormalTransform"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uLightDir"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uLightDiffuse"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uSceneAmbient"));
	prog2UniformIds.push_back(glGetUniformLocation(prog2.programId(), "uCamPosition"));
	state.prog2UniformIds = prog2UniformIds;

	std::vector<GLuint> progParticleUniformIds;
	progParticleUniformIds.push_back(glGetUniformLocation(progParticle.programId(), "uProjCameraWorld"));
	progParticleUniformIds.push_back(glGetUniformLocation(progParticle.programId(), "uColour"));
	progParticleUniformIds.push_back(glGetUniformLocation(progParticle.programId(), "uOffset"));
	state.progParticleUniformIds = progParticleUniformIds;

	auto last = Clock::now();

#pragma region ModelLoad




	uint32_t terrainLoadFlags = kLoadTextureCoords | kLoadVertexColour;
	ModelObject terrain( "assets/cw2/parlahti.obj", terrainLoadFlags );
	state.numTerrainVerts = static_cast<GLsizei>( terrain.Vertices().size() );

	// Load model into VBOs
	ModelObjectGPU terrainGPU( terrain );
	state.terrainGPU = &terrainGPU;

	// Create VAO
	//GLuint vao = 0;
	glGenVertexArrays( 1, &state.terrainVAO );
	glBindVertexArray( state.terrainVAO );
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
	state.numLandingPadVerts = static_cast<GLsizei>( landingPad.Vertices().size() );

	ObjectInstanceGroup landingPadInstances( landingPadGPU );
	landingPadInstances.CreateInstance( Transform( { .mPosition{-19.f,  -0.97f, 10.f} } ) );
	landingPadInstances.CreateInstance( Transform( { .mPosition{-32.5f, -0.97f, 2.f } } ) ); //og -34.7f, -0.97f, 1.f
	state.landingPadInstPtr = &landingPadInstances;


	//GLuint vaoLandingPad = 0;
	glGenVertexArrays( 1, &state.landingPadVAO );
	glBindVertexArray( state.landingPadVAO );
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
	spaceShipModel.OriginToGeometry();
	state.numSpaceShipVerts = spaceShipModel.Vertices().size();

	// Creaete the vbos for the model object
	ModelObjectGPU spaceShipModelGPU( spaceShipModel );

	// Create an instance of the model object
	// Makes the model object have a position that we can later modify

	ObjectInstanceGroup spaceShipInstances( spaceShipModelGPU );
	state.spaceShipInstPtr = &spaceShipInstances;
	const Transform& spaceShipInitialTransform{state.spaceShipInitialTransform};
	spaceShipInstances.CreateInstance( spaceShipInitialTransform );

	// Create and calculate ship cam directions
	CamCtrl shipCam;
	shipCam.cameraPos = state.shipCamOriginalPos;

	[&] () {
		state.camControl.push_back(&shipCam);
		shipCam.cameraDirection =  normalize(-(spaceShipInstances.GetTransform(0).mPosition) - shipCam.cameraPos);

		shipCam.pitch = asin(shipCam.cameraDirection.y);
		shipCam.yaw = atan2f(shipCam.cameraDirection.z, shipCam.cameraDirection.x);

		shipCam.cameraRight = normalize(cross({ 0.f, 1.f, 0.f }, shipCam.cameraDirection));
		shipCam.cameraUp = cross(shipCam.cameraDirection, shipCam.cameraRight);
	} ();

	//GLuint vaoSpaceShip = 0;
	glGenVertexArrays( 1, &state.shipVAO );
	glBindVertexArray( state.shipVAO );
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
	Vec4f l1InitialTransform = { -33.75f, 0.3f, 2.f, 0.f};
	Vec4f l2InitialTransform = { -32.55f, 0.6f, 2.f, 0.f};
	Vec4f l3InitialTransform = { -31.75f, -0.5f, 2.f, 0.f};
	std::vector<Vec4f> lightOriginalPositions = {l1InitialTransform, l2InitialTransform, l3InitialTransform};
	state.lightOriginalPositions = &lightOriginalPositions;

	//lights: position, colour, intensity
	PointLight l1 = { l1InitialTransform, { 0.8f, 0.77f, 0.72f, 1.f}, {0.15f, 0.f, 0.f} }; //under saucer light
	PointLight l2 = { l2InitialTransform, { 0.988f, 0.1f, 0.1f, 1.f }, {0.1f, 0.f, 0.f} }; //rear light
	PointLight l3 = { l3InitialTransform, { 0.1f, 0.1f, 0.9f, 1.f }, {0.2f, 0.f, 0.f} }; //bottom light 

	std::vector<PointLight> lights(N_LIGHTS);
	lights[0] = l1;
	lights[1] = l2;
	lights[2] = l3;
	state.lights = &lights;

	//GLuint uboLights;
	glGenBuffers(1, &state.lightsUBO );
	glBindBuffer(GL_UNIFORM_BUFFER, state.lightsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight)* N_LIGHTS, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//bind to material shader
	GLuint blockIndex = glGetUniformBlockIndex(prog2.programId(), "LightBlock");
	glUniformBlockBinding(prog2.programId(), blockIndex, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, state.lightsUBO);

	//bind to default shader
	GLuint blockIndexdefault = glGetUniformBlockIndex(prog.programId(), "LightBlock");
	glUniformBlockBinding(prog.programId(), blockIndexdefault, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, state.lightsUBO);

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

			// We are keeping the X and Z rotation key frames even though we don't
			// end up using them because we need to keep the key framed float vector
			// to be 6 values.
			[[maybe_unused]] KeyFramedFloat& spaceShipXRotKF = ret.emplace_back();
			KeyFramedFloat& spaceShipYRotKF = ret.emplace_back();
			[[maybe_unused]] KeyFramedFloat& spaceShipZRotKF = ret.emplace_back();


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
			Vec3f newSpaceShipPosition2  = (spaceShipForward * 9999.f) + newSpaceShipPosition;
			spaceShipXKF.InsertKeyframe({
				newSpaceShipPosition2.x,
				1.f,
				ShapingFunctions::PolynomialEaseOut<6>
				});

			spaceShipYKF.InsertKeyframe({
				newSpaceShipPosition2.y,
				1.f,
				ShapingFunctions::PolynomialEaseOut<6>
				});

			spaceShipZKF.InsertKeyframe({
				newSpaceShipPosition2.z,
				1.f,
				ShapingFunctions::PolynomialEaseOut<6>
				});


			return ret;
		} ();

	state.animatedFloatsPtr = &spaceShipAnimatedFloats;

	//UI initialisation
	PITBFontManager::Get().SetShaderProgram(&progFont);

	UIGroup UI = createUI(window);
	state.UI = &UI;

	PSourceParams source1
	{
		.Colour = {1.f, 1.f, 1.f, 1.f},
		.Velocity = {0.f, 0.f, 0.f},
		.SourceOrigin = { -31.60f, -0.1f, 2.1f }, //-32.65f, 0.5f, 2.f
		.spread = 0.1f,
		.lifeTime = 0.5f,
		.fade = 2.f,
		.maxParticles = 200,
		.spawnRate = 2
	};

	//Particle effect initialisation
	ParticleSource pSource(source1, "assets/cw2/Particle.png");
	pSource.SetRelativePosition(pSource.GetOrigin() - state.spaceShipInitialTransform.mPosition);
	state.pSource = &pSource;

	PITBFontManager& fm = PITBFontManager::Get();

	PITBStyleID style1 = fm.MakeStyle("./assets/cw2/DroidSansMonoDotted.ttf", 0.03f, FonsRGBA(255, 0, 0, 255));
	PITBText& spaceShipHeightText = fm.MakeText(style1, {0.f, 0.f}, "Space ship height:");

	PITBStyleID styleBtnText = fm.MakeStyleDerived(style1, 0.03f, FonsRGBA(0, 0, 0, 255), FONS_ALIGN_CENTER | FONS_ALIGN_TOP);

	UI.getElement(0).SetString(styleBtnText, "Play/Pause");
	UI.getElement(1).SetString(styleBtnText, "Reset");

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
		}

		state.fbheight = fbheight;
		state.fbwidth = fbwidth;

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// Update state

		auto const now = Clock::now();
		state.dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;

		// Update space ship animation first before camera
		Vec3f spaceShipAnimatedPosition{
			spaceShipAnimatedFloats[0].Update(state.dt),
			spaceShipAnimatedFloats[1].Update(state.dt),
			spaceShipAnimatedFloats[2].Update(state.dt)
		};

		Vec3f spaceShipAnimatedRotation{
			spaceShipAnimatedFloats[3].Update(state.dt),
			spaceShipAnimatedFloats[4].Update(state.dt),
			spaceShipAnimatedFloats[5].Update(state.dt)
		};

		// Bind animated values to space ship transform
		Transform& spaceShipTrans = spaceShipInstances.GetTransform(0);
		spaceShipTrans.mPosition = spaceShipAnimatedPosition;
		spaceShipTrans.mRotation = spaceShipAnimatedRotation;

		updateCamera(state);


		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// actual rendering here
		if (!state.isSplitScreen)
		{
			glViewport( 0, 0, fbwidth, fbheight );
			RenderScene( *(state.camControl[state.selectedCamera_topScreen]), window );
		}
		else
		{
			// Render top screen
			glViewport( 0, fbheight/2, int(fbwidth), int(fbheight/2) );
			RenderScene( *(state.camControl[state.selectedCamera_topScreen]), window );

			// Render bottom screen
			glViewport( 0, 0, int(fbwidth), int(fbheight/2) );
			RenderScene( *(state.camControl[state.selectedCamera_bottomScreen]), window );
		}


		//Render UI
		glViewport( 0, 0, fbwidth, fbheight );
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

			Vec4f element_colour = UI.getElement(i).getColour();
			glUniform4fv(progUIUniformIds[0], 1, &element_colour.x);
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(UI.getElement(i).Vertices().size()));
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);


		// Update the text before the font system update
		spaceShipHeightText.SetString("Spaceship height: {0:.2f} meters", spaceShipAnimatedPosition.y * 10.f);

		// Update the font system
		PITBFontManager::Get().Update(fbwidth, fbheight);


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

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int mod )
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
				state->pSource->ToggleActive();
			}

			if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				for ( auto& anim : *(state->animatedFloatsPtr) )
				{
					anim.Stop();
				}
				state->pSource->SetActive(false);
				state->pSource->DeleteParticles();
				
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


			if( GLFW_KEY_C == aKey && aAction == GLFW_PRESS )
			{
				if (GLFW_MOD_SHIFT & mod)
				{
					state->selectedCamera_bottomScreen = (state->selectedCamera_bottomScreen + 1) % 3;
				}
				else
				{
					state->selectedCamera_topScreen = (state->selectedCamera_topScreen + 1) % 3;
				}


			}

			if( GLFW_KEY_V == aKey && aAction == GLFW_PRESS )
			{
				state->isSplitScreen = !state->isSplitScreen;
			}
		}
	}


	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			const bool freeCamSelected = state->selectedCamera_topScreen == kFreeCam ||
										(state->selectedCamera_bottomScreen == kFreeCam && state->isSplitScreen);

			//get mouse position
			int width, height;
			glfwGetWindowSize(aWindow, &width, &height);
			state->mousePos = convertCursorPos(float(aX), float(aY), float(width), float(height));


			if( state->camControl[kFreeCam]->cameraActive && freeCamSelected )
			{
				auto const dx = float(aX-state->camControl[0]->lastX);
				auto const dy = float(aY-state->camControl[0]->lastY);

				state->camControl[kFreeCam]->yaw += dx*kMouseSensitivity_;


				constexpr float maxPitch =  std::numbers::pi_v<float>/2.f;
				constexpr float minPitch = -std::numbers::pi_v<float>/2.f;
				float tempPitch = state->camControl[0]->pitch + dy*kMouseSensitivity_;

				state->camControl[kFreeCam]->pitch = std::clamp(tempPitch, minPitch, maxPitch);
			}

			state->camControl[kFreeCam]->lastX = float(aX);
			state->camControl[kFreeCam]->lastY = float(aY);
		}
	}

	void glfw_callback_mouse_button_(GLFWwindow* aWindow, int aButton, int aAction, int )
	{
		auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow ));

		if ( aButton == GLFW_MOUSE_BUTTON_RIGHT && aAction == GLFW_PRESS )
		{
			state->camControl[0]->cameraActive = !state->camControl[0]->cameraActive;

			if( state->camControl[0]->cameraActive )
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
		const bool freeCamSelected = state.selectedCamera_topScreen == kFreeCam ||
									(state.selectedCamera_bottomScreen == kFreeCam && state.isSplitScreen);

		const bool groundCamSelected = state.selectedCamera_topScreen == kGroundCam ||
									  (state.selectedCamera_bottomScreen == kGroundCam && state.isSplitScreen);

		const bool shipCamSelected = state.selectedCamera_topScreen == kShipCam ||
									(state.selectedCamera_bottomScreen == kShipCam && state.isSplitScreen);


		if( freeCamSelected )
		{
			CamCtrl& cam = *(state.camControl[kFreeCam]);

			cam.cameraRight = normalize(cross({ 0.f, 1.f, 0.f }, cam.cameraDirection));
			cam.cameraUp = cross(cam.cameraDirection, cam.cameraRight);

			cam.cameraDirection = normalize( {float(cos(cam.yaw)) * float(cos(cam.pitch)),
											  float(sin(cam.pitch)),
											  float(sin(cam.yaw)) * float(cos(cam.pitch))} );

			if (cam.cameraActive)
			{
				if(state.pressedKeys[GLFW_KEY_LEFT_SHIFT])
					state.speedMod = 10;
				else if(state.pressedKeys[GLFW_KEY_LEFT_CONTROL])
					state.speedMod = 0.2;
				else
					state.speedMod = 1;

				float moveDistance = state.speedMod * kMovementPerSecond_ * state.dt;

				if (state.pressedKeys[GLFW_KEY_W])
					cam.cameraPos += cam.cameraDirection * moveDistance;
				if (state.pressedKeys[GLFW_KEY_S])
					cam.cameraPos -= cam.cameraDirection * moveDistance;
				if (state.pressedKeys[GLFW_KEY_A])
					cam.cameraPos -= normalize(cross(cam.cameraDirection, cam.cameraUp)) * moveDistance;
				if (state.pressedKeys[GLFW_KEY_D])
					cam.cameraPos += normalize(cross(cam.cameraDirection, cam.cameraUp)) * moveDistance;
				if (state.pressedKeys[GLFW_KEY_E])
					cam.cameraPos -= cam.cameraUp * moveDistance;
				if (state.pressedKeys[GLFW_KEY_Q])
					cam.cameraPos += cam.cameraUp * moveDistance;
			}
		}


		if( groundCamSelected )
		{
			CamCtrl& cam = *(state.camControl[kGroundCam]);

			cam.cameraDirection =  normalize(-(state.spaceShipInstPtr->GetTransform(0).mPosition) - cam.cameraPos);

			cam.pitch = asin(cam.cameraDirection.y);
			cam.yaw = atan2f(cam.cameraDirection.z, cam.cameraDirection.x);

			cam.cameraRight = normalize(cross({ 0.f, 1.f, 0.f }, cam.cameraDirection));
			cam.cameraUp = cross(cam.cameraDirection, cam.cameraRight);
		}


		if( shipCamSelected )
		{
			CamCtrl& cam = *(state.camControl[kShipCam]);

			Transform shipTrans = state.spaceShipInstPtr->GetTransform(0);

			Vec3f spaceShipDiff{ shipTrans.mPosition - state.spaceShipInitialTransform.mPosition };

			cam.cameraPos = (state.shipCamOriginalPos) - spaceShipDiff;
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

	UIGroup createUI( GLFWwindow* aWindow )
	{
		auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow));

		std::vector<UIElement> elements;

		UIElementProperties toggleAnimationBtn_prop
		{
			.uiColour = {0.1f, 0.9f, 0.1f, 0.5f},
			.uiPosition = {-0.2f, -0.9},
			.uiWidth = 0.25f,
			.uiHeight = 0.2f,
			.uiBorderWidth = 0.02f,
		};

		UIElement toggleAnimationBtn = UIElement(toggleAnimationBtn_prop);
		toggleAnimationBtn.InsertOnClickCallback([state] ()
			{
				for (auto& anim : *(state->animatedFloatsPtr))
				{
					anim.Toggle();
				}
				state->pSource->ToggleActive();
			});
		elements.push_back(toggleAnimationBtn);

		UIElementProperties resetAnimationBtn_prop2
		{
			.uiColour = {0.9f, 0.1f, 0.1f, 0.5f},
			.uiPosition = {0.1f, -0.9},
			.uiWidth = 0.25f,
			.uiHeight = 0.2f,
			.uiBorderWidth = 0.02f,
		};

		UIElement resetAnimationBtn = UIElement(resetAnimationBtn_prop2);
		resetAnimationBtn.InsertOnClickCallback([state]()
			{
				for (auto& anim : *(state->animatedFloatsPtr))
				{
					anim.Stop();
				}
				state->pSource->SetActive(false);
				state->pSource->DeleteParticles();
			});
		elements.push_back(resetAnimationBtn);


		return UIGroup(elements);
	}

	void RenderScene( const CamCtrl& aCamCtrl, GLFWwindow* aWindow )
	{
		auto& state = *(static_cast<State_*>(glfwGetWindowUserPointer( aWindow )));


		Mat44f projection;

		if( !state.isSplitScreen )
		{
			projection = make_perspective_projection(
				60.f * std::numbers::pi_v<float> / 180.f,
				state.fbwidth/state.fbheight,
				0.1f, 200.0f
			);
		}
		else
		{
			projection = make_perspective_projection(
				60.f * std::numbers::pi_v<float> / 180.f,
				state.fbwidth/(state.fbheight / 2),
				0.1f, 200.0f
			);
		}


		Mat44f world2Camera = MakeLookAt(aCamCtrl.cameraPos,
										 aCamCtrl.cameraDirection,
										 aCamCtrl.cameraUp,
										 aCamCtrl.cameraRight);

		Mat44f world2CamFlat = MakeBillboardLookAt(aCamCtrl.cameraDirection, aCamCtrl.cameraUp,
			aCamCtrl.cameraRight);


		auto& prog = *(state.progs[0]);
		glUseProgram( prog.programId() );
		GLint locCamPosTerrain = state.progUniformIds[0];
		Mat44f terrainProjectCamWorld = projection * world2Camera * kIdentity44f;
		glUniformMatrix4fv(0, 1, GL_TRUE, terrainProjectCamWorld.v);

		Vec3f lightDir = normalize(Vec3f{ -1.f, 1.f, 0.5f }); // light direction
		glUniform3fv(1, 1, &lightDir.x);
		glUniform3f(2, state.currentGlobalLight[0], state.currentGlobalLight[1], state.currentGlobalLight[2] ); // light diffuse: 0.9f, 0.9f, 0.6f
		glUniform3f(3, 0.05f, 0.05f, 0.05f); // light ambient

		//camera
		glUniform3f(locCamPosTerrain,
					state.camControl[state.selectedCamera_topScreen]->cameraPos[0],
					state.camControl[state.selectedCamera_topScreen]->cameraPos[1],
					state.camControl[state.selectedCamera_topScreen]->cameraPos[2]);

		//lights
		GLuint& uboLights = state.lightsUBO;
		std::vector<PointLight>& lights = *(state.lights);
		glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight) * lights.size(), lights.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//action
		glBindVertexArray( state.terrainVAO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, state.terrainGPU->BufferId(kDiffuseTexture) );
		glDrawArraysInstanced( GL_TRIANGLES, 0, state.numTerrainVerts, 1);

		glBindTexture( GL_TEXTURE_2D, 0 );



		// Render the landing pad
		auto& prog2 = *state.progs[1];
		auto& landingPadInstances = *state.landingPadInstPtr;
		glUseProgram( prog2.programId() );

		GLint locProj        = state.prog2UniformIds[0];
		GLint locModelTrans  = state.prog2UniformIds[1];
		GLint locNormalTrans = state.prog2UniformIds[2];
		GLint locLightDir    = state.prog2UniformIds[3];
		GLint locDiffuse     = state.prog2UniformIds[4];
		GLint locAmbient     = state.prog2UniformIds[5];

		GLint locCamPos = state.prog2UniformIds[6];
		//get camera projection
		std::vector<Mat44f> projectionList = landingPadInstances.GetProjCameraWorldArray(projection, world2Camera);
		glUniformMatrix4fv(locProj, (GLsizei)projectionList.size(), GL_TRUE, projectionList.data()[0].v);
		//get translations
		std::vector<std::array<float, 3>> transformList = landingPadInstances.GetTranslationArray();
		glUniform3fv(locModelTrans, (GLsizei) projectionList.size(), transformList.data()[0].data());
		//get normal updates
		std::vector<Mat33f> normalUpdates = landingPadInstances.GetNormalUpdateArray();
		glUniformMatrix3fv(locNormalTrans, (GLsizei)normalUpdates.size(), GL_TRUE, normalUpdates.data()[0].v);

		glUniform3fv(locLightDir, 1, &lightDir.x);
		glUniform3f(locDiffuse,
					state.currentGlobalLight[0],
					state.currentGlobalLight[1],
					state.currentGlobalLight[2]); // light diffuse
		glUniform3f(locAmbient, 0.05f, 0.05f, 0.05f); // light ambient
		glUniform3f(locCamPos,
					state.camControl[state.selectedCamera_topScreen]->cameraPos[0],
					state.camControl[state.selectedCamera_topScreen]->cameraPos[1],
					state.camControl[state.selectedCamera_topScreen]->cameraPos[2]);

		//point light uniforms
		Vec3f spaceShipAnimatedPosition = state.spaceShipInstPtr->GetTransform(0).mPosition;
		Vec3f spaceShipAnimatedRotation = state.spaceShipInstPtr->GetTransform(0).mRotation;
		Vec4f spaceShipOffset = Vec3ToVec4(spaceShipAnimatedPosition - state.spaceShipInitialTransform.mPosition);
		for(size_t i = 0; i < lights.size(); i++)
		{
			lights[i].lPosition = state.lightOriginalPositions->at(i) + spaceShipOffset;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight)* lights.size(), lights.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindVertexArray( state.landingPadVAO );
		glDrawArraysInstanced( GL_TRIANGLES, 0, state.numLandingPadVerts, landingPadInstances.GetInstanceCount());


		// Spaceship
		std::vector<Mat44f> projectionList2 = state.spaceShipInstPtr->GetProjCameraWorldArray(projection, world2Camera);
		glUniformMatrix4fv(locProj, (GLsizei) projectionList2.size(), GL_TRUE, projectionList2.data()[0].v );
		//get ship translation
		std::vector<std::array<float, 3>> shipTransformList = state.spaceShipInstPtr->GetTranslationArray();
		glUniform3fv(locModelTrans, (GLsizei)projectionList2.size(), shipTransformList.data()[0].data());
		//get normal updates
		std::vector<Mat33f> shipNormalUpdates = state.spaceShipInstPtr->GetNormalUpdateArray();
		glUniformMatrix3fv(locNormalTrans, (GLsizei)shipNormalUpdates.size(), GL_TRUE, shipNormalUpdates.data()[0].v);

		glBindVertexArray( state.shipVAO );
		glDrawArraysInstanced( GL_TRIANGLES, 0, state.numSpaceShipVerts, state.spaceShipInstPtr->GetInstanceCount());

		//Particles
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);
		auto& progParticle = *state.progs[3];
		glUseProgram(progParticle.programId());
		GLint locProjPart = state.progParticleUniformIds[0];
		GLint locColour = state.progParticleUniformIds[1];
		GLint locOffset = state.progParticleUniformIds[2];

		//move source and update particles
		

		Mat44f spaceShipRotMat = make_rotation_y(spaceShipAnimatedRotation.y);
		Vec3f sourcePos = Vec4ToVec3(spaceShipRotMat * Vec3ToVec4(state.pSource->GetRelativePosition())) + spaceShipAnimatedPosition;

		state.pSource->SetPosition(sourcePos);

		//get verticies and texture
		glBindVertexArray(state.pSource->ParticleVAO());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state.pSource->GetTexture());

		state.pSource->UpdateParticles(state.dt);
		std::vector<Particle> particles = state.pSource->GetParticles();
		for (size_t i = 0; i < particles.size(); i++) 
		{
			if (particles[i].life > 0) 
			{
				Mat44f particleProjection = projection * world2Camera * make_translation(particles[i].Position) * world2CamFlat;
				glUniformMatrix4fv(locProjPart, 1, GL_TRUE, particleProjection.v);
				glUniform3fv(locOffset, 1, &particles[i].Position.x);
				glUniform4fv(locColour, 1, &particles[i].Colour.x);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				
			}
			
		}
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}



	Vec2f convertCursorPos(float x, float y, float width, float height)
	{
		Vec2f R;
		R.x = (x / width) - 0.5f;
		R.y = -(y / height) + 0.5f;

		return R * 2.f;
	}
}
