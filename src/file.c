#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char fexists(char* filename) {
	return (access(filename, F_OK) == 0);
}

char* readfile(char* filename) {
	FILE *fr;
	fr = fopen(filename,"r");

	fseek(fr, 0, SEEK_END); // seek to the end to find filesize
	int filesize = ftell(fr);
	fseek(fr, 0, SEEK_SET); // seek back to read file
	char* toReturn = (char*) malloc(filesize);

	char c; // used to read characters from the file
	for (int i = 0; (c=getc(fr)) != EOF; ++i) {
		toReturn[i] = c;
	}
	fclose(fr);
	return toReturn;
}

int getfilesize(char* filename) {
	FILE *fr;
	fr = fopen(filename,"r");
	fseek(fr, 0, SEEK_END);
	return ftell(fr);
}

void writefile(char* filename, char* toWrite) {
	FILE *fr;
	fr = fopen(filename, "w+");
	fputs(toWrite, fr);
	fclose(fr);
}