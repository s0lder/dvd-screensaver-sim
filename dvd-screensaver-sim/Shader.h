#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:	
	unsigned int ID;

public:
	Shader(const char* vertex_path, const char* fragment_path);
	
	void Use();
	void Delete();
	void SetBool(const std::string& name, bool value);
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
};

