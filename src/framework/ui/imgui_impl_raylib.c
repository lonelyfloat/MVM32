#include "imgui_impl_raylib.h"
#include <stdio.h>
#include "dear_bindings/dcimgui.h"
#include "raylib/raylib.h"
#include "arena.h"

#include "imgui_impl_raylib.h"

#include <raylib/raylib.h>
#include <raylib/rlgl.h>

#include "dear_bindings/dcimgui.h"

#define MAX_TEXTURES 50


static const char* GetClipTextCallback(ImGuiContext* _ctx)
{
    return GetClipboardText();
}

static void SetClipTextCallback(ImGuiContext* _ctx, const char* text)
{
    SetClipboardText(text);
}

typedef struct ImGui_ImplRaylib_Data {

} ImGui_ImplRaylib_Data;

typedef union TexChunk {
    union TexChunk* next;
    char data[sizeof(Texture)];
} TexChunk;

typedef struct TexturePool {
    TexChunk* freeChunk;
    TexChunk* chunkArray;
} TexturePool;

TexturePool* CreatePool(size_t maxSize);
Texture* PoolAlloc(TexturePool* pool);
void PoolFree(TexturePool* pool, Texture* ptr);
void DestroyPool(TexturePool* pool);

static TexturePool* g_TexturePool;

static ImGui_ImplRaylib_Data g_BackendData = {};
static ImGuiMouseCursor g_CurrentCursor = 0;
static ImGuiMouseCursor g_LastFrameFocused = 0;
static ImGuiMouseCursor g_LastCtrl = 0;
static ImGuiMouseCursor g_LastShift = 0;
static ImGuiMouseCursor g_LastAlt = 0;
static ImGuiMouseCursor g_LastSuper = 0;

#define MAX_RAYLIB_KEY 349
static ImGuiKey g_raylibKeymap[MAX_RAYLIB_KEY];
static ImGuiKey g_mouseCursorMap[ImGuiMouseCursor_COUNT] = {
    MOUSE_CURSOR_ARROW,
    MOUSE_CURSOR_IBEAM,
    MOUSE_CURSOR_RESIZE_ALL,
    MOUSE_CURSOR_RESIZE_NS,
    MOUSE_CURSOR_RESIZE_EW, 
    MOUSE_CURSOR_RESIZE_NESW, 
    MOUSE_CURSOR_RESIZE_NWSE,
    MOUSE_CURSOR_POINTING_HAND,
    MOUSE_CURSOR_DEFAULT,
    MOUSE_CURSOR_DEFAULT,
    MOUSE_CURSOR_NOT_ALLOWED
};


