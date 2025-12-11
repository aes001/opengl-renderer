#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../vmlib/vec3.hpp"

#include "glad/glad.h"
#include <vector>
#include <random>

struct Particle 
{
	Vec3f Position;
	float life;

	Particle()
		: Position(0.f), life(0.f) {}
};

class ParticleSource 
{
public:
	explicit ParticleSource(Vec3f position, int n_particles);
	
	void UpdateParticles(int newParticles, float spread);

	std::vector<Particle> GetParticles();

	const GLuint ParticleVAO() const;

	const Vec3f GetOrigin() const;

private:
	void CreatePositionsVBO();

	void CreateParticleVAO();

	void SpawnParticle(int pIndex, float spread);

private:
	std::vector<Particle> mParticles;
	Vec3f mSourceOrigin;

	GLuint mVboVertices;
	GLuint mParticleVAO;

	std::default_random_engine mRandomGenerator;
	std::uniform_real_distribution<float> mDistribution;

};

#endif