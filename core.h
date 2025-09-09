/*
MIT License

Copyright (c) 2025 Robert Burnett

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _CORE_H_
#define _CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#ifdef CORE_IMPLEMENTATION
#   define _CORE_FN_BODY(...) __VA_ARGS__
#else
#   define _CORE_FN_BODY(...) ;
#endif /*CORE_IMPLEMENTATION*/

//// ATTRIBUTES
#if defined(__clang__) || defined(__GNUC__)
#   define CORE_NORETURN __attribute__((noreturn))
#   define CORE_NODISCARD __attribute__((warn_unused_result))
#else
#   define CORE_NORETURN
#   define CORE_NODISCARD
#endif /*__clang__ || __GNUC__*/


//// ANSI
#define CORE_ANSI_RED     "\x1b[31m"
#define CORE_ANSI_GREEN   "\x1b[32m"
#define CORE_ANSI_YELLOW  "\x1b[33m"
#define CORE_ANSI_BLUE    "\x1b[34m"
#define CORE_ANSI_MAGENTA "\x1b[35m"
#define CORE_ANSI_CYAN    "\x1b[36m"
#define CORE_ANSI_RESET   "\x1b[0m"


//// MACROS
#define CORE_LOG(...) do { \
    fprintf(stderr, "%s:%d:0:   ", __FILE__, __LINE__);  \
    fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); \
    fflush(stderr); \
} while (0)

void core_exit(int exitcode);

#define CORE_UNREACHABLE do { CORE_LOG("unreachable code block reached!"); core_exit(1); } while (0)
#define CORE_TODO(...) do { CORE_LOG(CORE_ANSI_RESET "TODO:  " __VA_ARGS__); core_exit(1); } while (0)
#define CORE_FATAL_ERROR(...) do {CORE_LOG("ERROR"); CORE_LOG(__VA_ARGS__); core_exit(1); } while (0)


//// EXIT
#define CORE_ON_EXIT_MAX_FUNCTIONS 64
void (*core_on_exit_fns[CORE_ON_EXIT_MAX_FUNCTIONS])(void * ctx) = {0};
void * core_on_exit_ctx[CORE_ON_EXIT_MAX_FUNCTIONS] = {0}; 
int core_on_exit_fn_count = 0;

void core_on_exit(void (*fn)(void *ctx), void * ctx) _CORE_FN_BODY({
    if(core_on_exit_fn_count + 1 > CORE_ON_EXIT_MAX_FUNCTIONS) CORE_UNREACHABLE;
    core_on_exit_fns[core_on_exit_fn_count] = fn; 
    core_on_exit_ctx[core_on_exit_fn_count] = ctx; 
    ++core_on_exit_fn_count;
})

CORE_NORETURN
void core_exit(int exitcode) _CORE_FN_BODY({
    for(int i = 0; i < core_on_exit_fn_count; ++i) {
        core_on_exit_fns[i](core_on_exit_ctx[i]);
    }
    exit(exitcode);
})



//// PROFILER

#include <sys/time.h>
unsigned long _core_profiler_timestamp(void) _CORE_FN_BODY({
    struct timeval currentTime = {0};
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
})

static FILE * _core_profiler_output_file = NULL;
void core_profiler_init(const char * output_file_path) _CORE_FN_BODY({
    _core_profiler_output_file = fopen(output_file_path, "w");
    fprintf(_core_profiler_output_file, "[\n");
})

void core_profiler_deinit(void) _CORE_FN_BODY({
    fprintf(_core_profiler_output_file, "\n]\n");
    fclose(_core_profiler_output_file);
})

void _core_profiler_log(const char * event_name, char begin_or_end, const char * srcfile, const int srcline) _CORE_FN_BODY({
    static bool prepend_comma = false;
    if(prepend_comma) {
        fprintf(_core_profiler_output_file, ",\n");
    } 
    prepend_comma = true;
    fprintf(_core_profiler_output_file,
            "{ \"name\": \"%s\", \"ph\": \"%c\", \"ts\": %lu, \"tid\": 1, \"pid\": 1, \"args\": { \"file\": \"%s\", \"line\": %d } }",
            event_name, begin_or_end, _core_profiler_timestamp(), srcfile, srcline);
})


#define core_profiler_start(event) _core_profiler_log(event, 'B', __FILE__, __LINE__)
#define core_profiler_stop(event) _core_profiler_log(event, 'E', __FILE__, __LINE__)


//// ARENA
typedef struct core_Allocation {
    struct core_Allocation * next;

    void * mem;
    size_t len;
    bool active;
} core_Allocation;

typedef struct {
    core_Allocation * head;
} core_Arena;