void SetupKeymap() {
    g_raylibKeymap[KEY_APOSTROPHE] = ImGuiKey_Apostrophe;
    g_raylibKeymap[KEY_COMMA] = ImGuiKey_Comma;
    g_raylibKeymap[KEY_MINUS] = ImGuiKey_Minus;
    g_raylibKeymap[KEY_PERIOD] = ImGuiKey_Period;
    g_raylibKeymap[KEY_SLASH] = ImGuiKey_Slash;
    g_raylibKeymap[KEY_ZERO] = ImGuiKey_0;
    g_raylibKeymap[KEY_ONE] = ImGuiKey_1;
    g_raylibKeymap[KEY_TWO] = ImGuiKey_2;
    g_raylibKeymap[KEY_THREE] = ImGuiKey_3;
    g_raylibKeymap[KEY_FOUR] = ImGuiKey_4;
    g_raylibKeymap[KEY_FIVE] = ImGuiKey_5;
    g_raylibKeymap[KEY_SIX] = ImGuiKey_6;
    g_raylibKeymap[KEY_SEVEN] = ImGuiKey_7;
    g_raylibKeymap[KEY_EIGHT] = ImGuiKey_8;
    g_raylibKeymap[KEY_NINE] = ImGuiKey_9;
    g_raylibKeymap[KEY_SEMICOLON] = ImGuiKey_Semicolon;
    g_raylibKeymap[KEY_EQUAL] = ImGuiKey_Equal;
    g_raylibKeymap[KEY_A] = ImGuiKey_A;
    g_raylibKeymap[KEY_B] = ImGuiKey_B;
    g_raylibKeymap[KEY_C] = ImGuiKey_C;
    g_raylibKeymap[KEY_D] = ImGuiKey_D;
    g_raylibKeymap[KEY_E] = ImGuiKey_E;
    g_raylibKeymap[KEY_F] = ImGuiKey_F;
    g_raylibKeymap[KEY_G] = ImGuiKey_G;
    g_raylibKeymap[KEY_H] = ImGuiKey_H;
    g_raylibKeymap[KEY_I] = ImGuiKey_I;
    g_raylibKeymap[KEY_J] = ImGuiKey_J;
    g_raylibKeymap[KEY_K] = ImGuiKey_K;
    g_raylibKeymap[KEY_L] = ImGuiKey_L;
    g_raylibKeymap[KEY_M] = ImGuiKey_M;
    g_raylibKeymap[KEY_N] = ImGuiKey_N;
    g_raylibKeymap[KEY_O] = ImGuiKey_O;
    g_raylibKeymap[KEY_P] = ImGuiKey_P;
    g_raylibKeymap[KEY_Q] = ImGuiKey_Q;
    g_raylibKeymap[KEY_R] = ImGuiKey_R;
    g_raylibKeymap[KEY_S] = ImGuiKey_S;
    g_raylibKeymap[KEY_T] = ImGuiKey_T;
    g_raylibKeymap[KEY_U] = ImGuiKey_U;
    g_raylibKeymap[KEY_V] = ImGuiKey_V;
    g_raylibKeymap[KEY_W] = ImGuiKey_W;
    g_raylibKeymap[KEY_X] = ImGuiKey_X;
    g_raylibKeymap[KEY_Y] = ImGuiKey_Y;
    g_raylibKeymap[KEY_Z] = ImGuiKey_Z;
    g_raylibKeymap[KEY_LEFT_BRACKET] = ImGuiKey_LeftBracket;
    g_raylibKeymap[KEY_BACKSLASH] = ImGuiKey_Backslash;
    g_raylibKeymap[KEY_RIGHT_BRACKET] = ImGuiKey_RightBracket;
    g_raylibKeymap[KEY_GRAVE] = ImGuiKey_GraveAccent;
    g_raylibKeymap[KEY_SPACE] = ImGuiKey_Space;
    g_raylibKeymap[KEY_ESCAPE] = ImGuiKey_Escape;
    g_raylibKeymap[KEY_ENTER] = ImGuiKey_Enter;
    g_raylibKeymap[KEY_TAB] = ImGuiKey_Tab;
    g_raylibKeymap[KEY_BACKSPACE] = ImGuiKey_Backspace;
    g_raylibKeymap[KEY_INSERT] = ImGuiKey_Insert;
    g_raylibKeymap[KEY_DELETE] = ImGuiKey_Delete;
    g_raylibKeymap[KEY_RIGHT] = ImGuiKey_RightArrow;
    g_raylibKeymap[KEY_LEFT] = ImGuiKey_LeftArrow;
    g_raylibKeymap[KEY_DOWN] = ImGuiKey_DownArrow;
    g_raylibKeymap[KEY_UP] = ImGuiKey_UpArrow;
    g_raylibKeymap[KEY_PAGE_UP] = ImGuiKey_PageUp;
    g_raylibKeymap[KEY_PAGE_DOWN] = ImGuiKey_PageDown;
    g_raylibKeymap[KEY_HOME] = ImGuiKey_Home;
    g_raylibKeymap[KEY_END] = ImGuiKey_End;
    g_raylibKeymap[KEY_CAPS_LOCK] = ImGuiKey_CapsLock;
    g_raylibKeymap[KEY_SCROLL_LOCK] = ImGuiKey_ScrollLock;
    g_raylibKeymap[KEY_NUM_LOCK] = ImGuiKey_NumLock;
    g_raylibKeymap[KEY_PRINT_SCREEN] = ImGuiKey_PrintScreen;
    g_raylibKeymap[KEY_PAUSE] = ImGuiKey_Pause;
    g_raylibKeymap[KEY_F1] = ImGuiKey_F1;
    g_raylibKeymap[KEY_F2] = ImGuiKey_F2;
    g_raylibKeymap[KEY_F3] = ImGuiKey_F3;
    g_raylibKeymap[KEY_F4] = ImGuiKey_F4;
    g_raylibKeymap[KEY_F5] = ImGuiKey_F5;
    g_raylibKeymap[KEY_F6] = ImGuiKey_F6;
    g_raylibKeymap[KEY_F7] = ImGuiKey_F7;
    g_raylibKeymap[KEY_F8] = ImGuiKey_F8;
    g_raylibKeymap[KEY_F9] = ImGuiKey_F9;
    g_raylibKeymap[KEY_F10] = ImGuiKey_F10;
    g_raylibKeymap[KEY_F11] = ImGuiKey_F11;
    g_raylibKeymap[KEY_F12] = ImGuiKey_F12;
    g_raylibKeymap[KEY_LEFT_SHIFT] = ImGuiKey_LeftShift;
    g_raylibKeymap[KEY_LEFT_CONTROL] = ImGuiKey_LeftCtrl;
    g_raylibKeymap[KEY_LEFT_ALT] = ImGuiKey_LeftAlt;
    g_raylibKeymap[KEY_LEFT_SUPER] = ImGuiKey_LeftSuper;
    g_raylibKeymap[KEY_RIGHT_SHIFT] = ImGuiKey_RightShift;
    g_raylibKeymap[KEY_RIGHT_CONTROL] = ImGuiKey_RightCtrl;
    g_raylibKeymap[KEY_RIGHT_ALT] = ImGuiKey_RightAlt;
    g_raylibKeymap[KEY_RIGHT_SUPER] = ImGuiKey_RightSuper;
    g_raylibKeymap[KEY_KB_MENU] = ImGuiKey_Menu;
    g_raylibKeymap[KEY_KP_0] = ImGuiKey_Keypad0;
    g_raylibKeymap[KEY_KP_1] = ImGuiKey_Keypad1;
    g_raylibKeymap[KEY_KP_2] = ImGuiKey_Keypad2;
    g_raylibKeymap[KEY_KP_3] = ImGuiKey_Keypad3;
    g_raylibKeymap[KEY_KP_4] = ImGuiKey_Keypad4;
    g_raylibKeymap[KEY_KP_5] = ImGuiKey_Keypad5;
    g_raylibKeymap[KEY_KP_6] = ImGuiKey_Keypad6;
    g_raylibKeymap[KEY_KP_7] = ImGuiKey_Keypad7;
    g_raylibKeymap[KEY_KP_8] = ImGuiKey_Keypad8;
    g_raylibKeymap[KEY_KP_9] = ImGuiKey_Keypad9;
    g_raylibKeymap[KEY_KP_DECIMAL] = ImGuiKey_KeypadDecimal;
    g_raylibKeymap[KEY_KP_DIVIDE] = ImGuiKey_KeypadDivide;
    g_raylibKeymap[KEY_KP_MULTIPLY] = ImGuiKey_KeypadMultiply;
    g_raylibKeymap[KEY_KP_SUBTRACT] = ImGuiKey_KeypadSubtract;
    g_raylibKeymap[KEY_KP_ADD] = ImGuiKey_KeypadAdd;
    g_raylibKeymap[KEY_KP_ENTER] = ImGuiKey_KeypadEnter;
    g_raylibKeymap[KEY_KP_EQUAL] = ImGuiKey_KeypadEqual;
}

