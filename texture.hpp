#pragma once

#include <string>

using std::string;

enum Texture_Type{
	TEXTURE_DIFFUSE,
	TEXTURE_SPECULAR,
	TEXTURE_NORMAL,
	TEXTURE_HEIGHT,
};

class Texture {
public:
	unsigned int ID;
	Texture_Type type;
	string path;
	string name;
	
	Texture() = default;

	Texture(Texture_Type tType, string tPath, string tName);
};