core_Allocation * core_arena_allocation_new(size_t bytes) _CORE_FN_BODY({
    core_Allocation * ptr = malloc(sizeof(core_Allocation));
    ptr->mem = malloc(bytes);
    ptr->len = bytes;
    ptr->active = true;
    ptr->next = NULL;
    return ptr;
})

CORE_NODISCARD
void * core_arena_alloc(core_Arena * a, const size_t bytes) _CORE_FN_BODY({
    if(a->head == NULL) {
        core_Allocation * head =  core_arena_allocation_new(bytes);       
        a->head = head;
        return a->head->next;
    }
    core_Allocation * ptr = a->head;
    for(;ptr->next != NULL; ptr = ptr->next) {
        if(!ptr->active && ptr->len >= bytes) {
            ptr->active = true;
            return ptr->mem;
        }
    }
    assert(ptr != NULL);
    assert(ptr->next != NULL);
    core_Allocation * next =  core_arena_allocation_new(bytes);       
    ptr->next = next;
    return next->mem;
})

void core_arena_free(core_Arena * a) _CORE_FN_BODY({
    if(a->head == NULL) return;
    for(core_Allocation * ptr = a->head;;) {
        core_Allocation * next = ptr->next;
        free(ptr->mem);
        free(ptr);
        if(next == NULL) return;
        ptr = next;
    }
})


//// VEC
#define core_Vec(Type) struct {Type * items; int len; int cap; }
#define core_vec_append(vec, arena, item) do { \
    if((vec)->cap <= 0) { \
        (vec)->cap = 8; \
        (vec)->len = 0; \
        (vec)->items = core_arena_alloc(arena, sizeof(item) * (vec)->cap); \
    } else if((vec)->len + 1 >= (vec)->cap) { \
        (vec)->cap = (vec)->cap * 2 + 1; \
        void * newmem = core_arena_alloc(arena, sizeof(item) * (vec)->cap); \
        memcpy(newmem, (vec)->items, sizeof(item) * (vec)->len); \
    } \
    (vec)->items[(vec)->len++] = item; \
} while (0)


//// CTYPE
bool core_isidentifier(char ch) _CORE_FN_BODY({
    return isalpha(ch) || isdigit(ch) || ch == '_';
})

//// SYMBOL
#ifndef CORE_SYMBOL_MAX_LEN
#   define CORE_SYMBOL_MAX_LEN 32
#endif /*CORE_SYMBOL_MAX_LEN*/
#ifndef CORE_MAX_SYMBOLS
#   define CORE_MAX_SYMBOLS 1024
#endif /*CORE_MAX_SYMBOLS*/

typedef int core_Symbol;
typedef struct {
    char symbols[CORE_MAX_SYMBOLS][CORE_SYMBOL_MAX_LEN];
    int count;
} core_Symbols;

core_Symbol core_symbol_intern(core_Symbols * state, const char * str) _CORE_FN_BODY({
    for(int i = 0; i < state->count; ++i) {
        if(strcmp(state->symbols[i], str) == 0) return i;
    }
    strncpy(state->symbols[state->count], str, CORE_SYMBOL_MAX_LEN - 1);
    int result = state->count;
    ++state->count;
    return result;
})

const char * core_symbol_get(core_Symbols * state, core_Symbol sym) _CORE_FN_BODY({
    assert(sym < state->count);
    return state->symbols[sym];
})

//// PEEK
char core_peek(FILE * fp) _CORE_FN_BODY({
    char ch = fgetc(fp);
    ungetc(ch, fp);
    return ch;
})

void core_skip_whitespace(FILE * fp) _CORE_FN_BODY({
    while(isspace(core_peek(fp))) (void)fgetc(fp);
})


//// DEFER
#define core_defer(label) \
    while(0) \
        while(1) \
            if (1) { \
                goto label##_done_; \
            } else label:

#define core_deferred(label) do { goto label; label##_done_:; } while (0)



//// CONCAT
#define CORE_CONCAT9(x, y) x##y
#define CORE_CONCAT8(x, y) CORE_CONCAT9(x, y)
#define CORE_CONCAT7(x, y) CORE_CONCAT8(x, y)
#define CORE_CONCAT6(x, y) CORE_CONCAT7(x, y)
#define CORE_CONCAT5(x, y) CORE_CONCAT6(x, y)
#define CORE_CONCAT4(x, y) CORE_CONCAT5(x, y)
#define CORE_CONCAT3(x, y) CORE_CONCAT4(x, y)
#define CORE_CONCAT2(x, y) CORE_CONCAT3(x, y)
#define CORE_CONCAT1(x, y) CORE_CONCAT2(x, y)
#define CORE_CONCAT(x, y)  CORE_CONCAT1(x, y)

#endif /*_CORE_H_*/