void ImGui_ImplRaylib_Init() {
    ImGuiIO* io = ImGui_GetIO();
    // Set backend info
    io->BackendPlatformName = "imgui_impl_raylib";
    io->BackendPlatformUserData = &g_BackendData;
    // Set backend flags
	io->BackendFlags |= ImGuiBackendFlags_HasGamepad | ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_RendererHasTextures;
    SetupKeymap();

    ImGuiPlatformIO* platformIO = ImGui_GetPlatformIO();
    platformIO->Platform_SetClipboardTextFn = SetClipTextCallback;
    platformIO->Platform_GetClipboardTextFn = GetClipTextCallback;
    platformIO->Platform_ClipboardUserData = NULL;

    g_TexturePool = CreatePool(MAX_TEXTURES);
}

MouseCursor ImGuiToRaylibCursor(ImGuiMouseCursor cursor) {
    if(cursor < 0 || cursor >= ImGuiMouseCursor_COUNT) return MOUSE_CURSOR_DEFAULT;
    return g_mouseCursorMap[cursor];
}

void SetMouseEvent(ImGuiIO* io, int rayMouse, int imguiMouse) {
    if(IsMouseButtonPressed(rayMouse))
        ImGuiIO_AddMouseButtonEvent(io, imguiMouse, true);
    if(IsMouseButtonReleased(rayMouse))
        ImGuiIO_AddMouseButtonEvent(io, imguiMouse, false);
}

