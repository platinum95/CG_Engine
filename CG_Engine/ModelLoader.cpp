#include "ModelLoader.h"
#include <filesystem>

namespace GL_Engine {
    using namespace CG_Data;

#pragma region ModelAttribute
    ModelAttribute::ModelAttribute() {

    }
    ModelAttribute::~ModelAttribute() {

    }
    ModelAttribute::ModelAttribute( const aiScene *_Scene, unsigned int index, const std::string & _PathBase ) {
        //0 - Vertices
        //1 - Texture coords
        //2 - Normals
        //3 - Tangents
        //4 - Bitangents
        //5 - Bones
        this->BindVAO();
        MeshIndex = TexCoordIndex = NormalIndex = IndicesIndex = -1;
        auto mesh = _Scene->mMeshes[index];
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::unique_ptr<VBO> indexVBO = std::make_unique<VBO>(&indices[0], indices.size() * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
        this->VBOs.push_back(std::move(indexVBO));
        this->IndicesIndex = 0;
        this->VertexCount = indices.size();
        indices.clear();

        std::unique_ptr<VBO> meshVBO = std::make_unique<VBO>(mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
        meshVBO->BindVBO();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        this->VBOs.push_back(std::move(meshVBO));
        MeshIndex = (int) this->VBOs.size() - 1;

        if (mesh->HasNormals()) {
            std::unique_ptr<VBO> normalVBO = std::make_unique<VBO>(mesh->mNormals, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
            normalVBO->BindVBO();
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(2);
            this->VBOs.push_back(std::move(normalVBO));
            NormalIndex = (int) this->VBOs.size() - 1;
        }
        int i = 0;
        while (i == 0 && mesh->HasTextureCoords( i ) ) {//mesh->mTextureCoords[i]){
            std::vector<float> texCoords;
            texCoords.reserve(mesh->mNumVertices * sizeof(float) * 2);

            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                texCoords.push_back(mesh->mTextureCoords[i][j].x);
                texCoords.push_back(mesh->mTextureCoords[i][j].y);
            }
            std::unique_ptr<VBO> texCoordVBO = std::make_unique<VBO>(&texCoords[0], sizeof(float) * texCoords.size(), GL_STATIC_DRAW);
            texCoordVBO->BindVBO();
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(1);
            this->VBOs.push_back(std::move(texCoordVBO));
            TexCoordIndex = (int) this->VBOs.size() - 1;
            texCoords.clear();
            i++;
        }
        if (mesh->HasTangentsAndBitangents()) {
            std::unique_ptr<VBO> tangeantVBO = std::make_unique<VBO>(mesh->mTangents, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
            std::unique_ptr<VBO> bitangeantVBO = std::make_unique<VBO>(mesh->mBitangents, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
            tangeantVBO->BindVBO();
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(3);
            bitangeantVBO->BindVBO();
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(4);

            this->VBOs.push_back(std::move(tangeantVBO));
            this->VBOs.push_back(std::move(bitangeantVBO));
        }

        if (mesh->mMaterialIndex != -1) {
            aiMaterial *material = _Scene->mMaterials[mesh->mMaterialIndex];
            ModelLoader::loadMaterial(material,
                aiTextureType_DIFFUSE, _PathBase, this->ModelTextures);
            ModelLoader::loadMaterial(material,
                aiTextureType_NORMALS, _PathBase, this->ModelTextures);
            ModelLoader::loadMaterial(material,
                aiTextureType_HEIGHT, _PathBase, this->ModelTextures);
            ModelLoader::loadMaterial(material,
                aiTextureType_SPECULAR, _PathBase, this->ModelTextures);
            ModelLoader::loadMaterial(material,
                aiTextureType_SHININESS, _PathBase, this->ModelTextures);

        }

        if( auto numCol = mesh->GetNumColorChannels() ){
            for( int i = 0; i < numCol; i++ ){
                auto vCol = mesh->mColors[ i ];

            }
        }
    }
    void ModelAttribute::AddTexture(std::shared_ptr<CG_Data::Texture> _Texture) {
        this->ModelTextures.push_back(_Texture);
    }

    VBO* ModelAttribute::GetVBO(int index) {
        return this->VBOs[index].get();
    }

    const uint64_t ModelAttribute::GetVertexCount() const {
        return this->VertexCount;
    }


#pragma region ModelLoader
    std::map < std::string, std::shared_ptr< Texture > > 
        ModelLoader::cachedTextures;

    ModelAttribList ModelLoader::loadModel( const std::string &_modelPath,
                                            unsigned int _flags ){
        
        auto filePath = std::filesystem::path( _modelPath );
        auto pathBase = filePath.parent_path();
        auto modelFile = filePath.filename();
        const aiScene* _Scene = aImporter.ReadFile( filePath, _flags );
        if ( !_Scene ) {
            throw std::runtime_error( "Error loading model " + \
                _modelPath + "\n" + aImporter.GetErrorString() + "\n" );
        }

        auto numMeshes = _Scene->mNumMeshes;
        ModelAttribList attributes;
        attributes.reserve(numMeshes);

        for (unsigned int i = 0; i < _Scene->mNumMeshes; i++) {
            auto m = _Scene->mMeshes[i];
            auto newAttrib = std::make_shared< ModelAttribute >( _Scene, i, pathBase.string() );
            attributes.push_back( newAttrib );
        }
        aImporter.FreeScene();
        return attributes;
    }
    
    void LoadAnimations(std::map<std::string, std::shared_ptr<SceneNode>> &NodeList, const aiScene *_Scene) {
        for (unsigned int animID = 0; animID < _Scene->mNumAnimations; animID++) {
            aiAnimation* anim = _Scene->mAnimations[animID];
            for (unsigned int i = 0; i < anim->mNumChannels; i++) {
                aiNodeAnim *animNode = anim->mChannels[i];
                auto node = NodeList[animNode->mNodeName.data];
                node->Animation =std::make_shared<NodeAnimation>(animNode, anim->mDuration);
            }
        }
    }

    std::shared_ptr<SceneNode> LoadNodes(std::map<std::string, std::shared_ptr<SceneNode>> &NodeList, aiNode* node) {
        auto newNode = std::make_shared<SceneNode>(node);
        NodeList[node->mName.data] = newNode;
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            newNode->AddChild(LoadNodes(NodeList, node->mChildren[i]));
        }
        return newNode;
    }

    struct VertexBoneData {
        GLuint IDs[4] = { 0,0,0,0 };
        float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };
    void insertVertexData(std::vector<VertexBoneData> &vbd, unsigned int loc, float weight, unsigned int matID) {
        float minWeight = vbd[loc].Weights[0];
        int minPos = 0;
        int i;
        for (i = 0; i < 4; i++) {
            if (vbd[loc].Weights[i] == 0.0f) {
                minPos = i;
                break;
            }
            if (vbd[loc].Weights[i] < minWeight) {
                minWeight = vbd[loc].Weights[i];
                minPos = i;
            }
        }
        vbd[loc].IDs[minPos] = matID;
        vbd[loc].Weights[minPos] = weight;
    }

    void normaliseVertexData(std::vector<VertexBoneData> &vbd) {
        for (int vi = 0; vi < vbd.size(); vi++) {
            auto data = vbd.at(vi);
            float sum = data.Weights[0] + data.Weights[1] + data.Weights[2] + data.Weights[3];
            for (int i = 0; i < 4; i++) {
                data.Weights[i] = data.Weights[i] / sum;
                vbd[vi] = data;
            }
        }
    }

    std::unique_ptr<RiggedModel>
        ModelLoader::loadRiggedModel( const std::string &_modelPath,
                                      unsigned int _flags ){

        auto filePath = std::filesystem::path( _modelPath );
        auto pathBase = filePath.parent_path();
        auto modelFile = filePath.filename();

        const aiScene* _Scene = aImporter.ReadFile( filePath, _flags);
        if (!_Scene) {
            throw std::runtime_error("Error loading model " + _modelPath + "\n" + aImporter.GetErrorString() + "\n");
        }
        //Load in the scene's nodes
        std::map<std::string, std::shared_ptr<SceneNode>> Nodes;
        auto rootNode = LoadNodes(Nodes, _Scene->mRootNode);
        LoadAnimations(Nodes, _Scene);
        auto numMeshes = _Scene->mNumMeshes;
        ModelAttribList attributes;
        attributes.reserve(numMeshes);

        std::map<std::string, std::shared_ptr<SceneBone>> SceneBoneMap;
        //Load in the scene's meshes, as well as the bones for each mesh
        for (unsigned int i = 0; i < _Scene->mNumMeshes; i++) {
            std::map<std::string, std::shared_ptr<MeshBone>> MeshBoneMap;
            auto mesh = _Scene->mMeshes[i];
            std::shared_ptr<ModelAttribute> newAttrib = std::make_shared<ModelAttribute>( _Scene, i, pathBase );
            std::vector<VertexBoneData> WeightVBOData;
            WeightVBOData.resize(mesh->mNumVertices);
            for (unsigned int bi = 0; bi < mesh->mNumBones; bi++) {
                unsigned int BoneIndex = 0;
                aiBone *mBone = mesh->mBones[bi];
                auto newMeshBone = std::make_shared<MeshBone>(mBone);
                std::shared_ptr<SceneBone> sceneBone;
                if (SceneBoneMap.count(mBone->mName.data) == 0) {
                    sceneBone = std::make_shared<SceneBone>(mBone);
                    SceneBoneMap[mBone->mName.data] = sceneBone;
                    Nodes[mBone->mName.data]->sceneBone = sceneBone;
                }
                else {
                    sceneBone = SceneBoneMap[mBone->mName.data];
                }
                sceneBone->AddMeshBone(newMeshBone);

                newAttrib->meshBones.push_back(newMeshBone);
                BoneIndex = (unsigned int)newAttrib->meshBones.size() - 1;
                newAttrib->BoneIndex[mBone->mName.data] = BoneIndex;

                for (unsigned int bwi = 0; bwi < mBone->mNumWeights; bwi++) {
                    auto weightData = mBone->mWeights[bwi];
                    auto vertexID = weightData.mVertexId;
                    auto weight = weightData.mWeight;
                    insertVertexData(WeightVBOData,vertexID, weight, BoneIndex);
                }
            }
            
            normaliseVertexData(WeightVBOData);
            std::vector<float> Weights;
            std::vector<GLuint> IDs;
            for (auto data : WeightVBOData) {
                Weights.insert(Weights.end(), { data.Weights[0],data.Weights[1], data.Weights[2], data.Weights[3] });
                IDs.insert(IDs.end(), { data.IDs[0],data.IDs[1], data.IDs[2], data.IDs[3] });
            }
            newAttrib->BindVAO();

            std::unique_ptr<VBO> IDVBO = std::make_unique<VBO>();
            IDVBO->BindVBO();
            glBufferData(GL_ARRAY_BUFFER, IDs.size() * sizeof(GLuint), &IDs[0], GL_STATIC_DRAW);
            glVertexAttribIPointer(5, 4, GL_UNSIGNED_INT, 0, nullptr);
            glEnableVertexAttribArray(5);
            newAttrib->AddVBO(std::move(IDVBO));

            std::unique_ptr<VBO> WeightsVBO = std::make_unique<VBO>();
            WeightsVBO->BindVBO();
            glBufferData(GL_ARRAY_BUFFER, Weights.size() * sizeof(float), &Weights[0], GL_STATIC_DRAW);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(6);
            newAttrib->AddVBO(std::move(WeightsVBO));

            for (int i = 0; i < Weights.size(); i+=4) {
                float sum = Weights[i] + Weights[i + 1] + Weights[i + 2] + Weights[i + 3];
                if (sum > 1.01 || sum < 0.99) {
                    std::cout << sum << std::endl;
                }
            }

            attributes.push_back(std::move(newAttrib));
        }

        aImporter.FreeScene();

        return std::make_unique<RiggedModel>(std::make_unique<Skeleton>(rootNode, Nodes), std::move(attributes));
    }

    void ModelLoader::cleanup() {
        aImporter.FreeScene();
        cachedTextures.clear();
    }

    std::vector< std::shared_ptr< Texture > > 
        ModelLoader::loadMaterial( const aiMaterial *material,
                                   const aiTextureType _Type, 
                                   const std::filesystem::path &_PathBase,
                                   std::vector< std::shared_ptr< Texture > >
                                    & _Textures ){
        for (unsigned int i = 0; i < material->GetTextureCount(_Type); i++) {
            aiString texPathAiStr;
            material->GetTexture( _Type, i, &texPathAiStr );
            
            // Convert separator to x-platform
            std::string texPathStr = std::string( texPathAiStr.C_Str() );
            std::replace( texPathStr.begin(), texPathStr.end(), '\\', '/' );

            auto texRelPath = std::filesystem::path( texPathStr );
            if ( texRelPath.empty() )
                continue;

            auto texPath = std::filesystem::path( _PathBase ) / texRelPath;
            if ( cachedTextures[ texPath ] ) {
                _Textures.push_back( cachedTextures[ texPath ] );
                continue;
            }
            GLuint texUnit = GL_TEXTURE0;

            switch( _Type ){
                case aiTextureType_DIFFUSE:
                    texUnit = GL_TEXTURE0;
                    break;
                case aiTextureType_NORMALS:
                    texUnit = GL_TEXTURE1;
                    break;
                case aiTextureType_SPECULAR:
                    texUnit = GL_TEXTURE2;
                    break;
                case aiTextureType_SHININESS:
                    texUnit = GL_TEXTURE3;
                    break;
                case aiTextureType_HEIGHT:
                    texUnit = GL_TEXTURE4;
                    break;

            }
            const auto texture = loadTexture( texPath, texUnit );
            _Textures.push_back( texture );
            cachedTextures[ texPath ] = std::move( texture );
        }
        return _Textures;
    }

    std::shared_ptr<Texture>
    ModelLoader::loadTexture( const std::filesystem::path & _Path, 
							  GLuint _Unit ){
        int width, height, nChannels;
        void* data = File_IO::LoadImageFile( _Path, width, 
                                             height, nChannels, true );
        GLint format = GL_RGB;
        switch( nChannels ){
            case 0:
                break;
            case 1:
                format = GL_RED;
                break;
            case 2:
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
        }
        auto parameters = []() {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        };
        std::shared_ptr<Texture> newTexture =
            std::make_shared<Texture>( data, width, height, _Unit, format, 
                                       parameters, GL_TEXTURE_2D );
        free(data);
        //File_IO::FreeImageData(data);
        return newTexture;
    }


}