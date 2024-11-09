#include<assert.h>
#include<mem.h>
#include<stdlib.h>

#include "commoner.h"
#include "trie.h"

static int trie_cell_dump (const cell* root, int error_mask);

cell* trie_ctor() {
    cell* root = (cell*)calloc(1, sizeof(cell));
    if (!root) return 0;
    root->symbol = '#';

    return root;
}

int trie_dtor(cell* root) {
    ASSERT_TRIE(root);

    if (root->dest) {
        for (size_t i = 0; i < DEST_SIZE; ++i) trie_dtor(root);
        free(root->dest);
    }

    *root = {};
    free(root);

    return 0;
}

int trie_cell_verify(const cell* root) {
    // int error = 0;
    if (!root) return ZERO_ROOT;
    return 0;
}
int trie_cell_assert(const cell* root) {
    return !trie_cell_dump(root, trie_cell_verify(root));
}
int trie_print (const cell* root, char* prefix, size_t pref_size) {
    ASSERT_TRIE(root);

    if (root->is_term) LOG_PURPLE("%s\n", prefix);

    if (!root->dest) return 0;

    LOG_GREEN("%s: ", prefix);
    trie_cell_dump(root, PRINT_CELL);

    if (!root->dest) return 0;

    for(size_t i = 0; i < DEST_SIZE; ++i) {
        prefix[pref_size] = (char)root->dest[i].symbol;
        trie_print(root->dest + i, prefix, pref_size + 1);
    }

    return 0;
}

int trie_get_terms  (const cell* root, char* prefix, size_t pref_size, char** buffer, size_t* buffer_size) {
    ASSERT_TRIE(root);
    assert(prefix);
    assert(buffer);
    assert(buffer_size);

    if (!root->symbol) return 0;

    if (root->is_term) {
        buffer[*buffer_size] = (char*)calloc(pref_size + 1, sizeof(char));
        memcpy(buffer[(*buffer_size)++], prefix, pref_size);
    }

    for(size_t i = 0; i < DEST_SIZE; ++i) {
        prefix[pref_size] = (char)root->dest[i].symbol;
        trie_get_terms(root->dest + i, prefix, pref_size + 1, buffer, buffer_size);
    }

    prefix[pref_size] = '\0';
    return 0;
}

int trie_insert(cell* root, const char* src) {
    ASSERT_TRIE(root);
    assert(src);

    if (!*src) {
        root->is_term = 1;
        return 0;
    }

    if (*src != '_' && (*src < 'A' || *src > 'A' + (char)ALPHABET_SIZE)) return 1;

    if (!root->dest) root->dest = (cell*)calloc(DEST_SIZE, sizeof(cell));

    cell* next = *src == '_' ? root->dest + ALPHABET_SIZE: root->dest + *src - 'A';
    if (!next->symbol) next->symbol = *src;

    return trie_insert(next, src + 1);
}

int trie_exist(const cell* root, const char* src) {
    ASSERT_TRIE(root);
    assert(src);

    if (!*src)       return root->is_term;
    if (!root->dest) return 0;
    cell* next = *src == '_' ? root->dest + ALPHABET_SIZE: root->dest + *src - 'A';

    if (!next->symbol) return 0;

    return trie_exist(next, src + 1);
}

int trie_equal (const void* first, const void* second) {
    const cell* cell_1 = (const cell*)first;
    const cell* cell_2 = (const cell*)second;

    ASSERT_TRIE(cell_1);
    ASSERT_TRIE(cell_2);

    if (cell_1->symbol != cell_2->symbol || cell_1->is_term != cell_2->is_term) return 0;

    if (cell_1->dest == 0 || cell_2->dest == 0) {
        if (cell_2->dest != cell_1->dest) return 0;
        return 1;
    }

    for (size_t i = 0; i < DEST_SIZE; ++i)
        if(!trie_equal(cell_2->dest + i, cell_1->dest + i)) return 0;

    return 1;
}

static int trie_cell_dump (const cell* root, int error_mask) {
    if (error_mask == 0) {
        return 0;
    }
    if (error_mask & ZERO_ROOT) {
        LOG_FATAL("Zero pointer to cell\n");
        return error_mask;
    }
    assert(root);
    if (!(error_mask & PRINT_CELL)) {
        if (root->symbol != '\0') LOG_GREEN("%c : ", root->symbol);
        else                      LOG_GREEN("\\0: ");
    }
    if (!root->dest) return error_mask & ~PRINT_CELL;

    for(size_t i = 0; i < DEST_SIZE; ++i) {
        if (root->dest[i].symbol != '\0') {
            if (root->dest[i].is_term) LOG_PURPLE("%c ", root->dest[i].symbol);
            else                       LOG_CYAN  ("%c ", root->dest[i].symbol);
        }
    }
    LOG_WHITE("\n");
    return error_mask & ~PRINT_CELL;
}