void SetGamepadButtonEvent(ImGuiIO* io, int rayButton, int imguiKey) {
    if(IsGamepadButtonPressed(0,rayButton))
        ImGuiIO_AddMouseButtonEvent(io, imguiKey, true);
    if(IsGamepadButtonReleased(0,rayButton))
        ImGuiIO_AddMouseButtonEvent(io, imguiKey, false);
}

void SetGamepadStickEvent(ImGuiIO* io, GamepadAxis axis, ImGuiKey negKey, ImGuiKey posKey) {
    const float deadzone = 0.20f;
    float axisValue = GetGamepadAxisMovement(0, axis);
    ImGuiIO_AddKeyAnalogEvent(io,negKey, axisValue < -deadzone, axisValue < -deadzone ? -axisValue : 0);
    ImGuiIO_AddKeyAnalogEvent(io,posKey, axisValue > deadzone, axisValue > deadzone ? axisValue : 0);
}


void ImGui_ImplRaylib_NewFrame() {
    ImGuiIO* io = ImGui_GetIO();
    // Update window properties
    io->DeltaTime = GetFrameTime();
    io->DisplaySize = (ImVec2){GetScreenWidth(), GetScreenHeight()};
#ifndef PLATFORM_DRM
    if(IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        io->DisplaySize = (ImVec2){GetMonitorWidth(monitor), GetMonitorHeight(monitor)};
    }
#endif
    Vector2 dpi = GetWindowScaleDPI();
    io->DisplayFramebufferScale = (ImVec2){dpi.x, dpi.y};
    // Update mouse cursor shape
    if(io->BackendFlags & ImGuiBackendFlags_HasMouseCursors) {
        if(!(io->ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) {
            ImGuiMouseCursor cursor = ImGui_GetMouseCursor();
            if(io->MouseDrawCursor) { // If platform wants ImGui to draw the in-app cursor
                g_CurrentCursor = -1;
                HideCursor();
            }
            else if(cursor != g_CurrentCursor) {
                g_CurrentCursor = cursor;
                if(cursor == ImGuiMouseCursor_None) {
                    HideCursor();
                } else {
                    ShowCursor();
                }
                SetMouseCursor(ImGuiToRaylibCursor(cursor));
            }
        }
    }
    // Input handling
    // Window focus
    bool focused = IsWindowFocused();
    if (focused != g_LastFrameFocused)
        ImGuiIO_AddFocusEvent(io, focused);
    g_LastFrameFocused = focused;

    // Modifier keys
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (ctrl != g_LastCtrl)
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, ctrl);
    g_LastCtrl = ctrl;
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (shift != g_LastShift)
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Shift, shift);
    g_LastShift = shift;
    bool alt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    if (alt != g_LastAlt)
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Alt, alt);
    g_LastAlt = alt;
    bool super = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
    if (super != g_LastSuper)
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Super, super);
    g_LastSuper = super;

    // Keymap event update
    for(int k = 0; k < MAX_RAYLIB_KEY; ++k) {
        ImGuiKey key = g_raylibKeymap[k];
        if(key == 0) continue;
        if(IsKeyPressed(k))
            ImGuiIO_AddKeyEvent(io, key, true);
        if(IsKeyReleased(k))
            ImGuiIO_AddKeyEvent(io, key, false);
    }

    // Add text input
    if(io->WantCaptureKeyboard) {
        char pressed = GetCharPressed();
        while (pressed != 0) {
            ImGuiIO_AddInputCharacter(io, pressed);
            pressed = GetCharPressed();
        }
    }

    // Mouse input 
    if(focused) {
        if(!io->WantSetMousePos) {
            ImGuiIO_AddMousePosEvent(io, GetMouseX(), GetMouseY());
        }
        SetMouseEvent(io,MOUSE_BUTTON_LEFT, ImGuiMouseButton_Left);
        SetMouseEvent(io,MOUSE_BUTTON_RIGHT, ImGuiMouseButton_Right);
        SetMouseEvent(io,MOUSE_BUTTON_MIDDLE, ImGuiMouseButton_Middle);
        SetMouseEvent(io,MOUSE_BUTTON_FORWARD, ImGuiMouseButton_Middle+1);
        SetMouseEvent(io,MOUSE_BUTTON_BACK, ImGuiMouseButton_Middle+2);

        Vector2 mouseWheel = GetMouseWheelMoveV();
        ImGuiIO_AddMouseWheelEvent(io, mouseWheel.x, mouseWheel.y);
    }

    // Gamepad input
    if (io->ConfigFlags & ImGuiConfigFlags_NavEnableGamepad && IsGamepadAvailable(0))
    {
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_FACE_UP, ImGuiKey_GamepadDpadUp);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_FACE_RIGHT, ImGuiKey_GamepadDpadRight);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_FACE_DOWN, ImGuiKey_GamepadDpadDown);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_FACE_LEFT, ImGuiKey_GamepadDpadLeft);

        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_FACE_UP, ImGuiKey_GamepadFaceUp);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, ImGuiKey_GamepadFaceLeft);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, ImGuiKey_GamepadFaceDown);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, ImGuiKey_GamepadFaceRight);

        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_TRIGGER_1, ImGuiKey_GamepadL1);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_TRIGGER_2, ImGuiKey_GamepadL2);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, ImGuiKey_GamepadR1);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_TRIGGER_2, ImGuiKey_GamepadR2);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_LEFT_THUMB, ImGuiKey_GamepadL3);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_RIGHT_THUMB, ImGuiKey_GamepadR3);

        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_MIDDLE_LEFT, ImGuiKey_GamepadStart);
        SetGamepadButtonEvent(io, GAMEPAD_BUTTON_MIDDLE_RIGHT, ImGuiKey_GamepadBack);

        SetGamepadStickEvent(io, GAMEPAD_AXIS_LEFT_X, ImGuiKey_GamepadLStickLeft, ImGuiKey_GamepadLStickRight);
        SetGamepadStickEvent(io, GAMEPAD_AXIS_LEFT_Y, ImGuiKey_GamepadLStickUp, ImGuiKey_GamepadLStickDown);

        SetGamepadStickEvent(io, GAMEPAD_AXIS_RIGHT_X, ImGuiKey_GamepadRStickLeft, ImGuiKey_GamepadRStickRight);
        SetGamepadStickEvent(io, GAMEPAD_AXIS_RIGHT_Y, ImGuiKey_GamepadRStickUp, ImGuiKey_GamepadRStickDown);
    }
}

