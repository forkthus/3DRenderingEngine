#pragma once

#include <string>
#include <vector>

using std::string, std::vector;

enum Texture_Type{
	TEXTURE_DIFFUSE,
	TEXTURE_SPECULAR,
	TEXTURE_NORMAL,
	TEXTURE_HEIGHT,
	TEXTURE_CUBE_MAP
};

class Texture {
public:
	unsigned int ID;
	Texture_Type type;
	string path;
	string name;
	
	Texture() = default;

	Texture(Texture_Type tType, string tPath, string tName="");

	Texture(Texture_Type, vector<string> tPaths, string tName="");

	~Texture();
};
