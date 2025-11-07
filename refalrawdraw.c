#include <android_native_app_glue.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CNFG_IMPLEMENTATION
#define CNFGOGL

#include "CNFG.h"
#include "refal05rts.h"

volatile int suspended;

typedef struct {
    int x;
    int y;
    int button;
    int bDown;
} ButtonEvent;

ButtonEvent last_button_event = {};
bool last_button_event_processed = true;

int HandleDestroy() {
    printf("Destroying\n");
    return 0;
}

void HandleSuspend() {
    suspended = 1;
}

void HandleResume() {
    suspended = 0;
}

void HandleKey(int keycode, int bDown) {
    printf("Key: keycode=%d bDown=%d\n", keycode, bDown);
}

void HandleButton(int x, int y, int button, int bDown) {
    printf("Button: (x=%d, y=%d) button=%d bDown=%d \n", x, y, button, bDown);

    last_button_event_processed = false;
    last_button_event = (ButtonEvent) { .x = x, .y = y, .button = button, .bDown = bDown };
}

void HandleMotion(int x, int y, int mask) {
    printf("Motion: (x=%d, y=%d) mask=%d \n", x, y, mask);
}

#define signed_number_int(s) (int)((s).sign * (s).value)

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
    struct r05_node** brackets, char buffer[], size_t buflen,
    struct r05_node* open_bracket
) {
    if (open_bracket->tag != R05_DATATAG_OPEN_BRACKET) {
        r05_recognition_impossible();
    }

    brackets[0] = open_bracket;

    size_t nread = 0;
    struct r05_node* cur = open_bracket->next;
    while (nread < buflen && R05_DATATAG_CHAR == cur->tag) {
        buffer[nread] = cur->info.char_;
        ++nread;
        cur = cur->next;
    }
    if (cur->tag != R05_DATATAG_CLOSE_BRACKET) {
        r05_recognition_impossible();
    }

    brackets[1] = cur;
    return nread;
}

static int parse_hex_color(const char* color, int color_len) {
    if (!color || color_len != 9) {
        printf("# bad color length %d\n", color_len);
        r05_recognition_impossible();
    }

    if (color[0] != '#') {
        printf("# missed\n");
        r05_recognition_impossible();
    }

    int result = 0;

    for (int i = 1; i < 9; i++) {
        char c = color[i];
        int digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else {
            return -1;
        }

        result = (result << 4) | digit;
    }

    return result;
}

/**
 * Get list of button events.
 *
 * <RawDrawButtonEvents> == t.ButtonEvent*
 *
 * t.ButtonEvent = (s.X s.Y s.Button s.bDown)
 */
R05_DEFINE_ENTRY_FUNCTION(RawDrawButtonEvents, "RawDrawButtonEvents") {
    printf("RawDrawButtonEvents called from Refal\n");

    struct r05_node* callee = arg_begin->next;
    if (!r05_empty_hole(callee, arg_end)) {
        r05_recognition_impossible();
    }

    if (!last_button_event_processed) {
        last_button_event_processed = true;

        struct r05_node *left_bracket, *right_bracket;
        r05_reset_allocator();
        r05_alloc_open_bracket(&left_bracket);
        r05_alloc_number((r05_number)last_button_event.x);
        r05_alloc_number((r05_number)last_button_event.y);
        r05_alloc_number((r05_number)last_button_event.button);
        r05_alloc_number((r05_number)last_button_event.bDown);
        r05_alloc_close_bracket(&right_bracket);
        r05_link_brackets(left_bracket, right_bracket);
        r05_splice_from_freelist(arg_begin);
        r05_splice_to_freelist(arg_begin, arg_end);
    } else {
        r05_splice_to_freelist(arg_begin, arg_end);
    }
}

