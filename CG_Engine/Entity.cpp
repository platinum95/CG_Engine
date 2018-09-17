#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Utilities.h"

namespace GL_Engine {
#pragma region ENTITY
	Entity::Entity() {
		Orientation = glm::quat(1, 0, 0, 0);
		eData.push_back(glm::value_ptr(this->TransformMatrix));
	}

	Entity::~Entity() {
	}


	void Entity::SetPosition(glm::vec3 _Position) {
		this->Position = glm::vec4(_Position, 1.0);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::Translate(glm::vec3 _Translation) {
		this->Position += glm::vec4(_Translation, 0.0);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::YawBy(float _Degrees) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Up);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Right = glm::rotate(Versor, this->Right);
		this->MatrixNeedsUpdating = true;
	}
	void Entity::PitchBy(float _degrees) {
		float Radians = glm::radians(_degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Right);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Up = glm::rotate(Versor, this->Up);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::RollBy(float _Degrees) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Forward);

		Orientation = Versor * Orientation;
		this->Right = glm::rotate(Versor, this->Right);
		this->Up = glm::rotate(Versor, this->Up);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::RotateBy(float _Degrees, glm::vec3 _Axis) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, _Axis);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Right = glm::rotate(Versor, this->Right);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::SetScale(glm::vec3 _Scale) {
		this->Scale = _Scale;
		MatrixNeedsUpdating = true;
	}

	void Entity::ScaleBy(glm::vec3 _Scale) {
		this->Scale *= _Scale;
		MatrixNeedsUpdating = true;
	}

	const glm::quat Entity::GetOrientation() const {
		return this->Orientation;
	}

	void Entity::UpdateUniforms() const {
		for (const auto uniform : this->EntityUniforms) {
			uniform->Update();
		}
	}

	void Entity::Rotate(glm::quat _Rotation) {
		this->Orientation = _Rotation * Orientation;
		this->Forward = _Rotation * Forward;
		this->Up = _Rotation * Up;
		this->Right = _Rotation * Right;
		this->MatrixNeedsUpdating = true;
	}

	void Entity::SetOrientation(glm::quat _Orientation) {
		this->Orientation = _Orientation; 
		this->MatrixNeedsUpdating = true; 
	}

	const glm::mat4 Entity::GetTransformMatrix() {
		//Check if the local values have changed
		if (MatrixNeedsUpdating) {
			UpdateMatrix();	//Update matrix if so
			MatrixNeedsUpdating = false;
		}
		//Return the model matrix
		return this->TransformMatrix;
	}

	void Entity::UpdateMatrix() {
		//Generate rotation matrix from the orientation versor
		glm::mat4 R = glm::toMat4(Orientation);		

		//Create a translation matrix from identity
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(this->Position));	

		//Generate a scale matrix from identity
		glm::mat4 S = glm::scale(glm::mat4(1.0), this->Scale);

		//Regenerate local axes for next set of rotations
		this->Forward = glm::vec3(R * glm::vec4(0, 0, 1, 0));
		this->Up = glm::vec3(R * glm::vec4(0, 1, 0, 0));
		this->Right = glm::vec3(R * glm::vec4(1, 0, 0, 0));

		//Rotation matrix needs to be inverted to correctly
		//rotate the vertices. (I think, I was having 
		//problems when R isn't inverted)
		this->TransformMatrix = T * glm::inverse(R) * S;

	}

	const glm::vec4 Entity::GetPosition() const { 
		return this->Position; 
	}

	const glm::mat4 Entity::TransformBy(glm::mat4 _Transform) {
		return this->TransformMatrix = _Transform * this->TransformMatrix;
	}

	const uint16_t Entity::AddData(void* _Data) {
		eData.push_back(_Data);
		return (uint16_t)eData.size() - 1;
	}

	void Entity::SetData(int index, void* data) {
		eData[index] = data;
	}

	void Entity::SetActive(bool _State) {
		this->Active = _State;
	}

	void* Entity::GetData(int index) {
		return eData[index];
	}

	bool Entity::isActive() const { 
		return Active;
	}
	void Entity::Activate() { 
		Active = true;
	}
	void Entity::Deactivate() { 
		Active = false;
	}

	const std::vector<void*> Entity::GeteDataList() const { 
		return this->eData; 
	};

#pragma endregion

#pragma region RiggedModel
#pragma region MeshBone

	MeshBone::MeshBone(const aiBone* _Bone) {
		this->OffsetMatrix = Utilities::AiToGLMMat4(_Bone->mOffsetMatrix);
		this->Name = _Bone->mName.data;
	}
	const glm::mat4 &MeshBone::GetFinalTransform(const glm::mat4 &_GlobalInverse, const glm::mat4& nodeGlobalTransform) {
		this->FinalTransformation = _GlobalInverse * nodeGlobalTransform * this->OffsetMatrix;
		return this->FinalTransformation;
	}


