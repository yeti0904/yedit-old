#pragma once
#include <unistd.h>
char fexists(char* filename);
char* readfile(char* filename);
int getfilesize(char* filename);
void writefile(char* filename, char* toWrite);