/**
 * <CNFGSetupFullscreen (e.WindowName) s.ScreenNumber> == empty
 *
 * e.WindowName = s.CHAR+
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGSetupFullscreen, "CNFGSetupFullscreen") {
    printf("CNFGSetupFullscreen called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct r05_node* tWindowName[2];
    char window_name[1024];
    int window_name_len = parse_chars_in_brackets(tWindowName, window_name, sizeof(window_name) - 1, callee->next);
    window_name[window_name_len] = '\0';

    struct signed_number screen_number;
    struct r05_node* sScreenNumber = parse_signed_number(&screen_number, tWindowName[1]->next)->prev;
    if (screen_number.sign == -1) {
        r05_recognition_impossible();
    }
    if (!r05_empty_hole(sScreenNumber, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGSetupFullscreen(window_name, (int)screen_number.value);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGHandleInput> == s.ShouldExit
 *
 * s.ShouldExit ::= 0 | 1
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGHandleInput, "CNFGHandleInput") {
    printf("CNFGHandleInput called from Refal\n");

    struct r05_node* callee = arg_begin->next;
    if (!r05_empty_hole(callee, arg_end)) {
        r05_recognition_impossible();
    }

    int should_exit = CNFGHandleInput();

    struct r05_node* sShouldExit = arg_begin;
    sShouldExit->tag = R05_DATATAG_NUMBER;
    sShouldExit->info.number = (r05_number)should_exit;
    r05_splice_to_freelist(sShouldExit->next, arg_end);
}

/**
 * <CNFGClearFrame> == empty
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGClearFrame, "CNFGClearFrame") {
    printf("CNFGClearFrame called from Refal\n");

    struct r05_node* callee = arg_begin->next;
    if (!r05_empty_hole(callee, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGClearFrame();

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGGetDimensions> == s.W s.H
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGGetDimensions, "CNFGGetDimensions") {
    printf("CNFGGetDimensions called from Refal\n");

    struct r05_node* callee = arg_begin->next;
    if (!r05_empty_hole(callee, arg_end)) {
        r05_recognition_impossible();
    }

    struct r05_node* sW = arg_begin;
    struct r05_node* sH = arg_begin->next;

    short w, h;
    CNFGGetDimensions(&w, &h);

    sW->tag = R05_DATATAG_NUMBER;
    sW->info.number = (r05_number)w;
    sH->tag = R05_DATATAG_NUMBER;
    sH->info.number = (r05_number)h;
    r05_splice_to_freelist(sH->next, arg_end);
}

/**
 * <CNFGSwapBuffers> == empty
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGSwapBuffers, "CNFGSwapBuffers") {
    printf("CNFGSwapBuffers called from Refal\n");

    struct r05_node* callee = arg_begin->next;
    if (!r05_empty_hole(callee, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGSwapBuffers();

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * Set CNFGBGColor color value.
 *
 * <CNFGSetBGColor e.Color> == empty
 *
 * e.Color = '#RRGGBBAA'
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGSetBGColor, "CNFGSetBGColor") {
    printf("CNFGSetBGColor called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct r05_node* eColor[2];
    char color[9];
    int color_len = r05_read_chars(eColor, color, sizeof(color), callee, arg_end);

    if (!r05_empty_hole(eColor[1], arg_end)) {
        r05_recognition_impossible();
    }

    CNFGBGColor = parse_hex_color(color, color_len);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGColor e.Color> == empty
 *
 * e.Color = '#RRGGBBAA'
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGColor, "CNFGColor") {
    printf("CNFGColor called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct r05_node* eColor[2];
    char color[9];
    int color_len = r05_read_chars(eColor, color, sizeof(color), callee, arg_end);
    printf("color=%.*s\n", color_len, color);

    if (!r05_empty_hole(eColor[1], arg_end)) {
        printf("extra args\n");
        r05_recognition_impossible();
    }

    int color_value = parse_hex_color(color, color_len);
    CNFGColor(color_value);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGSetPenX s.X> == empty
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGSetPenX, "CNFGSetPenX") {
    printf("CNFGSetPenX called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct signed_number x;
    struct r05_node* sX = parse_signed_number(&x, callee->next)->prev;
    if (!r05_empty_hole(sX, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGPenX = signed_number_int(x);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGSetPenY s.Y> == empty
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGSetPenY, "CNFGSetPenY") {
    printf("CNFGSetPenY called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct signed_number y;
    struct r05_node* sY = parse_signed_number(&y, callee->next)->prev;
    if (!r05_empty_hole(sY, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGPenY = (short)(y.sign * y.value);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGDrawText (e.Text) s.Scale> == empty
 *
 * s.Scale ::= s.NUMBER
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGDrawText, "CNFGDrawText") {
    printf("CNFGDrawText called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct r05_node* tText[2];
    char text[1024];
    int text_len = parse_chars_in_brackets(tText, text, sizeof(text) - 1, callee->next);
    text[text_len] = '\0';

    struct signed_number scale;
    struct r05_node* sScale = parse_signed_number(&scale, tText[1]->next)->prev;
    if (scale.sign == -1) {
        r05_recognition_impossible();
    }
    if (!r05_empty_hole(sScale, arg_end)) {
        r05_recognition_impossible();
    }

    CNFGDrawText(text, (short)scale.value);

    r05_splice_to_freelist(arg_begin, arg_end);
}

/**
 * <CNFGTackRectangle s.X s.Y s.W s.H> == empty
 *
 */
R05_DEFINE_ENTRY_FUNCTION(CNFGTackRectangle, "CNFGTackRectangle") {
    printf("CNFGTackRectangle called from Refal\n");

    struct r05_node* callee = arg_begin->next;

    struct signed_number x;
    struct r05_node* sX = parse_signed_number(&x, callee->next)->prev;

    struct signed_number y;
    struct r05_node* sY = parse_signed_number(&y, sX->next)->prev;

    struct signed_number w;
    struct r05_node* sW = parse_signed_number(&w, sY->next)->prev;
    if (w.sign == -1) {
        r05_recognition_impossible();
    }

    struct signed_number h;
    struct r05_node* sH = parse_signed_number(&h, sW->next)->prev;
    if (h.sign == -1) {
        r05_recognition_impossible();
    }

    if (!r05_empty_hole(sH, arg_end)) {
        r05_recognition_impossible();
    }

    int x1 = signed_number_int(x);
    int y1 = signed_number_int(y);
    int x2 = x1 + signed_number_int(w);
    int y2 = y1 + signed_number_int(h);
    CNFGTackRectangle(x1, y1, x2, y2);

    r05_splice_to_freelist(arg_begin, arg_end);
}
