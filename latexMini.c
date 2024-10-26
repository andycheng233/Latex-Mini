// Mini Latex Application that takes files and interprets the statements within.

#include "latexMini.h"

// ---------------------- STRING FUNCTIONS ----------------------
// basic create string function
String *createString()
{
    String *newString = (String *)malloc(sizeof(String));
    newString->txt = (char *)malloc(INITIAL_CAPACITY * sizeof(char));
    newString->txt[0] = '\0';
    newString->length = 0;
    newString->capacity = INITIAL_CAPACITY;
    return newString;
}

// advanced create string function that adds the word automatically, utilizes appendWord2String
String *createString2(char *word)
{
    String *newString = createString();
    appendWord2String(newString, word);
    return newString;
}

// adds a character to the string
void appendString(String *s, char c)
{
    if (s->length + 2 > s->capacity)
        s->txt = DOUBLE(s->txt, s->capacity);

    s->txt[s->length] = c;
    s->length++;
    s->txt[s->length] = '\0';
}

// adds a word to the string, utilizes appendString
void appendWord2String(String *s, char *word)
{
    for (size_t i = 0; word[i] != '\0'; i++)
        appendString(s, word[i]);
}

// adds another string to a string, utilizes appendString
void concatString(String *s, String *s2)
{
    for (size_t i = 0; i < s2->length; i++)
        appendString(s, s2->txt[i]);
    destroyString(s2);
}

// compares two string to see if they are identical
bool compareString(String *s1, String *s2)
{
    if (s1->length != s2->length)
        return false;

    for (size_t i = 0; i < s1->length; i++)
        if (s1->txt[i] != s2->txt[i])
            return false;

    return true;
}

// free memory
void destroyString(String *s)
{
    if (s != NULL)
    {
        free(s->txt);
        free(s);
    }
}

// ---------------------- STATE FUNCTIONS ----------------------
// takes a string and needed arguments to create a new state
State *createState(String *name, size_t type, String *value)
{
    State *newState = (State *)malloc(sizeof(State));
    newState->name = name;
    newState->type = type;
    newState->value = value;
    newState->next = NULL;
    newState->prev = NULL;
    return newState;
}

// free memory
void destroyState(State *s)
{
    if (s->name != NULL)
        destroyString(s->name);

    if (s->value != NULL)
        destroyString(s->value);

    if (s != NULL)
        free(s);
}

// free memory in states
void freeStates(State **head)
{
    State *ptr = *head;
    while (ptr != NULL)
    {
        State *next = ptr->next;
        destroyState(ptr);
        ptr = next;
    }

    *head = NULL;
}

// adds a state to the list of states
void addState(State **head, String *name, size_t type, String *value)
{
    State *newState = createState(name, type, value);
    if (*head == NULL)
        *head = newState;

    else
    {
        State *ptr = *head;
        while (ptr->next != NULL)
            ptr = ptr->next;

        ptr->next = newState;
        newState->prev = ptr;
    }
}

// removes a state from the list of states
void removeState(State **head, String *name)
{
    if (*head == NULL)
    {
        destroyString(name);
        return;
    }

    else
    {
        State *ptr = *head;
        while (ptr != NULL)
        {
            if (compareString(ptr->name, name))
            {
                if (ptr == *head)
                {
                    *head = ptr->next;
                }

                State *next = ptr->next;
                State *prev = ptr->prev;

                if (prev != NULL)
                    prev->next = next;

                if (next != NULL)
                    next->prev = prev;

                destroyState(ptr);
                destroyString(name);
                return;
            }

            ptr = ptr->next;
        }

        destroyState(ptr);
        destroyString(name);
    }
}

// initilaizes the predefined states
void initializeStates(State **head)
{
    addState(head, createString2("\\def"), 0, NULL);
    addState(head, createString2("\\undef"), 1, NULL);
    addState(head, createString2("\\if"), 2, NULL);
    addState(head, createString2("\\ifdef"), 3, NULL);
    addState(head, createString2("\\include"), 4, NULL);
    addState(head, createString2("\\expandafter"), 5, NULL);
}

