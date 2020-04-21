#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

GLuint loadBMP_custom(const char* imagepath);

GLuint loadDDS(const char* imagepath);

GLuint loadTexture(const char* imagepath);
#endif