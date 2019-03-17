#include "Terrain.h"
namespace GL_Engine {

#pragma region TerrainGenerator


	MeshData 
	TerrainGenerator::CreateMesh(uint32_t MeshSize, uint32_t Divisions) {
		std::vector<glm::vec2> Mesh;
		Mesh.reserve(Divisions * Divisions);
		//Create the mesh
		for (unsigned int z = 0; z < Divisions; z++) {
			int zIndex = z * Divisions;
			float zRatio = (float)z / (float)(Divisions - 1);
			float zPosValue = zRatio * MeshSize;
			for (unsigned int x = 0; x < Divisions; x++) {
				float xRatio = (float)x / (float)(Divisions - 1);
				float xPosValue = xRatio * MeshSize;
				Mesh.push_back(glm::vec2(xPosValue, zPosValue));
			}
		}
		std::vector<unsigned int> Indices;
		//Create the indices
		for (unsigned int z = 0; z < Divisions - 1; z++) {
			unsigned int zIndex = z * Divisions;
			for (unsigned int x = 0; x < Divisions - 1; x++) {
				unsigned int CurrIndex = zIndex + x;
				unsigned int CurrIndexBelow = CurrIndex + Divisions;
				//need to add 2 triangles here.
				Indices.push_back(CurrIndex);
				Indices.push_back(CurrIndexBelow + 1);
				Indices.push_back(CurrIndex + 1);

				Indices.push_back(CurrIndex);
				Indices.push_back(CurrIndexBelow);
				Indices.push_back(CurrIndexBelow + 1);
			}
		}
		std::vector<glm::vec2> texcoord;
		texcoord.reserve(Divisions * Divisions);
		//Create the mesh
		for (unsigned int z = 0; z < Divisions; z++) {
			int zIndex = z * Divisions;
			float zRatio = (float)z / (float)(Divisions - 1);
			for (unsigned int x = 0; x < Divisions; x++) {
				float xRatio = (float)x / (float)(Divisions - 1);
				texcoord.push_back(glm::vec2(xRatio, zRatio));
			}
		}
		return MeshData{ Indices, Mesh, texcoord };
	}

	std::vector<float>
	TerrainGenerator::GenerateHeights ( int MeshSize, int DivisionCount, 
								        int GridX, int GridZ ){
		int startingX = GridX * (DivisionCount - 1);
		int startingZ = GridZ * (DivisionCount - 1);
		std::vector<float> heights;
		heights.reserve(DivisionCount * DivisionCount);
		for (int z = 0; z < DivisionCount; z++) {
			unsigned int zIndex = z * DivisionCount;
			for (int x = 0; x < DivisionCount; x++) {
				unsigned int CurrIndex = zIndex + x;
				heights.push_back(getHeight(startingX + x, startingZ + z));
			}
		}
		return heights;
	}

	std::vector<glm::vec3> 
	TerrainGenerator::GetSmoothNormals( const MeshData &meshData, 
										const std::vector< float > Heights ){
		auto indices = meshData.Indices;
		auto vertices = meshData.Mesh;

		std::vector< glm::vec3 > faceNormals;
		faceNormals.reserve( indices.size() / 3 );

		std::vector< std::vector< glm::vec3 > > vertexNormalList;
		vertexNormalList.resize( vertices.size() );

		// Calculate the normal for each face (3 vertices)
		for ( auto fi = 0; fi < indices.size(); fi += 3 ){
			// Get the vertex indices for the face
			unsigned int face[3] = { 
				indices[ fi ],
				indices[ fi + 1 ],
				indices[ fi + 2 ]
			};
			// Get the 3 vertices for the face
			glm::vec3 p1( vertices[ face[ 0 ] ].x,
						  Heights[ face[ 0 ] ],
						  vertices [face[ 0 ] ].y );

			glm::vec3 p2( vertices[ face[ 1 ] ].x,
						  Heights[ face[ 1 ] ],
						  vertices[ face[ 1 ] ].y );

			glm::vec3 p3( vertices[ face[ 2 ] ].x,
						  Heights[ face[ 2 ] ], 
						  vertices[ face[ 2 ] ].y )
						  ;
			glm::vec3 A = p3 - p2;
			glm::vec3 B = p1 - p2;
			glm::vec3 faceNorm = glm::cross( A, B );
			faceNorm = glm::normalize( faceNorm );
			faceNormals.push_back( faceNorm );
			vertexNormalList[ face[ 0 ] ].push_back( faceNorm );
			vertexNormalList[ face[ 1 ] ].push_back( faceNorm );
			vertexNormalList[ face[ 2 ] ].push_back( faceNorm );
		}
		// We may have more than one normal per vertex, so here we merge them
		std::vector< glm::vec3 > vertexNormals;
		vertexNormals.resize( vertices.size() );
		for ( unsigned int vli = 0; vli < vertexNormalList.size(); vli++ ){
			glm::vec3 nSum( 0.0f, 0.0f, 0.0f );
			for ( auto fn : vertexNormalList[ vli ] ){
				nSum += fn;
			}
			//nSum = nSum / (float)vertexNormalList[vli].size();
			vertexNormals[ vli ] = glm::normalize( nSum );
		}


		return vertexNormals;
	}

