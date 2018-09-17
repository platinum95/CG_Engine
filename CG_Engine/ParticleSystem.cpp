#include "ParticleSystem.h"
#include <iostream>
#include <time.h>

namespace GL_Engine {

	const std::string ParticleSystem::ParticleSystemVSource = {
				#include "ParticleSystemVShader.glsl" 
	};

	const std::string ParticleSystem::ParticleSystemFSource = {
					#include "ParticleSystemFShader.glsl" 
	};
	
	ParticleSystem::ParticleSystem(){
		
	}


	ParticleSystem::~ParticleSystem()
	{
	}

	std::unique_ptr<RenderPass> ParticleSystem::GenerateParticleSystem(const ParticleStats &stats, CG_Data::UBO *_CameraUBO)
	{
		srand(123184103u);
		//Set entity/particle initial values
		this->ParticleCount = stats.ParticleCount;
		this->Position = glm::vec4(stats.Position, 1.0);
		this->CameraUBO = _CameraUBO;
		this->Orientation = glm::quat(0.0, 0.0, 0.0, 1.0);
		this->Forward = glm::vec3(0, 0, 1);
		this->Up = glm::vec3(0, 1, 0);
		this->Right = glm::vec3(1, 0, 0);
		this->UpdateMatrix();
		this->Time = 0.0;

		
		//Generate initial data conditions
		float *InitialVelocityData = new float[this->ParticleCount * 3]; // start velocities vec3
		float *InitialTimeData = new float[this->ParticleCount]; // start times
		float *InitialSizeData = new float[this->ParticleCount]; // initial size
		float *InitialColourData = new float[this->ParticleCount * 3]; //initial colour
		float *InitialOpacityData = new float[this->ParticleCount];	//initial opacity
		float *LifetimeData = new float[this->ParticleCount];	//lifetime

		float TimeAccumulator = 0.0f;
		int j = 0;
		for (unsigned int i = 0; i < this->ParticleCount; i++) {

			auto RandFloat = [](float max) {return ((float)rand() / (float)RAND_MAX) * max; };
			// start times
			InitialTimeData[i] = TimeAccumulator;
			TimeAccumulator += 0.01f;

			//size
			auto SizeRangeDiff = stats.SizeRange[1] - stats.SizeRange[0];
			float size = RandFloat(SizeRangeDiff) + stats.SizeRange[0];
			InitialSizeData[i] = size;

			//opacity
			auto OpacityRangeDiff = stats.OpacityRange[1] - stats.OpacityRange[0];
			float opacity = RandFloat(OpacityRangeDiff) + stats.OpacityRange[0];
			InitialOpacityData[i] = opacity;

			//lifetime
			auto LifetimeRangeDiff = stats.LifetimeRange[1] - stats.LifetimeRange[0];
			float lifetime = RandFloat(LifetimeRangeDiff) + stats.LifetimeRange[0];
			LifetimeData[i] = lifetime;

			//colour
			glm::vec3 ColourRangeDiff = stats.ColourRange[1] - stats.ColourRange[0];
			glm::vec3 Colour = glm::vec3(RandFloat(ColourRangeDiff.x), RandFloat(ColourRangeDiff.y), RandFloat(ColourRangeDiff.z));
			Colour += stats.ColourRange[0];
			InitialColourData[j] = Colour.x;	//r
			InitialColourData[j + 1] = Colour.y; // g
			InitialColourData[j + 2] = Colour.z; // b

			//Get particle direction as a variation of the base direction
			//Start by getting an "up" vector, orthogonal to the base direction
			glm::vec3 NormalisedBase = glm::normalize(stats.BaseDirection);
			glm::vec3 s = glm::cross(NormalisedBase, glm::vec3(0, 1, 0));
			glm::vec3 up = glm::cross(NormalisedBase, glm::normalize(s));
			//Normalise the up vector
			up = glm::normalize(up);
			float RotationDegrees = ((float)rand() / (float)RAND_MAX) * 360.0f;
			//Rotate this up vector by a random amount around the base direction
			glm::vec3 RotationAxis = glm::toMat4(glm::angleAxis(glm::radians(RotationDegrees), NormalisedBase)) * glm::vec4(up, 0.0f);
			RotationAxis = glm::normalize(RotationAxis);
			float Variation = RandFloat(stats.DirectionVariation);
			glm::mat4 VariationMatrix = glm::toMat4(glm::angleAxis(glm::radians(Variation), RotationAxis));
			//Rotate the base direction by "Variation" amount around the "RotationAxis"
			glm::vec3 particleVelocity = VariationMatrix * glm::vec4(stats.BaseDirection, 0.0);
			particleVelocity = particleVelocity * stats.SpeedVariation;

			InitialVelocityData[j] = particleVelocity.x; // x
			InitialVelocityData[j + 1] = particleVelocity.y; // y
			InitialVelocityData[j + 2] = particleVelocity.z; // z
			j += 3;
		}

		//Set up VAO/VBOs
		this->ParticleVAO = std::make_shared<CG_Data::VAO>();
		this->ParticleVAO->BindVAO();

		auto VelocityVBO = std::make_unique<CG_Data::VBO>(&InitialVelocityData[0], this->ParticleCount * 3 * sizeof(float), GL_STATIC_DRAW);
		auto TimeVBO = std::make_unique<CG_Data::VBO>(&InitialTimeData[0], this->ParticleCount * sizeof(float), GL_STATIC_DRAW);
		auto SizeVBO = std::make_unique<CG_Data::VBO>(&InitialSizeData[0], this->ParticleCount * sizeof(float), GL_STATIC_DRAW);
		auto ColourVBO = std::make_unique<CG_Data::VBO>(&InitialColourData[0], this->ParticleCount * 3 * sizeof(float), GL_STATIC_DRAW);
		auto OpacityVBO = std::make_unique<CG_Data::VBO>(&InitialOpacityData[0], this->ParticleCount * sizeof(float), GL_STATIC_DRAW);
		auto LifetimeVBO = std::make_unique<CG_Data::VBO>(&LifetimeData[0], this->ParticleCount * sizeof(float), GL_STATIC_DRAW);

		VelocityVBO->BindVBO();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		TimeVBO->BindVBO();
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
		SizeVBO->BindVBO();
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
		ColourVBO->BindVBO();
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		OpacityVBO->BindVBO();
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
		LifetimeVBO->BindVBO();
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

		this->ParticleVAO->AddVBO(std::move(VelocityVBO));
		this->ParticleVAO->AddVBO(std::move(TimeVBO));
		this->ParticleVAO->AddVBO(std::move(SizeVBO));
		this->ParticleVAO->AddVBO(std::move(ColourVBO));
		this->ParticleVAO->AddVBO(std::move(OpacityVBO));
		this->ParticleVAO->AddVBO(std::move(LifetimeVBO));
		delete[] InitialColourData;
		delete[] InitialVelocityData;
		delete[] InitialTimeData;
		delete[] InitialSizeData;
		delete[] LifetimeData;
		delete[] InitialOpacityData;

		auto MatrixLambda = [](const CG_Data::Uniform &u) {glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, static_cast<const GLfloat*>(u.GetData())); };
		auto FloatLambda = [](const CG_Data::Uniform &u) {glUniform1fv(u.GetID(), 1, static_cast<const GLfloat*>(u.GetData())); };
		auto Vec3Lambda = [](const CG_Data::Uniform &u) {glUniform3fv(u.GetID(), 1, static_cast<const GLfloat*>(u.GetData())); };
		auto Vec4Lambda = [](const CG_Data::Uniform &u) {glUniform4fv(u.GetID(), 1, static_cast<const GLfloat*>(u.GetData())); };

