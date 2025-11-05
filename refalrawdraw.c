#include <android_native_app_glue.h>
#include <stddef.h>
#include <stdio.h>

// Make it so we don't need to include any other C files in our build.
#define CNFG_IMPLEMENTATION

// Optional: Use OpenGL to do rendering on appropriate systems.
#define CNFGOGL

#include "CNFG.h"
#include "refal05rts.h"

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

struct signed_number {
    signed sign;
    r05_number value;
};

static struct r05_node* parse_signed_number(
    struct signed_number* sn,
    struct r05_node* p
) {
    if (R05_DATATAG_CHAR == p->tag) {
        if ('-' == p->info.char_) {
            sn->sign = -1;
        } else if ('+' == p->info.char_) {
            sn->sign = +1;
        } else {
            r05_recognition_impossible();
        }

        p = p->next;
    } else {
        sn->sign = +1;
    }

    if (R05_DATATAG_NUMBER != p->tag) {
        r05_recognition_impossible();
    }

    sn->value = p->info.number;

    if (0 == sn->value) {
        sn->sign = +1;
    }

    return p->next;
}

static size_t parse_chars_in_brackets(
    struct r05_node** chars, char buffer[], size_t buflen,
    struct r05_node* open_bracket
) {
    if (open_bracket->tag != R05_DATATAG_OPEN_BRACKET) {
        r05_recognition_impossible();
    }

    size_t nread = 0;
    struct r05_node* cur = chars[0] = open_bracket->next;
    while (nread < buflen && R05_DATATAG_CHAR == cur->tag) {
        buffer[nread] = cur->info.char_;
        ++nread;
        cur = cur->next;
    }
    if (cur->tag != R05_DATATAG_CLOSE_BRACKET) {
        r05_recognition_impossible();
    }

    chars[1] = cur->prev;
    return nread;
}

#define MAX_TITLE_LEN 1024

// R05_DEFINE_ENTRY_FUNCTION(CNFGSetupFullscreen, "CNFGSetupFullscreen") {
//     // struct r05_node *eTitle[2];
//     // char title[MAX_TITLE_LEN];
//     // size_t title_len = parse_chars_in_brackets(eTitle, title, MAX_TITLE_LEN, arg_begin->next);
//     CNFGSetupFullscreen("Refal", 0);

//     r05_splice_to_freelist(arg_begin, arg_end);
// }

R05_DEFINE_ENTRY_FUNCTION(DebugLoop, "DebugLoop") {
    printf("Application Started 69!\n");
    
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

    r05_splice_to_freelist(arg_begin, arg_end);
}

// int main(int argc, char** argv) {
//     CNFGSetupFullscreen("Refal", 0);

//     while (CNFGHandleInput()) {
//         if (suspended) {
//             usleep(50000);
//             continue;
//         }

//         CNFGBGColor = 0x000080ff; // Dark Blue Background

//         short w, h;
//         CNFGClearFrame();
//         CNFGGetDimensions(&w, &h); // required to initialize screen for rendering

//         // render user input text
//         {
//             CNFGColor(0xffffffff);
//             CNFGPenX = 16;
//             CNFGPenY = 16;
//             CNFGDrawText(text, 16);
//         }

//         CNFGSwapBuffers();
//     }
// }