	ChunkData 
	TerrainGenerator::GenerateChunk( int GridX, int GridZ,
									 const MeshData &meshData ) {
		auto Heights = GenerateHeights( meshData.MeshSize,
										meshData.DivisionCount, GridX, GridZ );
		auto Normals = GetSmoothNormals( meshData, Heights );
		ChunkData data = { Heights, Normals };
		return data;
	}
	float TerrainGenerator::getHeight( int x, int z ) {
		float total = getInterpolatedNoise(x / 32.0f, z / 32.0f);
		total += getInterpolatedNoise(x / 16.0f, z / 16.0f) / 2.0f;
		total += getInterpolatedNoise(x / 8.0f, z / 8.0f) / 7.0f;

		return total;
	}

	float TerrainGenerator::getSmoothNoise( int x, int z ) {
		float corners = ( getNoise( x + 1, z + 1 ) +
					      getNoise( x - 1, z + 1 ) +
						  getNoise( x - 1, z - 1 ) +
						  getNoise( x + 1, z - 1 ) ) / 16.0f;

		float sides = ( getNoise( x + 1, z ) +
						getNoise( x - 1, z ) +
						getNoise( x, z - 1 ) +
						getNoise( x, z - 1 ) ) / 8.0f;

		float centre = getNoise( x, z ) / 4.0f;
		return corners + centre + sides;
	}

