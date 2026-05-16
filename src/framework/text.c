#include "text.h"
#include <string.h>

void _Save_Text(FILE* stream, void* value) {
    Text* text = value;
    fprintf(stream, "%ld|%ld|", text->location->capacity, text->location->count);
    for(int i = 0; i < text->location->count; ++i) {
        fprintf(stream, "%c", text->location->data[i]);
    }
    fprintf(stream, " ");
}

void _Load_Text(FILE* stream, Arena* arena, void* value) {
    Text* text = value;
    printf("ALIVE\n");
    size_t capacity;
    size_t count;
    fscanf(stream, "%ld|%ld|", &capacity,&count);
    
    text->location = GrabSection(arena, capacity);
    for(int i = 0; i < count; ++i) {
        fscanf(stream, "%c", &text->location->data[i]);
    }
    text->location->count = count;
    text->location->data[text->location->count] = '\0';
    fscanf(stream, " ");
}

void _UI_Text(char* name, void* value) {
    Text* asset = value;
    if(asset->location != NULL) {
        ImGui_InputTextMultiline(name, asset->location->data, asset->location->capacity);
        asset->location->count = strlen(asset->location->data);
    }
}

#define MAX_TEXT_CAP 300
void _Allocate_Text(Arena* arena, void* value) {
    Text* text = value;
    text->location = GrabSection(arena, MAX_TEXT_CAP);
}