// sees if the state exists and returns a COPY of it or NULL if it does not exist
State *findState(State **head, String *name)
{
    if (*head == NULL)
    {
        destroyString(name);
        return NULL;
    }

    else
    {
        State *ptr = *head;
        while (ptr != NULL)
        {
            if (compareString(ptr->name, name))
            {

                if (ptr->value != NULL)
                {
                    String *temp = createString2(ptr->value->txt);
                    return createState(name, ptr->type, temp);
                }

                else
                    return createState(name, ptr->type, NULL);
            }

            ptr = ptr->next;
        }

        return NULL;
    }
}

// used when '/' is detected, returns the state or special character
State *readState(String *content, size_t *pos, State **states)
{
    char ch = content->txt[++(*pos)];
    String *store = createString2("\\");

    // if the next character is a special character - \\, \#, \%, \{, \}
    if (ch == '\\' || ch == '#' || ch == '%' || ch == '{' || ch == '}')
    {
        String *temp = createString();
        appendString(temp, ch);
        destroyString(store);
        return createState(temp, -1, NULL);
    }

    // if the character is alphanumeric (if its a macro)
    else if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
    {
        appendString(store, ch);
    }

    // if its anything else
    else
    {
        appendString(store, ch);
        return createState(store, -2, NULL);
    }

    // getting the rest of the macro --> can find end in two ways: when I see { or } or \0
    *pos += 1;
    while (content->txt[*pos] != '{' && content->txt[*pos] != '}' && content->txt[*pos] != '\0')
    {
        if (!isalnum(content->txt[*pos]))
            DIE("%s", "State is Not Alphanumeric");
        appendString(store, content->txt[*pos]);
        (*pos)++;
    }

    // seeing if the state exists and getting more information
    State *state = findState(states, store);
    if (state == NULL)
        DIE("%s", "state has not been defined");
    return state;
}

// read in string content and process
String *readIn(String *content, State **states)
{
    char ch;
    String *state;
    String *output = createString();

    size_t pos = 0;

    while (pos < content->length)
    {
        char ch = content->txt[pos];

        // special case
        if (ch == '\\')
        {
            State *save = readState(content, &pos, states);

            // escape character
            if (save->type == -1)
            {
                appendString(output, save->name->txt[0]);
                pos++;
            }

            // leave it alone
            else if (save->type == -2)
            {
                appendWord2String(output, save->name->txt);
                pos++;
            }

            // state
            else
            {
                String *result = stateMachine(content, &pos, save, states);

                if (result != NULL)
                {
                    appendWord2String(output, result->txt);
                    if (content->txt[pos] == '{')
                        pos++;
                }

                destroyString(result);
            }

            destroyState(save);
        }

        else
        {
            appendString(output, ch);
            pos++;
        }
    }

    return output;
}

// reading in arguments for states
String *readArgument(String *content, size_t *pos, size_t processMode, String **states)
{
    char ch = content->txt[*pos];
    String *argument = createString();
    bool recall = false;

    if (ch == '}')
    {
        ch = content->txt[++(*pos)];
        recall = true;
    }

    // every argument should be contained in {}
    else if (ch != '{')
    {
        DIE("%s", "Line 288: Improper Formatting: Expected { for Argument!");
    }

    // return the text as is and ensure brace balancing
    if (processMode == 0)
    {
        size_t braceCount = 1;

        while (braceCount != 0)
        {
            ch = content->txt[++(*pos)];

            if (ch == '\0')
                DIE("%s", "Bracket Balance Error!");

            else if (ch == '}')
                braceCount--;

            else if (ch == '{')
                braceCount++;

            if (braceCount != 0)
                appendString(argument, ch);
        }

        (*pos)++;
        return argument;
    }

    // ensure alphanumeric and brace balancing
    else if (processMode == 1)
    {
        size_t braceCount = 1;

        while (braceCount != 0)
        {
            ch = content->txt[++(*pos)];

            if (!isalnum(ch) && ch != '}')
                DIE("%s", "Not alphanumeric!");

            else if (ch == '}')
                braceCount--;
            else
                appendString(argument, ch);
        }

        (*pos)++;

        if (argument->length == 0)
            DIE("%s", "Not alphanumeric!");

        return argument;
    }

    // read and process argument
    else
    {
        size_t braceCount = 1;

        while (braceCount != 0)
        {
            ch = content->txt[++(*pos)];

            if (ch == '\0')
                DIE("%s", "Bracket Balance Error!");

            else if (ch == '\\')
            {
                ch = content->txt[++(*pos)];
                if (ch != '\0' && braceCount != 0 && (ch == '#' || ch == '\\' || ch == '%' || ch == '{' || ch == '}'))
                {
                    appendString(argument, ch);
                }

                else if (!isalnum(ch))
                {
                    appendString(argument, '\\');
                    appendString(argument, ch);
                }

                else
                {
                    ch = content->txt[--(*pos)];
                    State *save = readState(content, pos, states);
                    String *result = stateMachine(content, pos, save, states);

                    if (result != NULL)
                    {
                        appendWord2String(argument, result->txt);
                        (*pos) -= 2;
                        if (content->txt[(*pos)] == '{')
                            (*pos)++;
                    }

                    destroyString(result);
                    destroyState(save);
                }
            }

            else if (ch == '}')
            {
                braceCount--;
                if (braceCount != 0)
                    appendString(argument, ch);
            }

            else if (ch == '{')
            {
                braceCount++;
                if (braceCount != 0)
                    appendString(argument, ch);
            }

            else if (braceCount != 0)
                appendString(argument, ch);
        }

        (*pos)++;
        return argument;
    }
}

