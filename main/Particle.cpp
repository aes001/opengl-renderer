#include "Particle.hpp"
#include <stb_image.h>
#include <random>

ParticleSource::ParticleSource(Vec3f position, int n_particles)
	: mVboVertices(0)
	, mParticleVAO(0)
	, mRandomGenerator(0)
	, mDistribution(-1.f, 1.f)
{

	mSourceOrigin = position;
	for (int i = 0; i < n_particles; i++) 
	{
		mParticles.push_back(Particle());
	}
	
	CreatePositionsVBO();
	CreateParticleVAO();

}

void ParticleSource::UpdateParticles(int newParticles, float spread)
{
	//spawn particles
	int spawned = 0;
	for (int i = 0; i < mParticles.size() && spawned < newParticles; i++) {
		if (mParticles[i].life <= 0) {
			SpawnParticle(i, spread);
			spawned++;
		}
	}


	for (int i = 0; i < mParticles.size(); i++) 
	{
		mParticles[i].life -= 0.01;
	}

}

std::vector<Particle> ParticleSource::GetParticles() 
{
	return mParticles;
}


const GLuint ParticleSource::ParticleVAO() const
{
	return mParticleVAO;
}


const Vec3f ParticleSource::GetOrigin() const
{
	return mSourceOrigin;
}



void ParticleSource::CreatePositionsVBO() 
{
	Vec3f LL = { 0.f, 0.f, 0.f };
	Vec3f UL = { 0.f, 1.f, 0.f };
	Vec3f LR = { 1.f, 0.f, 0.f };
	Vec3f UR = { 1.f, 1.f, 0.f };
	std::vector<Vec3f> pV = {UL, LL, UR, UR, LL, LR};

	glGenBuffers(1, &mVboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, mVboVertices);
	glBufferData(GL_ARRAY_BUFFER, pV.size() * sizeof(Vec3f), pV.data(), GL_STATIC_DRAW);

}

void ParticleSource::CreateParticleVAO() 
{
	glGenVertexArrays(1, &mParticleVAO);
	glBindVertexArray(mParticleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVboVertices);
	glVertexAttribPointer(
		0,
		3, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(0);
}

void ParticleSource::SpawnParticle(int pIndex, float spread)
{
	mParticles[pIndex].life = 1;

	float rndx = mDistribution(mRandomGenerator);
	float rndy = mDistribution(mRandomGenerator);
	float rndz = mDistribution(mRandomGenerator);

	mParticles[pIndex].Position = { spread * rndx, spread * rndy, spread * rndz };
}