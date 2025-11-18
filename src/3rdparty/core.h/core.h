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
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <time.h>


/****  C STANDARD ****/
#ifdef __STDC_VERSION
#   if __STDC_VERSION >= 202311L
#      define CORE_C23
#   elif __STDC_VERSION__ >=  201710L
#      define CORE_C17
#   elif __STDC_VERSION__ >= 201112L
#      define CORE_C11
#   elif __STDC_VERSION__ >= 199901L
#      define CORE_C99
#   endif 
#else
#   define CORE_C89
#endif /*__STDC_VERSION__*/


/**** OPERATING SYSTEM ****/
#if defined(__unix__)
#    define CORE_UNIX
#elif defined(_WIN32) || defined(WIN32)
#    define CORE_WINDOWS
#endif


/**** COMPILER ****/
#if defined(__clang__)
#   define CORE_CLANG
#elif defined(__GNUC__)
#   define CORE_GCC
#endif


/**** BOOL ****/
#if !defined(CORE_C89)
    #include <stdbool.h>
    typedef bool core_Bool;
    #define CORE_TRUE true
    #define CORE_FALSE false
#else
    typedef unsigned char core_Bool;
    #define CORE_TRUE 1
    #define CORE_FALSE 0
#endif /*C89*/

/**** ATTRIBUTES ****/
#if defined(CORE_CLANG) || defined(CORE_GCC)
#   define CORE_NORETURN __attribute__((noreturn))
#   define CORE_NODISCARD __attribute__((warn_unused_result))
#else
#   define CORE_NORETURN
#   define CORE_NODISCARD
#endif /* CLANG GCC */


/**** ANSI ****/
#define CORE_ANSI_RED     "\x1b[31m"
#define CORE_ANSI_GREEN   "\x1b[32m"
#define CORE_ANSI_YELLOW  "\x1b[33m"
#define CORE_ANSI_BLUE    "\x1b[34m"
#define CORE_ANSI_MAGENTA "\x1b[35m"
#define CORE_ANSI_CYAN    "\x1b[36m"
#define CORE_ANSI_RESET   "\x1b[0m"


/**** MACROS ****/
#define CORE_LOG(msg) do { \
    fprintf(stderr, "%10s:%4d:0:   ", __FILE__, __LINE__);  \
    fprintf(stderr, "%s\n", msg); \
    fflush(stderr); \
} while (0)

#define CORE_UNREACHABLE do { CORE_LOG("unreachable code block reached!"); core_exit(1); } while (0)
#define CORE_TODO(msg) do { CORE_LOG(CORE_ANSI_RESET "TODO:  "); CORE_LOG(msg); core_exit(1); } while (0)
#define CORE_FATAL_ERROR(msg) do {CORE_LOG("ERROR"); CORE_LOG(msg); core_exit(1); } while (0)
#define CORE_ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))


/**** EXIT ****/
#ifdef CORE_IMPLEMENTATION
#define CORE_ON_EXIT_MAX_FUNCTIONS 64
void (*core_on_exit_fns[CORE_ON_EXIT_MAX_FUNCTIONS])(void * ctx) = {0};
void * core_on_exit_ctx[CORE_ON_EXIT_MAX_FUNCTIONS] = {0};
int core_on_exit_fn_count = 0;
#endif /*CORE_IMPLEMENTATION*/


