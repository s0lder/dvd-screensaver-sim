#include "Shader.h"

Shader::Shader(const char* vertex_path, const char* fragment_path)
{
	// retrieve fragment, vertex source from file paths
	// error handling optional
	// ------------------------------------------------
	std::string vertex_code, fragment_code;
	std::ifstream vertex_file, fragment_file;
	std::stringstream vertex_stream, fragment_stream;

	// open files
	vertex_file.open(vertex_path);
	fragment_file.open(fragment_path);

	// read file into streams
	vertex_stream << vertex_file.rdbuf();
	fragment_stream << fragment_file.rdbuf();

	// close files
	vertex_file.close();
	fragment_file.close();

	// convert stream into string
	vertex_code = vertex_stream.str();
	fragment_code = fragment_stream.str();

	// convert string into cstring
	const char* vertex_code_cstr = vertex_code.c_str();
	const char* fragment_code_cstr = fragment_code.c_str();

	// compile shaders
	// error handling optional
	// ---------------
	unsigned int vertex, fragment;

	// vertex
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertex_code_cstr, NULL);
	glCompileShader(vertex);

	// fragment
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragment_code_cstr, NULL);
	glCompileShader(fragment);

	// create program and link shaders
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	// delete shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
