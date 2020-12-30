#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

Texture::Texture(std::string path, GLint interpolation)
	: textureID(), path(path), interpolation(interpolation)
{
	int numComponents;
	stbi_set_flip_vertically_on_load(true);
	const char* pathData = path.c_str();
	unsigned char* data = stbi_load(pathData, &width, &height, &numComponents, 0);
	if (data != nullptr)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		//Set alignment to be 1

		bind();

		//Set number of components by format of the texture
		GLuint format = GL_RGB;
		switch (numComponents)
		{
		case 4:
			std::cout << "RGBA" << std::endl;
			format = GL_RGBA;
			break;
		case 3:
			std::cout << "RGB" << std::endl;
			format = GL_RGB;
			break;
		case 2:
			std::cout << "RG" << std::endl;
			format = GL_RG;
			break;
		case 1:
			std::cout << "R" << std::endl;
			format = GL_RED;
			break;
		default:
			std::cout << "Invalid Texture Format" << std::endl;
			break;
		};
		//Loads texture data into bound texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);

		// Clean up
		unbind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	//Return to default alignment
		stbi_image_free(data);

	}
	else {
		std::cout << "Failed" << std::endl;
		throw std::runtime_error("Failed to read texture data from file!");
	}
}