void ImGui_ImplRaylib_UpdateTexture(ImTextureData* tex) {
    switch(tex->Status) {
        case ImTextureStatus_OK:
        case ImTextureStatus_Destroyed:
        default:
            break;
        case ImTextureStatus_WantCreate:
            {
            Image img = { 0 };
            img.width = tex->Width;
            img.height = tex->Height;

            img.format = tex->Format == ImTextureFormat_Alpha8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            img.mipmaps = 1;
            img.data = ImTextureData_GetPixels(tex);

            Texture* texture = PoolAlloc(g_TexturePool);
            tex->BackendUserData = texture;
            *texture = LoadTextureFromImage(img);
            ImTextureData_SetTexID(tex, (ImTextureID){texture->id});
            tex->Status = ImTextureStatus_OK;
            }
            break;
        case ImTextureStatus_WantUpdates:
            {
            Texture* texture = (Texture*)tex->BackendUserData;
            if (!texture)
                break;

            UpdateTexture(*texture, ImTextureData_GetPixels(tex));

            tex->Status = ImTextureStatus_OK;
            }
            break;
        case ImTextureStatus_WantDestroy:
            {
            Texture* texture = (Texture*)tex->BackendUserData;
            if (!texture)
                break;
            UnloadTexture(*texture);
            tex->Status = ImTextureStatus_Destroyed;
            PoolFree(g_TexturePool, texture);
            tex->BackendUserData = NULL;
            ImTextureData_SetTexID(tex,ImTextureID_Invalid);
            }
            break;
    }
}

static void ImGuiTriangleVert(const ImDrawVert* idxVert)
{
    Color c = {
        .r = idxVert->col >> 0,
        .g = idxVert->col >> 8,
        .b = idxVert->col >> 16,
        .a = idxVert->col >> 24,


    };
    
    rlColor4ub(c.r, c.g, c.b, c.a);
    rlTexCoord2f(idxVert->uv.x, idxVert->uv.y);
    rlVertex2f(idxVert->pos.x, idxVert->pos.y);
}