		this->ParticleShader = std::make_unique<Shader>();
		this->ParticleShader->RegisterShaderStage(ParticleSystemVSource.c_str(), GL_VERTEX_SHADER);
		this->ParticleShader->RegisterShaderStage(ParticleSystemFSource.c_str(), GL_FRAGMENT_SHADER);
		this->ParticleShader->RegisterAttribute("Velocity", 0);
		this->ParticleShader->RegisterAttribute("Time", 1);
		this->ParticleShader->RegisterAttribute("Size", 2);
		this->ParticleShader->RegisterAttribute("Colour", 3);
		this->ParticleShader->RegisterAttribute("Opacity", 4);
		this->ParticleShader->RegisterAttribute("Lifetime", 5);
		this->ParticleShader->RegisterUBO(std::string("CameraProjectionData"), this->CameraUBO);
		auto TimeUniform = this->ParticleShader->RegisterUniform("CurrentTime", FloatLambda);
		auto EmitterUniform = this->ParticleShader->RegisterUniform("EmitterPosition", Vec3Lambda);
		auto DirectionUniform = this->ParticleShader->RegisterUniform("EmitterDirection", Vec3Lambda);
		auto ModelUniform = this->ParticleShader->RegisterUniform("model", MatrixLambda);
		auto GravityUniform = this->ParticleShader->RegisterUniform("Gravity", Vec3Lambda);
		this->ParticleShader->CompileShader();

		glUniform3f(GravityUniform->GetID(), 0, -1, 0);

	//	auto ModelIndex = this->AddData((void*)glm::value_ptr(this->TransformMatrix));
		auto EmitterIndex = this->AddData((void*)glm::value_ptr(this->Position));
		auto DirectionIndex = this->AddData((void*)glm::value_ptr(this->Forward));
		auto TimeIndex = this->AddData((void*)&(this->Time));

		auto ParticlePass = std::make_unique<RenderPass>();
		ParticlePass->renderFunction = std::function<void(RenderPass&, void*)>(this->ParticleRenderer);
		ParticlePass->shader = this->ParticleShader.get();
		uint32_t count = this->ParticleCount;
		ParticlePass->SetDrawFunction([count]() {glDrawArrays(GL_POINTS, 0, count); });
		ParticlePass->BatchVao = this->ParticleVAO;
		ParticlePass->AddBatchUnit(this);
		ParticlePass->AddDataLink(ModelUniform, 0);
		ParticlePass->AddDataLink(TimeUniform, TimeIndex);
		ParticlePass->AddDataLink(EmitterUniform, EmitterIndex);
		ParticlePass->AddDataLink(DirectionUniform, DirectionIndex);
		

		return std::move(ParticlePass);
	}

	void ParticleSystem::UpdateTime(const float &_Diff) {
		this->Time += _Diff;
	}
	void ParticleSystem::SetTime(const float& _CurrentTime) {
		this->Time = _CurrentTime;
	}
	const float& ParticleSystem::GetTime() const {
		return this->Time;
	}

	void ParticleSystem::ParticleRenderer(RenderPass &_Pass, void *_Data) {
		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_BLEND);

		_Pass.shader->UseShader();
		_Pass.BatchVao->BindVAO();

		for (auto&& batch : _Pass.batchUnits) {
			if (batch->active && batch->entity->isActive()) {
				for (auto l : _Pass.dataLink) {
					l.uniform->SetData(batch->entity->GetData(l.eDataIndex));
					l.uniform->Update();
				}
				batch->entity->UpdateUniforms();
				_Pass.DrawFunction();
			}
		}

		glDisable(GL_BLEND);
		glDisable(GL_PROGRAM_POINT_SIZE);
	}

}