#pragma region SceneBone

	SceneBone::SceneBone(const aiBone *_Bone) {
		this->Name = _Bone->mName.data;
	}
	void SceneBone::AddMeshBone(std::shared_ptr<MeshBone> _mBone) {
		this->meshBones.push_back(_mBone);
	}
	void SceneBone::UpdateBone(const glm::mat4 &GlobalInverse, const glm::mat4 &_GlobalTransform) {
		this->GlobalTransformation = _GlobalTransform;
		for (auto mb : meshBones) {
			mb->GetFinalTransform(GlobalInverse, this->GlobalTransformation);
		}
	}

#pragma region NodeAnimation
	NodeAnimation::NodeAnimation(const aiNodeAnim *animNode, double _Length) {
		this->Name = animNode->mNodeName.data;
		this->AnimationLength = _Length;
		for (unsigned int i = 0; i < animNode->mNumPositionKeys; i++) {
			aiVector3D pos = animNode->mPositionKeys[i].mValue;
			double time = animNode->mPositionKeys[i].mTime;
			this->Positions.push_back(std::make_pair(glm::vec3(pos.x, pos.y, pos.z), time));
		}
		for (unsigned int i = 0; i < animNode->mNumScalingKeys; i++) {
			aiVector3D scale = animNode->mScalingKeys[i].mValue;
			double time = animNode->mScalingKeys[i].mTime;
			this->Scalings.push_back(std::make_pair(glm::vec3(scale.x, scale.y, scale.z), time));
		}
		for (unsigned int i = 0; i < animNode->mNumRotationKeys; i++) {
			auto rot = animNode->mRotationKeys[i];
			double time = animNode->mPositionKeys[i].mTime;
			glm::quat rotQuat;
			rotQuat.x = rot.mValue.x;
			rotQuat.y = rot.mValue.y;
			rotQuat.z = rot.mValue.z;
			rotQuat.w = rot.mValue.w;
			this->Rotations.push_back(std::make_pair(rotQuat, time));
		}
	}
	

#pragma region SceneNode
	SceneNode::SceneNode(const aiNode* _node) {
		this->NodeTransform = Utilities::AiToGLMMat4(_node->mTransformation);
		this->Name = _node->mName.data;
	}
	void SceneNode::AddChild(std::shared_ptr<SceneNode> _node) {
		this->ChildNodes.push_back(_node);
	}
	void SceneNode::Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse) {
		this->GlobalTransform = ParentTransform * this->NodeTransform;
		if(sceneBone)
			sceneBone->UpdateBone(GlobalInverse, this->GlobalTransform);
		
		for (auto cn : ChildNodes) {
			cn->Update(this->GlobalTransform, GlobalInverse);
		}
	}
	void SceneNode::Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse, unsigned int AnimationID, double Time) {
		auto LocalMatrix = this->NodeTransform;
		if (Animation) {
			Time = fmod(Time, Animation->AnimationLength);
			glm::mat4 ScaleMatrix, RotateMatrix, TranslateMatrix;

			ScaleMatrix = this->GetInterpolatedScale(Animation->Scalings, Time);
			TranslateMatrix = this->GetInterpolatedTranslate(Animation->Positions, Time);
			RotateMatrix = this->GetInterpolatedRotate(Animation->Rotations, Time);

			LocalMatrix = TranslateMatrix * RotateMatrix * ScaleMatrix;
		}
		this->GlobalTransform = ParentTransform * LocalMatrix;
		if(sceneBone)
			sceneBone->UpdateBone(GlobalInverse, this->GlobalTransform);
		
		for (auto cn : ChildNodes) {
			cn->Update(this->GlobalTransform, GlobalInverse, AnimationID, Time);
		}
	}
	glm::mat4 SceneNode::GetInterpolatedScale(std::vector<std::pair<glm::vec3, double>> Scalings, double time) {
		std::pair<glm::vec3, double> LowerScale, UpperScale;
		for (int i = 0; i < Scalings.size() - 1; i++) {
			if (time > Scalings[i].second && time < Scalings[i + 1].second) {
				LowerScale = Scalings[i];
				UpperScale = Scalings[i + 1];
				break;
			}
			if (i == Scalings.size() - 2) {
				return glm::scale(glm::mat4(1.0), Scalings[i + 1].first);
			}
		}

		double timeDiff = UpperScale.second - LowerScale.second;
		double timeNorm = time - LowerScale.second;
		double ratio = timeNorm / timeDiff;
		glm::vec3 interpolatedScale = (LowerScale.first * (float)(1.0 - ratio)) + (UpperScale.first * (float)ratio);
		return glm::scale(glm::mat4(1.0), interpolatedScale);

	}
	glm::mat4 SceneNode::GetInterpolatedTranslate(std::vector<std::pair<glm::vec3, double>> Translations, double time) {
		std::pair<glm::vec3, double> LowerScale, UpperScale;
		for (int i = 0; i < Translations.size() - 1; i++) {
			if (time > Translations[i].second && time < Translations[i + 1].second) {
				LowerScale = Translations[i];
				UpperScale = Translations[i + 1];
				break;
			}
			if (i == Translations.size() - 2) {
				return glm::translate(glm::mat4(1.0), Translations[i + 1].first);
			}
		}

		double timeDiff = UpperScale.second - LowerScale.second;
		double timeNorm = time - LowerScale.second;
		double ratio = timeNorm / timeDiff;
		glm::vec3 interpolatedTranslate = (LowerScale.first * (float)(1.0 - ratio)) + (UpperScale.first * (float)ratio);
		return glm::translate(glm::mat4(1.0), interpolatedTranslate);
	}
	glm::mat4 SceneNode::GetInterpolatedRotate(std::vector<std::pair<glm::quat, double>> Rotations, double time) {
		std::pair<glm::quat, double> LowerScale, UpperScale;
		for (int i = 0; i < Rotations.size() - 1; i++) {
			if (time > Rotations[i].second && time < Rotations[i + 1].second) {
				LowerScale = Rotations[i];
				UpperScale = Rotations[i + 1];
				break;
			}
			if (i == Rotations.size() - 2) {
				return glm::toMat4(Rotations[i + 1].first);
			}
		}
		double timeDiff = UpperScale.second - LowerScale.second;
		double timeNorm = time - LowerScale.second;
		double ratio = timeNorm / timeDiff;
		glm::quat interp = glm::slerp(LowerScale.first, UpperScale.first, (float)ratio);
		return glm::toMat4(interp);
	}