static void ImGuiRenderTriangles(unsigned int count, int indexStart, const ImVector_ImDrawIdx* indexBuffer, const ImVector_ImDrawVert* vertBuffer, ImTextureID texID) {
    if (count < 3)
        return;

    unsigned int textureId = texID;

    rlBegin(RL_TRIANGLES);
    rlSetTexture(textureId);

    for (unsigned int i = 0; i <= (count - 3); i += 3)
    {
        ImDrawIdx indexA = indexBuffer->Data[indexStart + i];
        ImDrawIdx indexB = indexBuffer->Data[indexStart + i + 1];
        ImDrawIdx indexC = indexBuffer->Data[indexStart + i + 2];

        ImDrawVert vertexA = vertBuffer->Data[indexA];
        ImDrawVert vertexB = vertBuffer->Data[indexB];
        ImDrawVert vertexC = vertBuffer->Data[indexC];

        ImGuiTriangleVert(&vertexA);
        ImGuiTriangleVert(&vertexB);
        ImGuiTriangleVert(&vertexC);
    }
    rlEnd();
}

void EnableScissor(int x, int y, int w, int h) {
    rlEnableScissorTest();
    ImGuiIO* io = ImGui_GetIO();
    ImVec2 scale = io->DisplayFramebufferScale;

    rlScissor((int)(x * scale.x),
        (int)((io->DisplaySize.y - (int)(y + h)) * scale.y),
        (int)(w * scale.x),
        (int)(h * scale.y));

}

void ImGui_ImplRaylib_RenderDrawData(ImDrawData* drawData) {
    if(drawData->Textures != NULL) {
        for(int i = 0; i < drawData->Textures->Size; ++i) {
            ImGui_ImplRaylib_UpdateTexture(drawData->Textures->Data[i]);
        }
    }

    // TODO: the rest of the f*cking renderer, lol
    rlDrawRenderBatchActive();
    rlDisableBackfaceCulling();

    for(int i = 0; i < drawData->CmdLists.Size; ++i) {
        const ImDrawList* cmdList = drawData->CmdLists.Data[i];
        for(int j = 0; j < cmdList->CmdBuffer.Size; ++j) {
            ImDrawCmd* cmd = &cmdList->CmdBuffer.Data[j];
            EnableScissor(cmd->ClipRect.x - drawData->DisplayPos.x,
                cmd->ClipRect.y - drawData->DisplayPos.y,
                cmd->ClipRect.z - (cmd->ClipRect.x - drawData->DisplayPos.x),
                cmd->ClipRect.w - (cmd->ClipRect.y - drawData->DisplayPos.y));
            if(cmd->UserCallbackData != NULL) {
                cmd->UserCallback(cmdList, cmd);
                continue;
            }
            // triangle
            ImGuiRenderTriangles(cmd->ElemCount, cmd->IdxOffset, &cmdList->IdxBuffer, &cmdList->VtxBuffer, ImDrawCmd_GetTexID(cmd));
            rlDrawRenderBatchActive();
        }
    }
    rlSetTexture(0);
    rlDisableScissorTest();
    rlEnableBackfaceCulling();
}

void ImGui_ImplRaylib_Shutdown(void) {
    DestroyPool(g_TexturePool);
    ImGuiIO* io = ImGui_GetIO();
    io->BackendPlatformUserData = NULL;
}

// Texture pool impl
TexturePool* CreatePool(size_t maxSize) {
    TexturePool* pool = malloc(sizeof(TexturePool));
    if(pool == NULL) 
        return NULL;
    pool->chunkArray = malloc(maxSize * sizeof(TexChunk));
    pool->freeChunk = pool->chunkArray;
    if(pool->chunkArray == NULL) {
        free(pool);
        return NULL;
    }

    for(int i = 0; i < maxSize-1; ++i) {
        pool->chunkArray[i].next = &pool->chunkArray[i+1];
    }
    pool->chunkArray[maxSize-1].next = NULL;
    return pool;
}

Texture* PoolAlloc(TexturePool* pool) {
    if(pool == NULL || pool->freeChunk == NULL) 
        return NULL;
    TexChunk* new  = pool->freeChunk;
    pool->freeChunk = pool->freeChunk->next;
    return (Texture*)new;
}

void PoolFree(TexturePool* pool, Texture* ptr) {
    if(pool == NULL || pool->freeChunk == NULL) 
        return;
    TexChunk* freed = (TexChunk*)ptr;
    freed->next = pool->freeChunk;
    pool->freeChunk = freed;

}

void DestroyPool(TexturePool* pool) {
    if(pool == NULL)
        return;

    free(pool->chunkArray);
    free(pool);
}
