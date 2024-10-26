#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Write message to stderr using format FORMAT
#define WARN(format, ...) fprintf(stderr, "proj1: " format "\n", __VA_ARGS__)

// Write message to stderr using format FORMAT and exit.
#define DIE(format, ...) WARN(format, __VA_ARGS__), exit(EXIT_FAILURE)

// Double the size of an allocated block PTR with NMEMB members and update
// NMEMB accordingly.  (NMEMB is only the xsize in bytes if PTR is a char *.)
#define DOUBLE(ptr, nmemb) realloc(ptr, (nmemb *= 2) * sizeof(*ptr))

#define INITIAL_CAPACITY 10
#define INITIAL_STATE_AMOUNT 6

// ---------------------- STRING LIBRARY ----------------------
typedef struct
{
    char *txt;
    size_t length;
    size_t capacity;
} String;

String *createString();
String *createString2(char *word);
void appendString(String *s, char c);
void concatString(String *s, String *s2);
void appendWord2String(String *s, char *word);
bool compareString(String *s1, String *s2);
void destroyString(String *s);

// ---------------------- STATE LIBRARY  ----------------------
typedef struct
{
    String *name;
    size_t type;
    String *value;
    struct State *next;
    struct State *prev;

} State;

State *createState(String *name, size_t type, String *value);
void addState(State **head, String *name, size_t type, String *value);
void removeState(State **head, String *name);
void initializeStates(State **head);
State *findState(State **head, String *name);

State *readState(String *content, size_t *pos, State **states);
String *readArguments(String *content, size_t *pos, size_t processMode, String **states);
String *stateMachine(String *content, size_t *pos, State *state, State **states);
String *readIn(String *content, State **states);
void destroyState(State *s);
void freeStates(State **head);

String *fileRead(FILE *file);