#pragma region Skeleton

	Skeleton::Skeleton(std::shared_ptr<SceneNode> _Root, std::map<std::string, std::shared_ptr<SceneNode>> SkeletonNodeMap) {
		this->rootNode = _Root;
		this->GlobalInverseMatrix = glm::inverse(_Root->NodeTransform);
		this->NodeMap = SkeletonNodeMap;
	}

	void Skeleton::Update() {
		rootNode->Update(glm::mat4(1.0f), GlobalInverseMatrix);
	}
	void Skeleton::Update(unsigned int AnimationID, double Time) {
		rootNode->Update(glm::mat4(1.0f), GlobalInverseMatrix, AnimationID, Time);
	}

#pragma region RiggedModel


	RiggedModel::RiggedModel(std::unique_ptr<Skeleton> _Rig, ModelAttribList &&_AttributeList) {
		this->ModelRig = std::move(_Rig);

		this->ModelAttributes = std::forward<ModelAttribList>(_AttributeList);

		Orientation = glm::quat(1, 0, 0, 0);
		eData.push_back(glm::value_ptr(this->TransformMatrix));
		this->ModelRig->Update();
	}
	RiggedModel::~RiggedModel() {};

	std::unique_ptr<RenderPass> RiggedModel::GenerateRenderpass(Shader* _Shader) {
		std::unique_ptr<RenderPass> renderPass = std::make_unique<RenderPass>();
		renderPass->renderFunction = RiggedModelRenderer;
		renderPass->shader = _Shader;
		renderPass->Data = (void*)this;
		return std::move(renderPass);
	}
	void RiggedModel::Update() {
		this->ModelRig->Update();
	}
	void RiggedModel::Update(unsigned int AnimationID, double Time) {
		this->ModelRig->Update(AnimationID, Time);
	}
	Skeleton *RiggedModel::GetRig() const { 
		return this->ModelRig.get(); 
	}



	void RiggedModel::RiggedModelRenderer(RenderPass& _Pass, void* _Data) {
		
		RiggedModel *Model = static_cast<RiggedModel*>(_Data);
		auto Rig = Model->GetRig();
		_Pass.shader->UseShader();

		for (auto l : _Pass.dataLink) {
			l.uniform->SetData(Model->GetData(l.eDataIndex));
			l.uniform->Update();
		}
		Model->UpdateUniforms();

		glm::mat4 t(1.0f);
		auto modelMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "model");
		glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(Model->GetTransformMatrix()));

		
		for (auto attrib : Model->ModelAttributes) {
			attrib->BindVAO();
			for (auto tex : attrib->ModelTextures) {
				tex->Bind();
			}
			auto boneMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "BoneMatrices");
			std::vector<glm::mat4> boneMatrices((const size_t)56, glm::mat4(1.0));
			int i = 0;
			if (attrib->meshBones.size() > 0) {
				int i = 0;
				for (auto bone : attrib->meshBones) {
					boneMatrices.at(i++) = bone->FinalTransformation;
				}
				glUniformMatrix4fv(boneMatLoc, 56, GL_FALSE, glm::value_ptr(boneMatrices[0]));
			}
			else {
				glm::mat4 id(1.0);
				auto boneMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "BoneMatrices");
				glUniformMatrix4fv(boneMatLoc, 1, GL_FALSE, glm::value_ptr(id));
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)attrib->GetVertexCount(), GL_UNSIGNED_INT, 0);
		}
		
	}

}

#pragma endregion