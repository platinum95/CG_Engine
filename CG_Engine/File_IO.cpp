#include "File_IO.h"
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace GL_Engine{

	File_IO::File_IO()
	{
	}


	File_IO::~File_IO()
	{
	}
	void File_IO::SaveImageFile(std::string _Path, int width, int height, int comp, void* data) {
		stbi_write_bmp(_Path.c_str(), width, height, comp, data);
	}

	void* File_IO::LoadImageFile(std::string _Path, int &width, int &height, int &nChannels, bool flip) {
		stbi_set_flip_vertically_on_load(flip);
		void *data = stbi_load(_Path.c_str(), &width, &height, &nChannels, 0);
		return data;
	}


	std::string File_IO::loadTextFile( const std::string & _filePath,
									   uint8_t * result ){
		if ( _filePath == "" || _filePath.empty() ){
			*result = 1;
			return std::string( "" );
		}
		std::ifstream fileStream( _filePath, std::ios::in | std::ios::binary );
		if ( !fileStream ){
			*result = 2;
			return std::string( "" );
		}
		std::string fileStr;
		fileStream.seekg( 0, std::ios::end );
		// Get stream size and resize string memory accordingly
		fileStr.reserve( fileStream.tellg() );
		fileStream.seekg( 0, std::ios::beg );

		// Read file into string with stream iterators
		fileStr.assign( std::istreambuf_iterator<char>( fileStream ),
					    std::istreambuf_iterator<char>() );

		fileStream.close();

		*result = 0;
		return fileStr;
	}

	void File_IO::FreeImageData(void* _Data){
		stbi_image_free(_Data);
	}

}