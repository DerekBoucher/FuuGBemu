#include "opengl/Shader.hpp"


Shader::Shader(std::string filepath) {
    std::fstream fileStream = std::fstream(filepath.c_str(), std::ios_base::in);
    if (fileStream.fail()) {
        fprintf(stderr, "error opening shader source file %s\n", filepath.c_str());
        exit(-1);
    }

    std::stringstream ss;
    GLenum shaderType = 0;

    for(std::string line; std::getline(fileStream, line);) {

        if (line.find("#shadertype vertex") != std::string::npos) {
            shaderType = GL_VERTEX_SHADER;
            continue;
        }

        if (line.find("#shadertype fragment") != std::string::npos) {
            shaderType = GL_FRAGMENT_SHADER;
            continue;
        }

        ss << line << std::endl;
    }

    if (shaderType == 0) {
        fprintf(stderr, "error creating shader: could not determine shader type\n");
        exit(EXIT_FAILURE);
    }

    std::string sourceCode = ss.str();
    const char * cstr = sourceCode.c_str();

    ID = glCreateShader(shaderType);
    glShaderSource(ID, 1, &cstr, nullptr);
    glCompileShader(ID);

    GLint success;
    glGetShaderiv(ID, GL_COMPILE_STATUS, &success);

    if (not success) {
        GLint length;
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &length);
        char message[length];
        glGetShaderInfoLog(ID, length, &length, message);
        fprintf(stderr, "Error compiling %s shader: %s", shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment", message);
        exit(EXIT_FAILURE);
    }
}

Shader::~Shader() {
    glDeleteShader(ID);
}