CORE_NORETURN void core_exit(int exitcode)
#ifdef CORE_IMPLEMENTATION
{
    int i = 0;
    for(i = 0; i < core_on_exit_fn_count; ++i) {
        core_on_exit_fns[i](core_on_exit_ctx[i]);
    }
    exit(exitcode);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


void core_on_exit(void (*fn)(void *ctx), void * ctx)
#ifdef CORE_IMPLEMENTATION
{
    if(core_on_exit_fn_count + 1 > CORE_ON_EXIT_MAX_FUNCTIONS) CORE_UNREACHABLE;
    core_on_exit_fns[core_on_exit_fn_count] = fn;
    core_on_exit_ctx[core_on_exit_fn_count] = ctx;
    ++core_on_exit_fn_count;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


/****  PROFILER ****/

/*profiler requires unix system*/
#ifdef __unix__

#include <sys/time.h>
long _core_profiler_timestamp(void)
#ifdef CORE_IMPLEMENTATION
{
    struct timeval currentTime = {0};
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

#ifdef CORE_IMPLEMENTATION
static FILE * _core_profiler_output_file = NULL;
#endif /*CORE_IMPLEMENTATION*/

void core_profiler_init(const char * output_file_path)
#ifdef CORE_IMPLEMENTATION
{
    _core_profiler_output_file = fopen(output_file_path, "w");
    fprintf(_core_profiler_output_file, "[\n");
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_profiler_deinit(void)
#ifdef CORE_IMPLEMENTATION
{
    fprintf(_core_profiler_output_file, "\n]\n");
    fclose(_core_profiler_output_file);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void _core_profiler_log(const char * event_name, char begin_or_end, const char * srcfile, const int srcline)
#ifdef CORE_IMPLEMENTATION
{
    static core_Bool prepend_comma = CORE_FALSE;
    if(prepend_comma) {
        fprintf(_core_profiler_output_file, ",\n");
    }
    prepend_comma = CORE_TRUE;
    fprintf(_core_profiler_output_file,
            "{ \"name\": \"%s\", \"ph\": \"%c\", \"ts\": %ld, \"tid\": 1, \"pid\": 1, \"args\": { \"file\": \"%s\", \"line\": %d } }",
            event_name, begin_or_end, _core_profiler_timestamp(), srcfile, srcline);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

#define core_profiler_start(event) _core_profiler_log(event, 'B', __FILE__, __LINE__)
#define core_profiler_stop(event) _core_profiler_log(event, 'E', __FILE__, __LINE__)
#endif /*__unix__*/


/**** ARENA ****/
typedef struct core_Allocation {
    struct core_Allocation * next;
    void * mem;
    size_t len;
    core_Bool active;
} core_Allocation;

typedef struct {
    core_Allocation * head;
} core_Arena;

core_Allocation * core_arena_allocation_new(size_t bytes)
#ifdef CORE_IMPLEMENTATION
{
    core_Allocation * ptr = malloc(sizeof(core_Allocation));
    assert(ptr);
    ptr->mem = malloc(bytes);
    assert(ptr->mem);
    ptr->len = bytes;
    ptr->active = CORE_TRUE;
    ptr->next = NULL;
    return ptr;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

CORE_NODISCARD
void * core_arena_alloc(core_Arena * a, const size_t bytes)
#ifdef CORE_IMPLEMENTATION
{
    core_Allocation * ptr = NULL;
    if(a->head == NULL) {
        core_Allocation * head = core_arena_allocation_new(bytes);
        a->head = head;
        return a->head->mem;
    }
   ptr = a->head;
    for(;ptr->next != NULL; ptr = ptr->next) {
        if(!ptr->active && ptr->len >= bytes) {
            ptr->active = CORE_TRUE;
            return ptr->mem;
        }
    }
    assert(ptr != NULL);
    assert(ptr->next == NULL);
    {
        core_Allocation * next =  core_arena_allocation_new(bytes);
        ptr->next = next;
        return next->mem;
    }
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

CORE_NODISCARD
void * core_arena_realloc(core_Arena * a, void * ptr, const size_t bytes)
#ifdef CORE_IMPLEMENTATION
{
    core_Allocation * node = NULL;
    core_Allocation * new = NULL;
    assert(ptr != NULL);
    for(node = a->head; node != NULL && node->mem != ptr; node = node->next);
    assert(node != NULL);
    assert(node->mem == ptr);
    assert(bytes >= node->len);
    node->active = CORE_FALSE;
    new = core_arena_allocation_new(bytes);
    assert(new);
    assert(new->len >= node->len);
    memcpy(new->mem, node->mem, node->len);
    new->next = a->head;
    a->head = new;
    return new->mem;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/
    
    

void core_arena_free(core_Arena * a)
#ifdef CORE_IMPLEMENTATION
{
    core_Allocation * ptr = NULL;
    if(a->head == NULL) return;
    for(ptr = a->head;;) {
        core_Allocation * next = ptr->next;
        free(ptr->mem);
        free(ptr);
        if(next == NULL) return;
        ptr = next;
    }
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


/**** VEC ****/
#define core_Vec(Type) struct {Type * items; unsigned int len; unsigned int cap; }

#define core_vec_append(vec, arena, item) do { \
    if((vec)->cap <= 0) { \
        (vec)->cap = 8; \
        (vec)->len = 0; \
        (vec)->items = core_arena_alloc(arena, sizeof(item) * (vec)->cap); \
    } else if((vec)->len + 1 >= (vec)->cap) { \
        (vec)->cap = (vec)->cap * 2 + 1; \
        { \
            void * newmem = core_arena_alloc(arena, sizeof(item) * (vec)->cap); \
            memcpy(newmem, (vec)->items, sizeof(item) * (vec)->len);    \
        } \
    } \
    (vec)->items[(vec)->len++] = item; \
} while (0)

#define core_vec_free(vec) do { free(vec->items); vec->cap = 0; vec->len = 0;} while (0)

/**** CTYPE ****/
core_Bool core_isidentifier(char ch)
#ifdef CORE_IMPLEMENTATION
{
    return isalpha(ch) || isdigit(ch) || ch == '_';
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

/**** SYMBOL ****/
#ifndef CORE_SYMBOL_MAX_LEN
#   define CORE_SYMBOL_MAX_LEN 128
#endif /*CORE_SYMBOL_MAX_LEN*/
#ifndef CORE_MAX_SYMBOLS
#   define CORE_MAX_SYMBOLS 2048
#endif /*CORE_MAX_SYMBOLS*/

typedef int core_Symbol;
typedef struct {
    char symbols[CORE_MAX_SYMBOLS][CORE_SYMBOL_MAX_LEN];
    int count;
} core_Symbols;

core_Symbol core_symbol_intern(core_Symbols * state, const char * str)
#ifdef CORE_IMPLEMENTATION
{
    int i = 0;
    int result = -1;
    for(i = 0; i < state->count; ++i) {
        if(strcmp(state->symbols[i], str) == 0) return i;
    }
    strncpy(state->symbols[state->count], str, CORE_SYMBOL_MAX_LEN - 1);
    result = state->count;
    ++state->count;
    return result;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

const char * core_symbol_get(core_Symbols * state, core_Symbol sym)
#ifdef CORE_IMPLEMENTATION
{
    assert(sym < state->count);
    return state->symbols[sym];
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

/**** PEEK ****/
char core_peek(FILE * fp)
#ifdef CORE_IMPLEMENTATION
{
    char ch = (char)fgetc(fp);
    ungetc(ch, fp);
    return ch;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_skip_whitespace(FILE * fp)
#ifdef CORE_IMPLEMENTATION
{
    while(isspace(core_peek(fp))) (void)fgetc(fp);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


/**** FILE ****/

core_Bool core_file_read_all(FILE * fp, char * dst, const unsigned long dst_cap)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long count = fread(dst, 1, dst_cap - 1, fp);
    dst[dst_cap - 1] = 0;
    if(count >= dst_cap - 1) return CORE_FALSE;
    return CORE_TRUE;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

char * core_file_read_all_arena(core_Arena * arena, FILE * fp)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long n = 128;
    unsigned long i = 0;
    char * buf = core_arena_alloc(arena, n);
    if (buf == NULL) return NULL;
    while(!feof(fp)) {
        unsigned long available = n - i - 1;
        const unsigned long elem_size = 1;
        unsigned long count = fread(&buf[i], elem_size, available, fp);
        i += count;
        if (i >= n - 1) {
            n *= 2;
            buf = core_arena_realloc(arena, buf, n);
            if (buf == NULL) return NULL;
        }
    }
    buf[i] = 0;
    return buf;
}
#else
;
#endif /* CORE_IMPLEMENTATION */


/*TODO
  
  CORE_ERR
  CORE_OK

  CORE_LOG_ERROR
*/


/**** DEFER ****/
#define CORE_DEFER(label) \
    while(0) \
        while(1) \
            if (1) { \
                goto label##_done_; \
            } else label:

#define CORE_DEFERRED(label) do { goto label; label##_done_:; } while (0)


/**** CONCAT ****/
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


/**** STATIC ASSERT ****/
#if defined(CORE_C23)
#   define STATIC_ASSERT(condition, message) static_assert(condition, message)
#elif defined(CORE_C11)
#   define STATIC_ASSERT(condition, message) _Static_assert(condition, message)
#else
#   define STATIC_ASSERT(condition, message) const int static_assertion_##__COUNTER__[ condition ? 1 : -1 ];
#endif /*__STDC_VERSION__*/

/**** ALIGNOF ****/
#if defined(__GNUC__) || defined(__clang__)
#    define CORE_ALIGNOF(type) __alignof__(type)
#elif defined(_MSC_VER)
#    define CORE_ALIGNOF(type) __alignof(type)
#else
#    define CORE_ALIGNOF(type) ((size_t)&((struct { char c; type member; } *)0)->member)
#endif /*defined(__GNUC__) || defined(__clang__)*/

/**** LIKELY ****/
#if defined(__GNUC__) || defined(__clang__)
#    define CORE_LIKELY_TRUE(expr)  __builtin_expect(expr, 1)
#    define CORE_LIKELY_FALSE(expr) __builtin_expect(expr, 0)
#else
#    define CORE_LIKELY_TRUE(expr)
#    define CORE_LIKELY_FALSE(expr)
#endif /*defined(__GNUC__) || defined(__clang__)*/

/**** MINMAX ****/
#define CORE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CORE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CORE_MIN3(a, b, c) CORE_MIN(CORE_MIN(a, b), c)
#define CORE_MAX3(a, b, c) CORE_MAX(CORE_MAX(a, b), c)

/**** STRING ****/
void core_strfmt(char * dst, size_t dst_len, size_t * dst_fill_pointer, const char * src, const size_t src_len)
#ifdef CORE_IMPLEMENTATION
{
    size_t i = 0;
    assert(dst_fill_pointer);
    assert(dst);
    assert(src);
    assert(strlen(src) == src_len && "inaccurate length");
    
    if(*dst_fill_pointer + src_len + 1 >= dst_len) CORE_FATAL_ERROR("Buffer overflow");
    for(i = 0; i < src_len; ++i) {
        dst[*dst_fill_pointer] = src[i];
        ++*dst_fill_pointer;
        assert(*dst_fill_pointer < dst_len);
    }
    dst[*dst_fill_pointer] = 0;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

#define core_streql(lhs, rhs) core_strneql(lhs, rhs, -1)
core_Bool core_strneql(const char * lhs, const char * rhs, unsigned long n)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long i = 0;
    for(i = 0; i < n; ++i) {
        assert(lhs[i] != 0 && "Unexpected NULL terminator");
        assert(rhs[i] != 0 && "Unexpected NULL terminator");
        if(n > 0 && i > n) return CORE_TRUE;
        if(lhs[i] != rhs[i]) return CORE_FALSE;
    }
    return CORE_TRUE;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

char * core_strdup_via_arena(core_Arena * arena, const char * str, size_t len)
#ifdef CORE_IMPLEMENTATION
{
    char * new = NULL;
    unsigned long i = 0;
    assert(strlen(str) == len && "inaccurate length");
    new = core_arena_alloc(arena, len + 1);
    for(i = 0; i <= len; ++i) {
        new[i] = str[i];
    }
    assert(new[len] == 0);
    return new;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

/*
  TODO: finish this function
int core_string_search_replace(char * str, unsigned long strcap, const char * search, const char * replace)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long i = 0;
    const unsigned long slen = strlen(search);
    const unsigned long rlen = strlen(replace);
    
    for(i = 0; i < strcap; ++i) {
        unsigned long remaining_capacity = strcap - i;
        if(core_streql(buf
    }
}
#else
;
#endif
*//*CORE_IMPLEMENTATION*/

/**** BITSET ****/
#define CORE_BITARRAY(n) struct { char bits[(n / CHAR_BIT) + 1]; }
typedef CORE_BITARRAY(8) core_BitArray8;
typedef CORE_BITARRAY(16) core_BitArray16;
typedef CORE_BITARRAY(32) core_BitArray32;
typedef CORE_BITARRAY(64) core_BitArray64;
typedef CORE_BITARRAY(128) core_BitArray128;
typedef CORE_BITARRAY(256) core_BitArray256;
typedef CORE_BITARRAY(512) core_BitArray512;
typedef CORE_BITARRAY(1024) core_BitArray1024;
typedef CORE_BITARRAY(2048) core_BitArray2048;
typedef CORE_BITARRAY(4096) core_BitArray4096;
typedef CORE_BITARRAY(8192) core_BitArray8192;

void core_bitarray_set(void * ptr, unsigned int bit)
#ifdef CORE_IMPLEMENTATION
{
    (void)(ptr);
    (void)(bit);
    CORE_TODO("implement");
}
#else
;
#endif /*CORE_IMPLEMENTATION*/
/*#define CORE_BITSET_SET(bitset, bit) (assert(bit / CHARBIT) < sizeof(bitset->bits), bitset->bits[bit / CHARBIT] >> bit % CHARBIT)*/

typedef struct {
    unsigned char * bits;
    unsigned int len;
} core_BitVec;

void core_bitvec_set(core_BitVec * self, unsigned int bit)
#ifdef CORE_IMPLEMENTATION
{
    const unsigned int index = bit / CHAR_BIT;
    const unsigned char shift = bit % CHAR_BIT;
    const unsigned char byte = (unsigned char)(1 << shift);
    if(self->bits == NULL || self->len == 0) {
        self->len = index + 1;
        self->bits = malloc(sizeof(self->bits[0]) * self->len);
        assert(self->bits);
        memset(self->bits, 0, self->len);
    } else if(index + 1 > self->len) {
        const unsigned int oldlen = self->len;
        self->len = index + 1;
        self->bits = realloc(self->bits, sizeof(self->bits[0]) * self->len);
        assert(self->bits);
        memset(&self->bits[oldlen], 0, self->len - oldlen);
    }
    self->bits[index] |= byte;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


/**** HASH ****/

unsigned long core_hash(const char * key, size_t keylen, unsigned long modulus) 
#ifdef CORE_IMPLEMENTATION
{
    /* Inspired by djbt2 by Dan Bernstein - http://www.cse.yorku.ca/~oz/hash.html */
    unsigned long hash = 5381;
    unsigned long i = 0;

    for(i = 0; i < keylen; ++i) {
        unsigned char c = (unsigned char)key[i];
        assert(c != 0 && "keylen should reflect the location of the null terminator");
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return (hash % modulus);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/


/**** UNTYPEDHASHMAP ****/

typedef struct {
    char * key;
    size_t keylen;
    void * value;
    size_t value_byte_count;
} core_UntypedHashmapEntry;

typedef core_Vec(core_UntypedHashmapEntry) core_UntypedHashmapBucket;

typedef core_Vec(core_UntypedHashmapBucket) core_UntypedHashmapBuckets;

typedef struct {
    core_Arena arena;
    unsigned int num_entries;
    core_UntypedHashmapBuckets buckets;
} core_UntypedHashmap;

#define CORE_UNTYPEDHASHMAP_REHASH_DENSITY_THRESHOLD 0.5f

float core_untypedhashmap_density_calculate(core_UntypedHashmap * self)
#ifdef CORE_IMPLEMENTATION
{
    if(self->buckets.len == 0) return 1.0;
    return (float)self->num_entries / (float)self->buckets.len;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_untypedhashmap_init_capacity(unsigned long initial_capacity, core_UntypedHashmap * result)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long i = 0;
    memset(result, 0, sizeof(core_UntypedHashmap));
    for(i = 0; i < initial_capacity; ++i) {
        core_UntypedHashmapBucket nullbucket = {0};
        core_vec_append(&result->buckets, &result->arena, nullbucket);
    }
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_untypedhashmap_set(core_UntypedHashmap * self, const char * key, size_t keylen, void * value, size_t value_byte_count);

void core_untypedhashmap_rehash(core_UntypedHashmap * self, unsigned long new_capacity)
#ifdef CORE_IMPLEMENTATION
{
    unsigned long i = 0;
    unsigned long j = 0;
    core_UntypedHashmap temp = {0};
    assert(new_capacity > self->buckets.len);
    core_untypedhashmap_init_capacity(new_capacity, &temp);
    for(i = 0; i < self->buckets.len; ++i) {
        core_UntypedHashmapBucket bucket = self->buckets.items[i];
        for(j = 0; j < bucket.len; ++j) {
            core_UntypedHashmapEntry entry = bucket.items[j];
            if(entry.key == NULL) continue;
            core_untypedhashmap_set(&temp, entry.key, entry.keylen, entry.value, entry.value_byte_count);
        }
    }
    core_arena_free(&self->arena);
    *self = temp;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

#ifdef CORE_IMPLEMENTATION
core_UntypedHashmapBucket * core_untypedhashmap_bucket_find(core_UntypedHashmap * self, const char * key, size_t keylen) {
    unsigned long hash = (unsigned long)-1;
    if(core_untypedhashmap_density_calculate(self) > CORE_UNTYPEDHASHMAP_REHASH_DENSITY_THRESHOLD) {
        core_untypedhashmap_rehash(self, (self->buckets.len + 1) * 2);
    }
    hash = core_hash(key, keylen, self->buckets.len);
    return &self->buckets.items[hash];
}
#endif /*CORE_IMPLEMENTATION*/

CORE_NODISCARD
core_Bool core_untypedhashmap_get(core_UntypedHashmap * self, const char * key, size_t keylen, void * result, size_t result_byte_count)
#ifdef CORE_IMPLEMENTATION
{
    core_UntypedHashmapBucket * bucket = core_untypedhashmap_bucket_find(self, key, keylen);
    unsigned long i = 0;
    for(i = 0; i < bucket->len; ++i) {
        core_UntypedHashmapEntry * entry = &bucket->items[i];
        if(core_strneql(entry->key, key, keylen)) {
            assert(entry->value_byte_count == result_byte_count);
            memcpy(result, entry->value, result_byte_count);
            return CORE_TRUE;
        }
    }

    return CORE_FALSE;
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_untypedhashmap_set(core_UntypedHashmap * self, const char * key, size_t keylen, void * value, size_t value_byte_count)
#ifdef CORE_IMPLEMENTATION
{
    core_UntypedHashmapBucket * bucket = core_untypedhashmap_bucket_find(self, key, keylen);
    unsigned long i = 0;
    for(i = 0; i < bucket->len; ++i) {
        core_UntypedHashmapEntry * entry = &bucket->items[i];
        if(core_strneql(entry->key, key, keylen)) {
            assert(entry->value_byte_count == value_byte_count);
            memcpy(entry->value, value, value_byte_count);
            return;
        }
    }
    /*no matching key found, append a new one*/
    {
        core_UntypedHashmapEntry entry = {0};
        entry.key = core_strdup_via_arena(&self->arena, key, keylen);
        entry.keylen = keylen;
        entry.value = core_arena_alloc(&self->arena, value_byte_count);
        memcpy(entry.value, value, value_byte_count);
        entry.value_byte_count = value_byte_count;
        core_vec_append(bucket, &self->arena, entry);
    }
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

void core_untypedhashmap_free(core_UntypedHashmap * self)
#ifdef CORE_IMPLEMENTATION
{
    core_arena_free(&self->arena);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/

#define core_Hashmap(Type) struct { core_UntypedHashmap backing; Type temp; Type * temp_ptr; }

#define core_hashmap_set(self, key, keylen, value) do {                                              \
        (self)->temp = value;                                                                        \
        core_untypedhashmap_set(&(self)->backing, key, keylen, &(self)->temp, sizeof((self)->temp)); \
    } while (0)

#define core_hashmap_get(self, key, keylen, result_ptr) (                                            \
        (self)->temp_ptr = result_ptr, /*Typecheck result_ptr*/                                      \
        core_untypedhashmap_get(&(self)->backing, key, keylen, result_ptr, sizeof(*result_ptr))      \
    )

#define core_hashmap_free(self) core_untypedhashmap_free(&(self)->backing)


/**** STAT ****/
typedef time_t core_Time;
core_Time core_file_modified_timestamp(const char * path);

#ifdef CORE_IMPLEMENTATION

#if defined(CORE_UNIX)
    #include <sys/stat.h>
    core_Time core_file_modified_timestamp(const char * path) {
        struct stat st;
        if(stat(path, &st) != 0) return -1;
        return st.st_mtime;
    }
#elif defined(CORE_WINDOWS)
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <io.h>
    core_Time core_file_modified_timestamp(const char * path) {
        int fd = _open(path, _O_RDONLY);
        struct _stat st;
        if(_fstat(fd, &st) != 0) return -1;
        return st.st_mtime;
    }
#endif

#endif /*CORE_IMPLEMENTATION*/

/**** ACCESS ****/
#if defined(CORE_UNIX)
#include <unistd.h>
core_Bool core_file_exists(const char * path)
#ifdef CORE_IMPLEMENTATION
{
    return (access(path, F_OK) == 0);
}
#else
;
#endif /*CORE_IMPLEMENTATION*/
#endif /*CORE_UNIX*/

#endif /*_CORE_H_*/

