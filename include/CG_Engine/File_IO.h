#ifndef FILE_IO_H
#define FILE_IO_H

#include <filesystem>
#include <memory>
#include <stdint.h>
#include <string>

namespace GL_Engine {
class File_IO
{
public:
    File_IO();
    ~File_IO();
    static std::string
        loadTextFile( const std::filesystem::path &_filePath, uint8_t *result );

    std::filesystem::path p;

    static void FreeImageData( void *_Data );
    //	static void* LoadPNGImage(std::string _Path, int &width, int &height);
    //	static void* LoadRawImage(std::string _Path, int &width, int &height);
    static void *LoadImageFile( const std::filesystem::path &_path, int &width, int &height, int &nChannels, bool flip );
    static void SaveImageFile( const std::filesystem::path &_path, int width, int height, int comp, void *data );
};

} // namespace GL_Engine
#endif // FILE_IO_H