String *stateMachine(String *content, size_t *pos, State *state, State **states)
{
    // \def{name}{value}
    if (state->type == 0)
    {
        String *p1 = createString();
        appendString(p1, '\\');
        concatString(p1, readArgument(content, pos, 1, states));

        String *p1Copy = createString2(p1->txt);

        State *s = findState(states, p1);

        if (s != NULL)
        {
            destroyState(s);
            DIE("%s", "already defined!");
        }

        destroyString(p1);

        String *p2 = readArgument(content, pos, 0, states);
        addState(states, p1Copy, 6, p2);

        return NULL;
    }

    // \undef{name}
    else if (state->type == 1)
    {
        String *p1 = createString();
        appendString(p1, '\\');
        concatString(p1, readArgument(content, pos, 1, states));

        State *s = findState(states, p1);
        if (s == NULL)
            DIE("%s", "can't undefine when it has not been defined!");
        else if (s->type != 6)
        {

            DIE("%s", "can't undef that!");
        }

        String *p1Copy = createString2(p1->txt);
        removeState(states, p1Copy);
        destroyState(s);

        return NULL;
    }

    // \if{cond}{then}{else}
    else if (state->type == 2)
    {
        String *p1 = readArgument(content, pos, 0, states);
        String *p2 = readArgument(content, pos, 0, states);
        String *p3 = readArgument(content, pos, 0, states);

        String *output;

        if (p1->length > 0)
        {
            output = p2;
            destroyString(p3);
        }

        else
        {
            output = p3;
            destroyString(p2);
        }

        while (content->txt[*pos] != '\n' && content->txt[*pos] != '\0')
        {
            appendString(output, content->txt[*pos]);
            (*pos)++;
        }

        String *final = readIn(output, states);
        destroyString(output);

        destroyString(p1);

        return final;
    }

    // \ifdef{name}{then}{else}
    else if (state->type == 3)
    {
        String *p1 = createString();
        appendString(p1, '\\');
        concatString(p1, readArgument(content, pos, 1, states));
        String *p2 = readArgument(content, pos, 0, states);
        String *p3 = readArgument(content, pos, 0, states);

        String *output;

        State *s = findState(states, p1);

        if (s != NULL && s->type == 6)
        {
            output = p2;
            destroyString(p3);
        }

        else
        {

            output = p3;
            destroyString(p2);
        }

        while (content->txt[*pos] != '\n' && content->txt[*pos] != '\0')
        {
            appendString(output, content->txt[*pos]);
            (*pos)++;
        }

        String *final = readIn(output, states);
        destroyString(output);

        if (s != NULL)
            destroyState(s);
        else
            destroyString(p1);

        return final;
    }

    // \include{path}
    else if (state->type == 4)
    {
        String *fileName = readArgument(content, pos, 0, states);
        FILE *file = fopen(fileName->txt, "r");

        if (file == NULL)
            DIE("%s", "One of the Files Not Working Properly!");

        String *fileContent = fileRead(file);

        while (content->txt[*pos] != '\n' && content->txt[*pos] != '\0')
        {
            appendString(fileContent, content->txt[*pos]);
            (*pos)++;
        }

        String *output = readIn(fileContent, states);

        destroyString(fileName);
        destroyString(fileContent);
        fclose(file);
        return output;
    }

    // \expanderafter{before}{after}
    else if (state->type == 5)
    {
        String *p1 = readArgument(content, pos, 0, states);
        String *p2 = readArgument(content, pos, 0, states);

        String *p2Processed = readIn(p2, states);
        destroyString(p2);

        concatString(p1, p2Processed);

        while (content->txt[*pos] != '\n' && content->txt[*pos] != '\0')
        {
            appendString(p1, content->txt[*pos]);
            (*pos)++;
        }

        String *final = readIn(p1, states);

        destroyString(p1);

        return final;
    }

    // user-defined macros
    else
    {
        String *p1 = createString();
        concatString(p1, readArgument(content, pos, 2, states));

        String *store = createString();
        for (size_t i = 0; i < state->value->length; i++)
        {
            if (state->value->txt[i] == '\\')
            {
                i++;
                if (i < state->value->length && (state->value->txt[i] == '#' || state->value->txt[i] == '\\' || state->value->txt[i] == '%' || state->value->txt[i] == '{' || state->value->txt[i] == '}'))
                {
                    appendString(store, '\\');
                    appendString(store, state->value->txt[i]);
                }
                else
                {
                    i--;
                    appendString(store, '\\');
                }
            }

            else if (state->value->txt[i] == '#')
                appendWord2String(store, p1->txt);

            else
                appendString(store, state->value->txt[i]);
        }

        while (content->txt[*pos] != '\n' && content->txt[*pos] != '\0')
        {
            appendString(store, content->txt[*pos]);
            (*pos)++;
        }

        String *final = readIn(store, states);
        destroyString(store);
        destroyString(p1);
        return final;
    }
}

