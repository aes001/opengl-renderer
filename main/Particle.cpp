#include "Particle.hpp"
#include "ModelObject.hpp"
#include <stb_image.h>
#include <random>
#include <string>

ParticleSource::ParticleSource(PSourceParams params, std::string tex_path)
	: mVboVertices(0)
	, mParticleVAO(0)
	, mRandomGenerator(0)
	, mDistribution(-1.f, 1.f)
{
	mParams = params;
	mActive = false;
	mSourceOrigin = params.SourceOrigin;
	mSourcePosition = params.SourceOrigin;
	for (int i = 0; i < params.maxParticles; i++) 
	{
		mParticles.push_back(Particle());
	}
	
	mTextureID = LoadTexture2D(tex_path.c_str());
	CreatePositionsVBO();
	CreateTextureCoordsVBO();
	CreateParticleVAO();

}

void ParticleSource::UpdateParticles(float dt)
{
	//spawn particles
	int spawned = 0;

	if (mActive)
	{
		for (int i = 0; i < mParticles.size() && spawned < mParams.spawnRate; i++) {

			for (int j = 0; j < mParticles.size(); j++) {

				if (mParticles[j].life <= 0) {
					SpawnParticle(j, mParams.spread);
					spawned++;
					break;
				}
			}

		}
	
	

		//update living particles
		for (int i = 0; i < mParticles.size(); i++) 
		{
			mParticles[i].life -= dt;
			if (mParticles[i].life > 0) 
			{
				mParticles[i].Position -= dt * mParams.Velocity;
				mParticles[i].Colour.w -= dt * mParams.fade;
			}
		
		}
	}

}

std::vector<Particle> ParticleSource::GetParticles() 
{
	return mParticles;
}

void ParticleSource::DeleteParticles()
{
	for (int i = 0; i < mParticles.size(); i++) 
	{
		mParticles[i].life = 0;
	}
}

//getters and setters

const GLuint ParticleSource::ParticleVAO() const
{
	return mParticleVAO;
}

const GLuint ParticleSource::GetTexture() const
{
	return mTextureID;
}

const Vec3f ParticleSource::GetOrigin() const
{
	return mSourceOrigin;
}

void  ParticleSource::SetPosition(Vec3f newPosition)
{
	if (newPosition == mSourcePosition) 
	{
		return;
	}
	mParams.Velocity = newPosition - mSourcePosition;
	mSourcePosition = newPosition;
}

const Vec3f ParticleSource::GetPosition() const
{
	return mSourcePosition;
}

const Vec3f ParticleSource::GetRelativePosition() const
{
	return mRelativePositionToParent;
}

void ParticleSource::SetRelativePosition(Vec3f relPos)
{
	mRelativePositionToParent = relPos;
}

void ParticleSource::ToggleActive()
{
	mActive = !mActive;
}

void ParticleSource::SetActive(bool active) 
{
	mActive = active;
}


//private functions
void ParticleSource::CreateTextureCoordsVBO() 
{
	Vec2f LL = { 0.f, 0.f};
	Vec2f UL = { 0.f, 1.f};
	Vec2f LR = { 1.f, 0.f};
	Vec2f UR = { 1.f, 1.f};
	std::vector<Vec2f> tV = { UL, LL, UR, UR, LL, LR };

	glGenBuffers(1, &mTextureCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, tV.size() * sizeof(Vec2f), tV.data(), GL_STATIC_DRAW);

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

	glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordsVBO);
	glVertexAttribPointer(
		1,
		2, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(1);
}

void ParticleSource::SpawnParticle(int pIndex, float spread)
{
	mParticles[pIndex].life = mParams.lifeTime;
	mParticles[pIndex].Colour = mParams.Colour;

	float rndx = mDistribution(mRandomGenerator);
	float rndy = mDistribution(mRandomGenerator);
	float rndz = mDistribution(mRandomGenerator);

	mParticles[pIndex].Position = Vec3f{ spread * rndx, spread * rndy, spread * rndz } + mSourcePosition;
}