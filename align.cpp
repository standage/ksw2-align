#include "ksw2.h"
#include <string>
#include <string.h>

// Stolen shamelessly from ksw2/cli.c
unsigned char seq_nt4_table[256] = {
    0, 1, 2, 3,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
    4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4
};

// Stolen shamelessly from ksw2/cli.c
static void ksw_gen_simple_mat(int m, int8_t *mat, int8_t a, int8_t b)
{
    int i, j;
    a = a < 0? -a : a;
    b = b > 0? -b : b;
    for (i = 0; i < m - 1; ++i) {
        for (j = 0; j < m - 1; ++j)
            mat[i * m + j] = i == j? a : b;
        mat[i * m + m - 1] = 0;
    }
    for (j = 0; j < m; ++j)
        mat[(m - 1) * m + j] = 0;
}

std::string align_call(const char *target, const char *query)
{
    /**
     * Align `query` to `target` using the `ksw_extz` algorithm. See
     * https://github.com/lh3/ksw2 for more info.
     *
     * @param target    reference genome sequence
     * @param query     contig assembled from variant-associated reads
     * @returns         CIGAR string of the alignment
     */
    char cigar[512];
    size_t ci = 0;
    size_t tlen = strlen(target);
    size_t qlen = strlen(query);

    uint8_t score_match = 1;
    uint8_t penalty_mismatch = 2;
    uint8_t penalty_gapopen = 5;
    uint8_t penalty_gap_extend = 0;
    uint8_t alphabet_size = 5;

    uint8_t *ti = (uint8_t *)calloc(strlen(target), 1);
    for (int i = 0; i < tlen; i++) {
        ti[i] = seq_nt4_table[(uint8_t)target[i]];
    }
    uint8_t *qi = (uint8_t *)calloc(strlen(query), 1);
    for (int i = 0; i < qlen; i++) {
        qi[i] = seq_nt4_table[(uint8_t)query[i]];
    }

    int8_t matrix[25];
    ksw_extz_t ez;

    memset(&ez, 0, sizeof(ksw_extz_t));
    ksw_gen_simple_mat(5, matrix, score_match, -penalty_mismatch);

    ksw_extz(
        NULL, // memory pool
        qlen, qi,
        tlen, ti,
        alphabet_size, matrix, penalty_gapopen, penalty_gap_extend,
        -1, // bandwidth
        -1, // zdrop
        0, //flag
        &ez
    );

    // Stolen shamelessly from ksw2/cli.c
    for (int i = 0; i < ez.n_cigar; ++i) {
        ci += sprintf(
            cigar + ci, "%d%c", ez.cigar[i] >> 4,"MID"[ez.cigar[i]&0xf]
        );
    }

    std::string cigarstring(cigar);
    free(qi);
    free(ti);
    return cigarstring;
}
