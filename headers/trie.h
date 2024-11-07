#ifndef   TRIE_H
#define   TRIE_H

struct cell {
    int   symbol;
    int   is_term;
    cell* dest;
};

typedef cell trie_t;

enum TRIE_ERRORS {
    PRINT_CELL   = 1<<0,
    ZERO_ROOT    = 1<<1,
};

const size_t DEST_SIZE     = 32;
const size_t ALPHABET_SIZE = 26;
#define ASSERT_TRIE(root) assert(trie_cell_assert(root))

int trie_ctor(cell* root);
int trie_dtor(cell* root);

int trie_cell_assert (const cell* root);
int trie_cell_verify (const cell* root);
int trie_print       (const cell* root, char* prefix, size_t pref_size);
int trie_get_terms   (const cell* root, char* prefix, size_t pref_size, char** buffer, size_t* buffer_size);
int trie_exist       (const cell* root, const char* src);

int trie_insert (cell* root, const char* src);

int trie_equal  (const void* first, const void* second);


#endif // TRIE_H
