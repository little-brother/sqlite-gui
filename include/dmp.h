// C language port of Neil Fraser's google-diff-match-patch code.
// Based on https://github.com/arrbee/diff-match-patch-c + @udif fix
#ifndef DMP_H
#define DMP_H

#ifdef __cplusplus
extern "C" {
#endif


#define DMP_VERSION "0.1.1"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Public: Each hunk of diff describes one of these operations.
 */
typedef enum {
	DMP_DIFF_DELETE = -1,
	DMP_DIFF_EQUAL = 0,
	DMP_DIFF_INSERT = 1
} dmp_operation_t;

/**
 * Public: Options structure configures behavior of diff functions.
 */
typedef struct {
    /* Number of seconds to map a diff before giving up (0 for infinity). */
    float timeout; /* = 1.0 */

	/* Cost of an empty edit operation in terms of edit characters. */
    int edit_cost; /* = 4 */

	/* At what point is no match declared (0.0 = perfection, 1.0 = very
	 * loose).
	 */
    float match_threshold; /* = 0.5 */

	/* How far to search for a match (0 = exact location, 1000+ = broad match).
	 * A match this many characters away from the expected location will add
	 * 1.0 to the score (0.0 is a perfect match).
	 */
    float match_distance; /* = 1000 */

	/* When deleting a large block of text (over ~64 characters), how close
	 * do the contents have to be to match the expected contents. (0.0 =
	 * perfection, 1.0 = very loose).  Note that match_threshold controls
	 * how closely the end points of a delete need to match.
	 */
    float patch_delete_threshold; /* = 0.5 */

	/* Chunk size for context length. */
    int patch_margin; /* = 4 */

	/* The number of bits in an int.
	 * Python has no maximum, thus to disable patch splitting set to 0.
	 * However to avoid long patches in certain pathological cases, use 32.
	 * Multiple short patches (using native ints) are much faster than long
	 * ones.
	 */
    int match_maxbits; /* = 32 */

	/* Should diff run an initial line-level diff to identify changed areas?
	 * Running initial diff will be slightly faster but slightly less optimal.
	 */
	int check_lines; /* = 1 */

	/* Should the diff trim the common prefix? */
	int trim_common_prefix; /* = 1 */

	/* Should the diff trim the common suffix? */
	int trim_common_suffix; /* = 1 */
} dmp_options;

/**
 * Public: Main diff object.
 *
 * This is an opaque structure.  It is internally a linked list of diff
 * records, each tracking one of the operations listed above, along with
 * pointers into the original text data and run lenths for the diff
 * records.
 */
typedef struct dmp_diff dmp_diff;

typedef struct dmp_patch dmp_patch;

/**
 * Public: Callback function for iterating over a diff.
 *
 * When you call `dmp_diff_foreach`, pass a function with this signature
 * to iterate over the diff records.  If the `op` is a DELETE, the `data`
 * pointer will be into the `text1` original text.  If the `op` is an
 * INSERT, the pointer will be into the `text2` new text.  If the `op` is
 * an EQUAL, we generally attempt to keep the pointer into the `text1`
 * original text, but that is not guaranteed.
 *
 * cb_ref - The reference pointer you passed to the foreach fn.
 * op - A `dmp_operation_t` value for the chunk of data.
 * data - Pointer to the diff data as described above.  This data will
 *        generally not be NUL-terminated, since it is a reference into
 *        the original data.  You must use the `len` parameter correctly.
 * len - Bytes of data after the pointer in this chunk.
 *
 * Returns 0 to keep iterator or non-zero to stop iteration.  Any value
 * you return will be passed back from the foreach function.
 */
typedef int (*dmp_diff_callback)(
	void *cb_ref, dmp_operation_t op, const void *data, uint32_t len);

/**
 * Public: Initialize options structure to default values.
 *
 * This initializes a `dmp_options` structure for passing into the various
 * functions that take options.  After initialization, you should set the
 * parameters explicitly that you wish to change.
 *
 * opts - Structure to be initialized, generally created on the stack.
 *
 * Returns 0 on success, -1 on failure.
 */
     int dmp_options_init(dmp_options *opts);

/**
 * Public: Calculate the diff between two texts.
 *
 * This will allocate and populate a new `dmp_diff` object with records
 * describing how to transform `text1` into `text2`.  This returns a diff
 * with byte-level differences between the two texts.  You can use one of
 * the diff transformation functions below to modify the diffs to word or
 * line level diffs, or to align diffs to UTF-8 boundaries or the like.
 *
 * diff - Pointer to a `dmp_diff` pointer that will be allocated.  You must
 *        call `dmp_diff_free()` on this pointer when done.
 * options - `dmp_options` structure to control diff, or NULL to use defaults.
 * text1 - The FROM text for the left side of the diff.
 * len1 - The number of bytes of data in `text1`.
 * text2 - The TO text for the right side of the diff.
 * len2 - The number of bytes of data in `text2`.
 *
 * Returns 0 if the diff was successfully generated, -1 on failure.  The
 * only current failure scenario would be a failed allocation.  Otherwise,
 * some sort of diff should be generated..
 */
     int dmp_diff_new(
	dmp_diff **diff,
	const dmp_options *options,
	const char *text1,
	uint32_t    len1,
	const char *text2,
	uint32_t    len2);

/**
 * Public: Generate diff from NUL-terminated strings.
 *
 * This is a convenience function when you know that you are diffing
 * NUL-terminated strings.  It simply calls `strlen()` and passes the
 * results along to `dmp_diff_new` (plus it deals correctly with NULL
 * strings, passing them in a zero-length texts).
 *
 * diff - Pointer to a `dmp_diff` pointer that will be allocated.  You must
 *        call `dmp_diff_free()` on this pointer when done.
 * options - `dmp_options` structure to control diff, or NULL to use defaults.
 * text1 - The FROM string for the left side of the diff.  Must be a regular
 *         NUL-terminated C string.
 * text2 - The TO string for the right side of the diff.  Must be a regular
 *         NUL-terminated C string.
 *
 * Returns 0 if the diff was successfully generated, -1 on failure.  The
 * only current failure scenario would be a failed allocation.  Otherwise,
 * some sort of diff should be generated..
 */
     int dmp_diff_from_strs(
	dmp_diff **diff,
	const dmp_options *options,
	const char *text1,
	const char *text2);

/**
 * Public: Free the diff structure.
 *
 * Call this when you are done with the diff data.
 *
 * diff - The `dmp_diff` object to be freed.
 */
     void dmp_diff_free(dmp_diff *diff);

/**
 * Public: Iterate over changes in a diff list.
 *
 * Invoke a callback on each hunk of a diff.
 *
 * diff - The `dmp_diff` object to iterate over.
 * cb - The callback function to invoke on each hunk.
 * cb_ref - A reference pointer that will be passed to callback.
 *
 * Returns 0 if iteration completed successfully, or any non-zero value
 * that was returned by the `cb` callback function to terminate iteration.
 */
     int dmp_diff_foreach(
	const dmp_diff *diff,
	dmp_diff_callback cb,
	void *cb_ref);

/**
 * Public: Count the number of diff hunks.
 *
 * This computes the number of hunks in a diff object.  This is the
 * number of times that your iterator function would be invoked.
 *
 * diff - The `dmp_diff` object.
 *
 * Returns a count of the number of hunks in the diff.
 */
     uint32_t dmp_diff_hunks(const dmp_diff *diff);

     void dmp_diff_print_raw(FILE *fp, const dmp_diff *diff);

     int dmp_patch_new(
	dmp_patch     **patch,
	const char      *text1,
	uint32_t         len1,
	const dmp_diff *diff);

     void dmp_patch_free(dmp_patch *patch);

/*
 * Utility functions
 */

     uint32_t dmp_common_prefix(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2);

     uint32_t dmp_common_suffix(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2);

     int dmp_has_prefix(
	const char *text, uint32_t tlen, const char *pfx, uint32_t plen);

     int dmp_has_suffix(
	const char *text, uint32_t tlen, const char *sfx, uint32_t slen);

     int dmp_strcmp(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2);

     const char *dmp_strstr(
	const char *haystack, uint32_t lh, const char *needle, uint32_t ln);

     void dmp_build_texts_from_diff(
	char **t1, uint32_t *l1, char **t2, uint32_t *l2, const dmp_diff *diff);


// dmp_pool.h
typedef int dmp_pos;

typedef struct {
	const char *text;
	uint32_t len;
	int op;
	dmp_pos next;
} dmp_node;

typedef struct {
	dmp_pos start, end;
} dmp_range;

typedef struct {
	dmp_node *pool;
	uint32_t pool_size, pool_used;
	dmp_pos free_list;
	int error;
} dmp_pool;

     int dmp_pool_alloc(dmp_pool *pool, uint32_t start_pool);

     void dmp_pool_free(dmp_pool *list);

     dmp_pos dmp_range_init(
	dmp_pool *list, dmp_range *run,
	int op, const char *data, uint32_t offset, uint32_t len);

     dmp_pos dmp_range_insert(
	dmp_pool *list, dmp_range *run, dmp_pos pos,
	int op, const char *data, uint32_t offset, uint32_t len);

     void dmp_range_splice(
	dmp_pool *list, dmp_range *onto, dmp_pos pos, dmp_range *from);

     int dmp_range_len(dmp_pool *pool, dmp_range *run);

/* remove all 0-length nodes and advance 'end' to actual end */
     void dmp_range_normalize(dmp_pool *pool, dmp_range *range);

     void dmp_node_release(dmp_pool *pool, dmp_pos idx);

#define dmp_node_at(POOL,POS)   (&((POOL)->pool[(POS)]))

#define dmp_node_pos(POOL,NODE) ((dmp_pos)((NODE) - (POOL)->pool))

#define dmp_range_foreach(POOL, RANGE, IDX, PTR) \
	for (IDX = (RANGE)->start; IDX >= 0; IDX = (PTR)->next)	\
		if (((PTR) = dmp_node_at((POOL),IDX))->len > 0)

#ifdef __cplusplus
}
#endif

#endif