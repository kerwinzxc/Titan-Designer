#include "Shader.h"

#include <string>
#include <algorithm>
#include <fstream>

#include <GL\glew.h>
#include <GL\freeglut.h>
#include "SDL.h"

#include "math/Math.h"
#include "math/Color.h"
#include "String.h"

#include "core/TMessage.h"

#include "core/ContentManager.h"

#define MAX_LOG_LENGTH 1000

Shader::Shader(const String &p_path)
{
	program_id = -1;
	vertexshader_id = -1;
	fragmentshader_id = -1;
	geometryshader_id = -1;

	vertex_path = p_path + ".vert";
	fragment_path = p_path + ".frag";
	geometry_path = p_path + ".geom";

	file = p_path;

	load();
}

Shader::~Shader()
{
	free();
}

void Shader::load()
{
	vertexshader_id = create_shader(vertex_path, GL_VERTEX_SHADER);

	if (vertexshader_id != -1)
		fragmentshader_id = create_shader(fragment_path, GL_FRAGMENT_SHADER);

	if (has_geometry_shader() && fragmentshader_id != -1)
		geometryshader_id = create_shader(geometry_path, GL_GEOMETRY_SHADER);

	isvalid = vertexshader_id != -1 && fragmentshader_id != -1;

	if (isvalid)
		create_program();
	else
		T_ERROR("Failed to compile: " + get_file() + "!");
}

void Shader::reload()
{
	free();
	load();
}

void Shader::free()
{
	CONTENT->free_textfile(vertex_path);
	CONTENT->free_textfile(fragment_path);
	CONTENT->free_textfile(geometry_path);

	if (!isvalid)
		return;

	glDeleteShader(vertexshader_id);
	glDeleteShader(fragmentshader_id);

	if (geometryshader_id > -1)
		glDeleteShader(geometryshader_id);

	glDeleteProgram(program_id);
}

void Shader::start()
{
    glUseProgram(program_id);
}

GLint Shader::create_shader(const String& p_path, GLenum ShaderType)
{
    GLint ShaderID = glCreateShader(ShaderType);
    GLchar infolog[MAX_LOG_LENGTH];
    const GLchar* shadersource;
    GLint infologlength;
    GLint compilestatus;

	TextFile* file = CONTENT->LoadTextFile(p_path);

	String s = file->get_source();
	shadersource = s.c_str();

    glShaderSource(ShaderID, 1, &shadersource, NULL);
    glCompileShader(ShaderID);
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &compilestatus);

    if (compilestatus != GL_TRUE)
    {
		glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &infologlength);
        glGetShaderInfoLog(ShaderID, MAX_LOG_LENGTH, NULL, infolog);

		String err = "Failed to compile: ";
		err += p_path;
		err += "\n\nStart InfoLog:\n\n";
		err += infolog;
		err += "End InfoLog\n";

		T_ERROR(err);

		ShaderID = -1;
    }
    return ShaderID;
}

void Shader::create_program()
{
    GLint linkStatus, infologlength;
    GLchar *infolog;
    program_id = glCreateProgram();

    glAttachShader(program_id, vertexshader_id);
    glAttachShader(program_id, fragmentshader_id);

	if (geometryshader_id != -1)
		glAttachShader(program_id, geometryshader_id);

	glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE)
    {
        T_ERROR("Program link failed!");

        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &infologlength);
        infolog = new GLchar(infologlength + 1);
        glGetProgramInfoLog(program_id, infologlength + 1, NULL, infolog);

        SDL_Log(infolog);
        delete infolog;
		return;
    } 
	else
    	 glUseProgram(program_id);

    set_info();
}

void Shader::bind()
{
	if (isvalid)
		glUseProgram(program_id);
}
void Shader::unbind()
{
	glUseProgram(0);
}

bool Shader::has_geometry_shader()
{
	return geometry_path.is_file();
}

void Shader::set_info()
{
	Uniform uni;
    GLint count = 0;

    glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &count);

    for (GLint c = 0; c < count; c++)
    {
        glGetActiveUniform(program_id, c, sizeof(uni.name) - 1, &uni.length, &uni.size, &uni.type, uni.name);
        uni.location = glGetUniformLocation(program_id, uni.name);
        uni.index = c;
        uniforms[uni.name] = uni;
    }

	Block block;

	glGetProgramiv(program_id, GL_ACTIVE_UNIFORM_BLOCKS, &count);

	for (GLint c = 0; c < count; c++)
	{
		glGetActiveUniformBlockName(program_id, c, sizeof(block.name) - 1, &block.length, block.name);
		block.location = glGetUniformBlockIndex(program_id, block.name);
		block.index = c;
		blocks[block.name] = block;
	}
}

#if 0
#define CHECK_NAME \
if (uniforms[name].count == 0)\
	return;
#endif

#define CHECK_NAME \
if (!uniforms.contains(name))\
{\
	T_WARNING("uniform with name: " + name + " does not exist in shader: " + get_file().get_name());\
	return;\
}

void Shader::set_uniform(const String &name, bool value)
{
	CHECK_NAME
	
    glUniform1i(uniforms[name].location, value ? 1 : 0);
}

void Shader::set_uniform(const String &name, const int value)
{
	CHECK_NAME

    glUniform1i(uniforms[name].location, value);
}

void Shader::set_uniform(const String &name, const float value)
{
	CHECK_NAME

	glUniform1f(uniforms[name].location, value);
}

void Shader::set_uniform(const String &name, const double value) //BETA
{
	CHECK_NAME

	glUniform1f(uniforms[name].location, (float)value);
}

void Shader::set_uniform(const String &name, const vec2 &value)
{
	CHECK_NAME

    glUniform2f(uniforms[name].location, value.x, value.y);
}

void Shader::set_uniform(const String &name, const vec2i &value)
{
	CHECK_NAME

	glUniform2i(uniforms[name].location, value.x, value.y);
}

void Shader::set_uniform(const String &name, const vec3 &value)
{
	CHECK_NAME

    glUniform3f(uniforms[name].location, value.x, value.y, value.z);
}

void Shader::set_uniform(const String &name, const vec3i &value)
{
	CHECK_NAME

	glUniform3i(uniforms[name].location, value.x, value.y, value.z);
}

void Shader::set_uniform(const String &name, const vec4 &value)
{
	CHECK_NAME

    glUniform4f(uniforms[name].location, value.x, value.y, value.z, value.w);
}

void Shader::set_uniform(const String &name, const vec4i &value)
{
	CHECK_NAME

	glUniform4i(uniforms[name].location, value.x, value.y, value.z, value.w);
}
void Shader::set_uniform(const String &name, const Color &value)
{
	CHECK_NAME

	glUniform4f(uniforms[name].location, value.x, value.y, value.z, value.w);
}

void Shader::set_uniform(const String &name, const mat4 &value)
{
	CHECK_NAME

    glUniformMatrix4fv(uniforms[name].location, 1, false, &value.m[0]);
}

void Shader::set_uniform(const String &name, const Array<mat4> &value)
{
	CHECK_NAME

	glUniformMatrix4fv(uniforms[name].location, value.size(), false, &(&value[0])->m[0]);
}

void Shader::bind_buffer(const String& p_name, Ref<UBO> p_ubo)
{
	glUniformBlockBinding(program_id, blocks[p_name].location, p_ubo->get_bound_index());
}

int Shader::get_program() const
{
	return program_id;
}