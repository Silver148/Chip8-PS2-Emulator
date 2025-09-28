#ifndef EXPLORER_H
#define EXPLORER_H

#include "main.h"

int ReadDir(char *path);
char* DeletePrefix(char *str, const char *prefix, char *out);

#endif