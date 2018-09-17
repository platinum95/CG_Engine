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


	const char* File_IO::LoadTextFile(const char* _FilePath, uint8_t *result){
		if (_FilePath == "" || _FilePath == nullptr){
			*result = 1;
			return nullptr;
		}
		std::ifstream FileStream(_FilePath, std::ios::in | std::ios::binary);
		if (!FileStream){
			*result = 2;
			return nullptr;
		}
		std::string *FileStr = new std::string();
		FileStream.seekg(0, std::ios::end);	//Jump to end of file stream
		FileStr->resize(FileStream.tellg());	//Get stream size and resize string accordingly
		FileStream.seekg(0, std::ios::beg);
		FileStream.read(&(*FileStr)[0], FileStr->size());
		FileStream.close();

		*result = 0;
		return(FileStr->c_str());	//Return a const char array rather than std string
	}

	void File_IO::FreeImageData(void* _Data){
		stbi_image_free(_Data);
	}

}