// reads in the text of a file into a string and gets rid of comments
String *fileRead(FILE *file)
{
    String *content = createString();
    char ch;

    size_t currentState = 0;

    while ((ch = fgetc(file)) != EOF)
    {
        switch (currentState)
        {
        case 0:
            if (ch == '%')
                currentState = 1;
            else if (ch == '\\')
                currentState = 3;
            else
                appendString(content, ch);
            break;

        case 1:
            if (ch == '\n')
                currentState = 2;
            break;

        case 2:
            if (!isblank(ch))
            {
                if (ch == '%')
                    currentState = 1;
                else if (ch == '\\')
                    currentState = 3;
                else
                {
                    appendString(content, ch);
                    currentState = 0;
                }
            }
            break;
        case 3:
            appendString(content, '\\');
            appendString(content, ch);
            currentState = 0;
            break;
        default:
            DIE("%s", "Issue with removing comments");
            break;
        }
    }

    return content;
}

void main(int argc, char *argv[])
{
    State *states = NULL;
    initializeStates(&states);

    // stdin mode
    if (argc == 1)
    {
        String *fileContent = fileRead(stdin);
        String *output = readIn(fileContent, &states);

        for (size_t i = 0; i < output->length; i++)
            fprintf(stdout, "%c", output->txt[i]);

        destroyString(fileContent);
        destroyString(output);
    }

    // file mode
    else
    {
        size_t fileNum = argc - 1;

        for (size_t i = 0; i < fileNum; i++)
        {
            FILE *file = fopen(argv[i + 1], "r");

            if (file == NULL)
                DIE("%s", "One of the Files Not Working Properly!");

            String *fileContent = fileRead(file);
            String *output = readIn(fileContent, &states);

            for (size_t i = 0; i < output->length; i++)
                fprintf(stdout, "%c", output->txt[i]);

            destroyString(fileContent);
            destroyString(output);
            fclose(file);
        }
    }

    freeStates(&states);
}