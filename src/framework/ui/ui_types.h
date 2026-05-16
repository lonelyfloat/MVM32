#ifndef H_UI_TYPES
#define H_UI_TYPES

#define _UI_STR(x) #x
#define _MAKE_UI_FIELD(type, name, ...) _UI_##type(#name, &_value->name); 
#define UI_GEN_HEADER(T,...) void _UI_##T(char* name, void* value);
#define UI_GEN_COMPONENT_HEADER(T,...) void _UI_Component_##T(void* value);


#define UI_GEN_(T,X)\
void _UI_Component_##T(void* value){ \
    T* _value = value;\
    ImGui_SeparatorText(#T);\
    ImGui_PushIDPtr(value);\
    X(_MAKE_UI_FIELD)\
    ImGui_PopID();\
}\

#define _ADD_DOT(t) .t
#define _FIELD_STRS(type, name, ...) _UI_STR(name) _UI_STR(|) _UI_STR(type),
#define _SELECT_UI_FIELD(type, name, enumname,...) case enumname: _UI_##type(buf, &_value->data.name); break;

#define UI_GEN_TAGGED_UNION(T,X)\
void _UI_Component_##T(void* value){ \
    T* _value = value;\
    ImGui_SeparatorText(#T);\
    int selectedField = _value->type;\
    const char* fieldStrs[] = { X(_FIELD_STRS) };\
    const char* id = _UI_STR(T##Type);\
    static char name[32] = _UI_STR(T##Type);\
    char buf[64];\
    sprintf(buf, "%s###Button", fieldStrs[selectedField]);\
    if(ImGui_Button(buf))\
        ImGui_OpenPopup(id, ImGuiPopupFlags_None);\
    if(ImGui_BeginPopup(id, ImGuiWindowFlags_None)) {\
        ImGui_SeparatorText(id);\
        for(int i = 0; i < IM_COUNTOF(fieldStrs); i++) {\
            if (ImGui_Selectable(fieldStrs[i])) {\
                selectedField = i;\
            }\
        }\
        ImGui_EndPopup();\
    }\
    if(selectedField > -1 && selectedField < X##_TYPE_COUNT) {\
        _value->type = selectedField;\
    }\
    sprintf(buf, "%s", fieldStrs[selectedField]);\
    ImGui_PushIDPtr(value);\
    switch(_value->type) {\
        X(_SELECT_UI_FIELD)\
        default:break;\
    }\
    ImGui_PopID();\
}\

UI_GEN_HEADER(float);
UI_GEN_HEADER(int);

#endif
