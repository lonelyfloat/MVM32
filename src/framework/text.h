#ifndef H_TEXT
#define H_TEXT

#include "component.h"

typedef struct Text {
    Arena* location;
} Text;

void _Save_Text(FILE* stream, void* value);
void _Load_Text(FILE* stream, Arena* arena, void* value);
void _UI_Text(char* name, void* value);
void _Allocate_Text(Arena* arena, void* value);

#endif 
