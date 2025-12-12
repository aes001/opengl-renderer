#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../vmlib/vec4.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec2.hpp"

#include "glad/glad.h"
#include <vector>
#include <random>

struct Particle 
{
	Vec3f Position;
	Vec4f Colour;
	float life;

	Particle()
		: Position(0.f), Colour({1.f, 1.f, 1.f, 1.f}), life(0.f) {
	}
};

struct PSourceParams 
{
	Vec4f Colour;
	Vec3f Velocity;
	Vec3f SourceOrigin;
	float spread;
	float lifeTime;
	float fade;
	int maxParticles;
	int spawnRate;
};

class ParticleSource 
{
public:
	explicit ParticleSource(PSourceParams params, std::string tex_path);
	
	void UpdateParticles(float dt);

	std::vector<Particle> GetParticles();

	void DeleteParticles();

	//getters and setters
	const GLuint ParticleVAO() const;

	const GLuint GetTexture() const;

	const Vec3f GetOrigin() const;

	const Vec3f GetPosition() const;

	const Vec3f GetRelativePosition() const;

	void SetRelativePosition(Vec3f relPos);

	void SetPosition(Vec3f newPosition);

	void ToggleActive();

	void SetActive(bool active);

private:
	void CreatePositionsVBO();

	void CreateTextureCoordsVBO();

	void CreateParticleVAO();

	void SpawnParticle(int pIndex, float spread);


private:
	std::vector<Particle> mParticles;
	Vec3f mSourceOrigin;
	Vec3f mSourcePosition;
	Vec3f mRelativePositionToParent; //whatever parent you decide (in this case ship)

	GLuint mVboVertices;
	GLuint mTextureCoordsVBO;
	GLuint mParticleVAO;
	GLuint mTextureID;

	std::default_random_engine mRandomGenerator;
	std::uniform_real_distribution<float> mDistribution;

	PSourceParams mParams;
	bool mActive;
};

#endif