	float TerrainGenerator::getNoise(int x, int z) {
		int n = x * 45 + z * 57;
		n = (n << 13) ^ n;
		int nn = (n*(n*n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		float toReturn = (float)(1.0 - ((double)nn / 1073741824.0));
		return toReturn * 40.0f;
	}

	float TerrainGenerator::interpolate(float a, float b, float blend) {
		float f = (1.0f - cos(blend * 3.145f)) * 0.5f;
		return a + (f)*(b - a);
	}

	float TerrainGenerator::getInterpolatedNoise(float x, float z) {
		int intX = (int)x;
		int intZ = (int)z;

		float fracX = x - intX;
		float fracZ = z - intZ;
		if (x <= 0) {
			fracX = 1 + fracX;
			intX--;
		}
		if (z <= 0) {
			fracZ = 1 + fracZ;
			intZ--;
		}
		float v1 = getSmoothNoise(intX, intZ);
		float v2 = getSmoothNoise(intX + 1, intZ);
		float v3 = getSmoothNoise(intX, intZ + 1);
		float v4 = getSmoothNoise(intX + 1, intZ + 1);
		float i1 = interpolate(v1, v2, fracX);
		float i2 = interpolate(v3, v4, fracX);
		return interpolate(i1, i2, fracZ);
	}

#pragma region TerrainChunk

	TerrainChunk::TerrainChunk( const MeshBaseVBOs & baseVBOs,
								const MeshData &meshData, 
								int GridX, int GridZ ) {

		auto chunkData = TerrainGenerator::GenerateChunk( GridX, GridZ, 
														  meshData );
		this->BindVAO();
		baseVBOs.IndexVBO->BindVBO();
		baseVBOs.MeshVBO->BindVBO();
		glEnableVertexAttribArray(0);	//Mesh always at index 0
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		baseVBOs.TexVBO->BindVBO();
		glEnableVertexAttribArray(2);	//UV always at index 2
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		auto Heights = chunkData.Heights;
		auto HeightVBO = std::make_unique<CG_Data::VBO>( 
			&Heights[0], Heights.size() * sizeof( float ), GL_STATIC_DRAW );
	
		glEnableVertexAttribArray(1);	//Heights always at index 1
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
		this->AddVBO(std::move(HeightVBO));

		auto Normals = chunkData.Normals;
		auto NormalVBO = std::make_unique< CG_Data::VBO >( 
			&Normals[0], Normals.size() * sizeof( glm::vec3 ), GL_STATIC_DRAW );

		glEnableVertexAttribArray(3);	//Normals always at index 3
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		this->AddVBO(std::move(NormalVBO));

		this->WorldGridPosition = glm::vec2(GridX, GridZ);
		this->WorldPos = WorldGridPosition * ((float)meshData.MeshSize);
		this->Translation = glm::translate( glm::mat4( 1.0f ), 
											glm::vec3( this->WorldPos.x, 0,
													   this->WorldPos.y ) );
	}


#pragma region Terrain
	Terrain::Terrain(uint32_t _MeshSize, uint32_t _DivisionCount) {
		this->MeshSize = _MeshSize;
		this->DivisionCount = _DivisionCount;
		this->meshData = TerrainGenerator::CreateMesh( this->MeshSize, 
													   this->DivisionCount );
		meshData.DivisionCount = this->DivisionCount;
		meshData.MeshSize = this->MeshSize;
		glBindVertexArray(0);
		baseVBOs.MeshVBO = std::make_shared<CG_Data::VBO>(
			&meshData.Mesh[0], meshData.Mesh.size() * sizeof(glm::vec2), 
			GL_STATIC_DRAW );

		baseVBOs.TexVBO = std::make_shared<CG_Data::VBO>(
			&meshData.TexCoords[0], 
			meshData.TexCoords.size() * sizeof(glm::vec2), GL_STATIC_DRAW );

		baseVBOs.IndexVBO = std::make_shared<CG_Data::VBO>(
			&meshData.Indices[0],
			meshData.Indices.size() * sizeof(unsigned int),
			GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER );
	}
	
	std::shared_ptr<TerrainChunk> 
	Terrain::GenerateChunk( int xGrid, int zGrid ){
		auto newChunk = std::make_shared< TerrainChunk >( 
			this->baseVBOs, meshData, xGrid, zGrid );

		this->tPack.TerrainChunks.push_back( newChunk );
		return newChunk;
	}

	std::unique_ptr< RenderPass >
	Terrain::GetRenderPass( Shader * _GroundShader, bool isProj ) {
		auto renderPass = std::make_unique< RenderPass >();
		renderPass->Data = static_cast< TerrainPack * >( &tPack );
		if( !isProj ){
			renderPass->renderFunction = &TerrainRenderer;
		} else {
			renderPass->renderFunction = &TerrainProjRenderer;
		}
		GLsizei nCount = ( GLsizei ) meshData.Indices.size();

		auto drawFunct = [ nCount ]() {
			glDrawElements( GL_TRIANGLES, nCount, GL_UNSIGNED_INT, nullptr );
		};
		
		renderPass->SetDrawFunction(drawFunct);
		renderPass->shader = _GroundShader;
		tPack.translationUniformLocation =
			_GroundShader->getUniform( "GroundTranslation" )->GetID();
		return std::move( renderPass );
	}


	void Terrain::TerrainRenderer(RenderPass &Pass, void* _Data) {
		auto chunks = static_cast<TerrainPack*>(_Data);

		Pass.shader->useShader();
		for (auto tex : Pass.Textures) {
			tex->Bind();
		}
		for( auto dLink : Pass.dataLink ){
			dLink.uniform->SetData( chunks->terrainEntity.GetData( 
										dLink.eDataIndex ) );
			dLink.uniform->Update();
		}
		auto translationUniformLocation =
			Pass.shader->getUniform( "GroundTranslation" )->GetID();
		chunks->terrainEntity.UpdateUniforms();
		for (auto chunk : chunks->TerrainChunks) {
			glUniformMatrix4fv( translationUniformLocation, 1, 
							    GL_FALSE, 
								glm::value_ptr( chunk->Translation ) );
			chunk->BindVAO();
			Pass.DrawFunction();
		}
	}

	void Terrain::TerrainProjRenderer( RenderPass &rPass, void* _data ) {
		auto chunks = static_cast< TerrainPack* >( _data );

		rPass.shader->useShader();
		auto translationUniformLocation =
			rPass.shader->getUniform( "GroundTranslation" )->GetID();

		for ( auto chunk : chunks->TerrainChunks ) {
			glUniformMatrix4fv( translationUniformLocation, 1, 
							    GL_FALSE, 
								glm::value_ptr( chunk->Translation ) );
			chunk->BindVAO();
			rPass.DrawFunction();
		}
	}

}
