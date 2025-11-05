#include <android_native_app_glue.h>
#include <stdio.h>

// Make it so we don't need to include any other C files in our build.
#define CNFG_IMPLEMENTATION

// Optional: Use OpenGL to do rendering on appropriate systems.
#define CNFGOGL

#include "CNFG.h"

#define KEYCODE_DEL 67

void AndroidDisplayKeyboard(int pShow);

void HandleMotion(int x, int y, int mask) { }

static char text[1024];
static char* text_ptr = text;

int HandleDestroy() {
    printf("Destroying\n");
    return 0;
}

volatile int suspended;
volatile int keyboard_up = 0;

void HandleSuspend() {
    suspended = 1;
}

void HandleResume() {
    suspended = 0;
}

void HandleKey(int keycode, int bDown) {
    printf("Key: keycode=%d bDown=%d\n", keycode, bDown);

    if (bDown) {
        if (keycode == KEYCODE_DEL) {
            if (text_ptr != text) {
                text_ptr--;
                *text_ptr = '\0';
            }
        } else {
            *text_ptr = (char)keycode;
            text_ptr++;
            *text_ptr = '\0';
        }
    }
}

void HandleButton(int x, int y, int button, int bDown) {
    printf("Button: (x=%d, y=%d) button=%d bDown=%d \n", x, y, button, bDown);

    if (button == 0) { // single touch
        if (!bDown) { // released
            keyboard_up = !keyboard_up;
            AndroidDisplayKeyboard(keyboard_up);
        }
    }
}

int main(int argc, char** argv) {
    CNFGSetupFullscreen("Refal", 0);

    while (CNFGHandleInput()) {
        if (suspended) {
            usleep(50000);
            continue;
        }

        CNFGBGColor = 0x000080ff; // Dark Blue Background

        short w, h;
        CNFGClearFrame();
        CNFGGetDimensions(&w, &h); // required to initialize screen for rendering

        // render user input text
        {
            CNFGColor(0xffffffff);
            CNFGPenX = 16;
            CNFGPenY = 16;
            CNFGDrawText(text, 16);
        }

        CNFGSwapBuffers();
    }
}
