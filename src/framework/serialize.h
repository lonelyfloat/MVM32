#ifndef H_SERIALIZE
#define H_SERIALIZE

// A bunch of pre-defined serialization functions for primitive types & raylib types for use in the ECS
// + A bunch of macros for generating these functions on any type

#include <stdio.h>
#include "ecs.h"

// Serialization for ECS structure

void LoadEntitiesFromFile(ECS* ecs, Arena* arena, char* filePath);
void SaveEntitiesToFile(ECS* ecs, char* filePath);


#define _MAKE_SERIALIZER(type, name, ...) _Save_##type(stream, &value->name);
#define _MAKE_TAGGED_UNION_SERIALIZER(type, name, enumname,...) case enumname: _Save_##type(stream, &value->data.name); break;
#define _MAKE_DESERIALIZER(type, name, ...)  _Load_##type(stream, arena, &value->name);
#define _MAKE_TAGGED_UNION_DESERIALIZER(type, name, enumname,...) case enumname: _Load_##type(stream, arena, &value->data.name); break;

#define SERIALIZE_GEN_PRIM(type, str)\
void _Save_##type(FILE* stream, void* value) {\
    fprintf(stream, str, *((type*)value));\
}\
\
void _Load_##type(FILE* stream, Arena* arena, void* value) {\
    fscanf(stream, str, (type*)value);\
}\

// Takes an X-macro table as "X"
#define SERIALIZE_GEN_(T,X)\
void _Save_##T(FILE* stream, void* _value){ \
    T* value = _value;\
    fprintf(stream, "(");\
    X(_MAKE_SERIALIZER)\
    fprintf(stream, ")");\
}\
void _Load_##T(FILE* stream, Arena* arena, void* _value) {\
    T* value = _value;\
    fscanf(stream, "(");\
    X(_MAKE_DESERIALIZER)\
    fscanf(stream, ")");\
}

#define SERIALIZE_GEN_TAGGED_UNION(T,X)\
void _Save_##T(FILE* stream, void* _value){ \
    T* value = _value;\
    fprintf(stream, "(");\
    fprintf(stream, "%u ", value->type);\
    switch(value->type) {\
    X(_MAKE_TAGGED_UNION_SERIALIZER)\
    default:break;\
    }\
    fprintf(stream, ")");\
}\
void _Load_##T(FILE* stream, Arena* arena,void* _value) {\
    T* value = _value;\
    fscanf(stream, "(");\
    fscanf(stream, "%u ", &value->type);\
    switch(value->type){\
    X(_MAKE_TAGGED_UNION_DESERIALIZER)\
    default:break;\
    }\
    fscanf(stream, ")");\
}


#define SERIALIZE_GEN_HEADER(type)\
void _Save_##type(FILE* stream, void* value);\
void _Load_##type(FILE* stream, Arena* arena, void* value);

SERIALIZE_GEN_HEADER(float)
SERIALIZE_GEN_HEADER(int)
SERIALIZE_GEN_HEADER(bool)
SERIALIZE_GEN_HEADER(Entity)
SERIALIZE_GEN_HEADER(Vector2)
SERIALIZE_GEN_HEADER(Rectangle)
SERIALIZE_GEN_HEADER(Color)

#endif
