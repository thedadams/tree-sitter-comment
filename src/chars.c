#include "chars.h"

static bool is_special_tag(int32_t c) {
    const int32_t special_chars[] = {
        CHAR_EXCLAMATION_STAR,
        CHAR_EXCLAMATION_MARK,
        CHAR_QUESTION_MARK,
        CHAR_HASH,
    };
    const int length = sizeof(special_chars) / sizeof(int32_t);
    for (int i = 0; i < length; i++) {
        if (c == special_chars[i]) {
            return true;
        }
    }
    return false;
}

static bool is_upper(int32_t c) {
    const int32_t upper = 65;
    const int32_t lower = 90;
    return c >= upper && c <= lower;
}

static bool is_lower(int32_t c) {
    const int32_t upper = 97;
    const int32_t lower = 122;
    return c >= upper && c <= lower;
}

static bool is_digit(int32_t c) {
    const int32_t upper = 48;
    const int32_t lower = 57;
    return c >= upper && c <= lower;
}

static bool is_alpha(int32_t c) { return is_upper(c) || is_lower(c); }

static bool is_newline(int32_t c) {
    const int32_t newline_chars[] = {
        CHAR_NEWLINE,
        CHAR_CARRIAGE_RETURN,
    };
    const int length = sizeof(newline_chars) / sizeof(int32_t);
    for (int i = 0; i < length; i++) {
        if (c == newline_chars[i]) {
            return true;
        }
    }
    return false;
}

static bool is_space_or_newline(int32_t c) { return is_space(c) || is_newline(c); }

static bool is_space(int32_t c) {
    const int32_t space_chars[] = {
        CHAR_SPACE,
        CHAR_FORM_FEED,
        CHAR_TAB,
        CHAR_VERTICAL_TAB,
    };
    const int length = sizeof(space_chars) / sizeof(int32_t);
    for (int i = 0; i < length; i++) {
        if (c == space_chars[i]) {
            return true;
        }
    }
    return false;
}

/// Check if the character is allowed inside the name.
static bool is_internal_char(int32_t c) {
    const int32_t valid_chars[] = {
        '-',
        '_',
    };
    const int length = sizeof(valid_chars) / sizeof(int32_t);
    for (int i = 0; i < length; i++) {
        if (c == valid_chars[i]) {
            return true;
        }
    }
    return false;
}

/// Check if the character is to start a new comment line.
static bool is_possible_start_of_tag(int32_t c) {
    const int32_t valid_chars[] = {
        '/', '#', '*', ';', '+', '-', '!', '|', '<', '[', '%',
    };
    const int length = sizeof(valid_chars) / sizeof(int32_t);
    for (int i = 0; i < length; i++) {
        if (c == valid_chars[i]) {
            return true;
        }
    }
    return false;
}
