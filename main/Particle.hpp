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
		: Position(0.f), life(0.f), Colour({1.f, 1.f, 1.f, 1.f}) {
	}
};

class ParticleSource 
{
public:
	explicit ParticleSource(Vec3f position, int n_particles, std::string tex_path);
	
	void UpdateParticles(int newParticles, float spread, float dt);

	std::vector<Particle> GetParticles();

	const GLuint ParticleVAO() const;

	const GLuint GetTexture() const;

	const Vec3f GetOrigin() const;

	void SetOrigin(Vec3f newOrigin);

private:
	void CreatePositionsVBO();

	void CreateTextureCoordsVBO();

	void CreateParticleVAO();

	void SpawnParticle(int pIndex, float spread);

private:
	std::vector<Particle> mParticles;
	Vec3f mSourceOrigin;

	GLuint mVboVertices;
	GLuint mTextureCoordsVBO;
	GLuint mParticleVAO;
	GLuint mTextureID;

	std::default_random_engine mRandomGenerator;
	std::uniform_real_distribution<float> mDistribution;

};

#endif