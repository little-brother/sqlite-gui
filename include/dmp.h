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

// dmp.c
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#define dmp_min(A,B)      (((A) < (B)) ? (A) : (B))
#define dmp_num_cmp(A,B)  (((A) < (B)) ? -1 : ((A) > (B)) ? 1 : 0)

#define START_POOL	8

struct dmp_diff {
	dmp_pool pool;
	dmp_range list;
	double deadline;
	/* original parameters */
	const char *t1, *t2;
	uint32_t l1, l2;
	/* used by bisect */
	int *v1, *v2;
	uint32_t v_alloc;
};

static double dmp_time(void);

static int diff_main(
	dmp_range *, dmp_diff *, const dmp_options *,
	const char *, uint32_t, const char *, uint32_t);

static int diff_bisect(
	dmp_range *, dmp_diff *, const dmp_options *,
	const char *, uint32_t, const char *, uint32_t);

static int diff_cleanup_merge(dmp_diff *diff, dmp_range *list);

static dmp_diff *alloc_diff(const dmp_options *opts)
{
	dmp_diff *diff = (dmp_diff*)malloc(sizeof(dmp_diff));
	if (!diff)
		return NULL;

	memset(diff, 0, sizeof(*diff));

	diff->deadline = (opts && opts->timeout > 0) ?
		dmp_time() + opts->timeout : -1.0;

	if (dmp_pool_alloc(&diff->pool, START_POOL) < 0) {
		free(diff);
		diff = NULL;
	}

	return diff;
}

int dmp_diff_new(
	dmp_diff **diff_ptr,
	const dmp_options *options,
	const char *text1,
	uint32_t    len1,
	const char *text2,
	uint32_t    len2)
{
	dmp_diff *diff;

	assert(diff_ptr);

	*diff_ptr = diff = alloc_diff(options);
	if (!diff)
		return -1;

	diff->t1 = text1;
	diff->l1 = len1;
	diff->t2 = text2;
	diff->l2 = len2;

	return diff_main(&diff->list, diff, options, text1, len1, text2, len2);
}

int dmp_diff_from_strs(
	dmp_diff **diff,
	const dmp_options *options,
	const char *text1,
	const char *text2)
{
	if (!text1)
		text1 = "";
	if (!text2)
		text2 = "";

	return dmp_diff_new(
		diff, options, text1, strlen(text1), text2, strlen(text2));
}

static int diff_main(
	dmp_range  *out,
	dmp_diff  *diff,
	const dmp_options *opts,
	const char *text1,
	uint32_t    len1,
	const char *text2,
	uint32_t    len2)
{
	const char *t_short, *t_long, *found;
	uint32_t l_short, l_long, common;
	dmp_pool *pool = &diff->pool;

	/* check for one-sided diffs */

	if (!text1 || !len1) {
		dmp_range_init(
			pool, out, DMP_DIFF_INSERT, text2, 0, len2);
		return pool->error;
	}

	if (!text2 || !len2) {
		dmp_range_init(
			pool, out, DMP_DIFF_DELETE, text1, 0, len1);
		return pool->error;
	}

	/* allocate sentinel */
	if (dmp_range_init(pool, out, DMP_DIFF_EQUAL, text1, len1, 0) < 0)
		goto finish;

	/* trim common prefix */

	common = dmp_common_prefix(text1, len1, text2, len2);
	if (common > 0) {
		dmp_range_insert(
			pool, out, -1, DMP_DIFF_EQUAL, text1, 0, common);

		text1 += common;
		len1  -= common;
		text2 += common;
		len2  -= common;
	}

	/* trim common suffix */

	common = dmp_common_suffix(text1, len1, text2, len2);
	if (common > 0) {
		dmp_range_insert(
			pool, out, out->end,
			DMP_DIFF_EQUAL, text1, len1 - common, common);

		len1 -= common;
		len2 -= common;
	}

	/* after trimming, check for degenerate cases */

	if (!len1) {
		if (len2)
			dmp_range_insert(
				pool, out, -1, DMP_DIFF_INSERT, text2, 0, len2);
		goto finish;
	} else if (!len2) {
		dmp_range_insert(
			pool, out, -1, DMP_DIFF_DELETE, text1, 0, len1);
		goto finish;
	}

	/* check for "common middle" - i.e. one text inside the other */

	if (len1 <= len2) {
		t_short = text1;
		l_short = len1;
		t_long  = text2;
		l_long  = len2;
	} else {
		t_short = text2;
		l_short = len2;
		t_long  = text1;
		l_long  = len1;
	}

	if ((found = dmp_strstr(t_long, l_long, t_short, l_short)) != NULL) {
		int op = (t_short == text1) ? DMP_DIFF_INSERT : DMP_DIFF_DELETE;
		uint32_t found_at = (found - t_long);

		dmp_range_insert(
			pool, out, -1, op, t_long, 0, found_at);
		dmp_range_insert(
			pool, out, -1, DMP_DIFF_EQUAL, t_short, 0, l_short);
		found_at += l_short;
		dmp_range_insert(
			pool, out, -1, op, t_long, found_at, l_long - found_at);

		goto finish;
	}

	if (l_short == 1) {
		/* this speed up applies after testing for short inside long above */
		dmp_range_insert(
			pool, out, -1, DMP_DIFF_DELETE, text1, 0, len1);
		dmp_range_insert(
			pool, out, -1, DMP_DIFF_INSERT, text2, 0, len2);
		goto finish;
	}

	/* TODO: "half match" and "line mode" optimizations */

	/* full Myers bisect diff */

	if (!pool->error)
		diff_bisect(out, diff, opts, text1, len1, text2, len2);

	if (!pool->error)
		diff_cleanup_merge(diff, out);

finish:
	dmp_range_normalize(pool, out);

	return pool->error;
}

static int diff_bisect_split(
	dmp_range *out,
	dmp_diff *diff,
	const dmp_options *opts,
	const char *t1,
	int t1pivot,
	uint32_t t1len,
	const char *t2,
	int t2pivot,
	uint32_t t2len)
{
	dmp_range l1, l2;
	int rv = diff_main(&l1, diff, opts, t1, t1pivot, t2, t2pivot);

	if (rv == 0)
		rv = diff_main(&l2, diff, opts,
			t1 + t1pivot, t1len - t1pivot, t2 + t2pivot, t2len - t2pivot);

	if (rv == 0) {
		dmp_range_splice(&diff->pool, out, -1, &l1);
		dmp_range_splice(&diff->pool, out, -1, &l2);
	}

	return rv;
}

/* bisect diff - find "middle snake" of a diff
 * See Myers 1986: An O(ND) Difference Algorithm and Its Variations.
 */
static int diff_bisect(
	dmp_range *out,
	dmp_diff *diff,
	const dmp_options *opts,
	const char *t1,
	uint32_t t1len,
	const char *t2,
	uint32_t t2len)
{
	int max_d, v_offset, v_length, d;
	int delta, front, k1start, k1end, k2start, k2end, *v1, *v2;

	v_offset = max_d = (t1len + t2len + 1) / 2;
	v_length = 2 * max_d;
	delta = (int)t1len - (int)t2len;
	front = (delta % 2 != 0);
	k1start = k1end = k2start = k2end = 0;

	if ((int)diff->v_alloc < v_length) {
		size_t asize = v_length * sizeof(int);
		diff->v1 = diff->v1 ? (int*)realloc(diff->v1, asize) : (int*)malloc(asize);
		diff->v2 = diff->v2 ? (int*)realloc(diff->v2, asize) : (int*)malloc(asize);
		diff->v_alloc = v_length;

		if (!diff->v1 || !diff->v2)
			return -1;
	}
	v1 = diff->v1;
	v2 = diff->v2;
	/* initialize arrays to -1 (except v_offset + 1 element to 0) */
	memset(v1, 0xff, v_length * sizeof(int));
	memset(v2, 0xff, v_length * sizeof(int));
	v1[v_offset + 1] = 0;
	v2[v_offset + 1] = 0;

	for (d = 0; d < max_d; d++) {
		int k1, k2;

		/* TODO: bail out if deadline is reached */

		/* advance the front contour */
		for (k1 = -d + k1start; k1 <= d - k1end; k1 += 2) {
			int k1off = v_offset + k1;
			uint32_t x1, y1;

			if (k1 == -d || (k1 != d && v1[k1off - 1] < v1[k1off + 1]))
				x1 = v1[k1off + 1];
			else
				x1 = v1[k1off - 1] + 1;
			y1 = x1 - k1;

			while (x1 < t1len && y1 < t2len && t1[x1] == t2[y1])
				x1++, y1++;

			v1[k1off] = x1;
			if (x1 > t1len) /* ran off the right of the graph */
				k1end += 2;
			else if (y1 > t2len) /* ran off bottom of the graph */
				k1start += 2;
			else if (front) {
				int k2off = v_offset + delta - k1;
				if (k2off >= 0 && k2off < v_length && v2[k2off] != -1) {
					/* mirror x2 onto top-left coordinate system */
					uint32_t x2 = (int)t1len - v2[k2off];
					if (x1 >= x2)
						return diff_bisect_split(
							out, diff, opts, t1, x1, t1len, t2, y1, t2len);
				}
			}
		}

		/* advance the reverse contour */
		for (k2 = -d + k2start; k2 <= d - k2end; k2 += 2) {
			int k2off = v_offset + k2;
			uint32_t x2, y2;

			if (k2 == -d || (k2 != d && v2[k2off - 1] < v2[k2off + 1]))
				x2 = v2[k2off + 1];
			else
				x2 = v2[k2off - 1] + 1;
			y2 = x2 - k2;

			while (x2 < t1len && y2 < t2len &&
				   t1[t1len - x2 - 1] == t2[t2len - y2 - 1])
				x2++, y2++;

			v2[k2off] = x2;
			if (x2 > t1len) /* ran off the left of the graph */
				k2end += 2;
			else if (y2 > t2len) /* ran off top of the graph */
				k2start += 2;
			else if (!front) {
				int k1off = v_offset + delta - k2;
				if (k1off >= 0 && k1off < v_length && v1[k1off] != -1) {
					/* mirror x2 onto top-left coordinate system */
					uint32_t x1 = v1[k1off], y1 = v_offset + x1 - k1off;
					x2 = t1len - x2;
					if (x1 >= x2)
						return diff_bisect_split(
							out, diff, opts, t1, x1, t1len, t2, y1, t2len);
				}
			}
		}
	}

	/* diff took too long or # diffs == # chars (i.e. no commonality) */
	dmp_range_insert(&diff->pool, out, -1, DMP_DIFF_DELETE, t1, 0, t1len);
	dmp_range_insert(&diff->pool, out, -1, DMP_DIFF_INSERT, t2, 0, t2len);

	return diff->pool.error;
}

static int diff_cleanup_merge(dmp_diff *diff, dmp_range *list)
{
	dmp_pool *pool = &diff->pool;
	int i, j, before, common, changes;
	int count_delete, count_insert, len_delete, len_insert;
	dmp_node *ins = NULL, *del = NULL, *last = NULL, *node, *next;

	count_insert = count_delete = 0;
	len_insert = len_delete = 0;
	before = -1;

	dmp_range_normalize(pool, list);

	/* ensure EQUAL at end to guarantee termination of cleanup passes */
	node = dmp_node_at(pool, list->end);
	if (node->op != DMP_DIFF_EQUAL)
		dmp_range_insert(
			pool, list, -1, DMP_DIFF_EQUAL, node->text, node->len, 0);

	/* first pass - look for groups of consecutive inserts and deletes
	 * that can be merged or that have unnoticed common prefixes/suffixes
	 * that can be extracted
	 */

	for (i = list->start; i != -1; i = j) {
		node = dmp_node_at(pool, i);
        j = node->next;

		switch (node->op) {
		case DMP_DIFF_INSERT:
			count_insert++;
			len_insert += node->len;
			if (!ins)
				ins = node;
			else {
				last->next = node->next; /* collapse node */
				dmp_node_release(pool, i);
			}
			break;
		case DMP_DIFF_DELETE:
			count_delete++;
			len_delete += node->len;
			if (!del)
				del = node;
			else {
				last->next = node->next; /* collapse node */
				dmp_node_release(pool, i);
			}
			break;
		case DMP_DIFF_EQUAL:
			if (count_delete + count_insert > 0) {
				if (count_delete > 0 && count_insert > 0) {
					/* factor out common prefix */
					common = dmp_common_prefix(
						ins->text, len_insert, del->text, len_delete);

					if (common > 0) {
						if (before == -1) {
							dmp_range_insert(pool, list, 0,
								DMP_DIFF_EQUAL, ins->text, 0, common);
						} else {
							last = dmp_node_at(pool, before);
							last->len += common;
						}
						ins->text  += common;
						len_insert -= common;
						del->text  += common;
						len_delete -= common;
					}

					/* factor out common suffix */
					common = dmp_common_suffix(
						ins->text, len_insert, del->text, len_delete);
					if (common > 0) {
						node->text -= common;
						node->len  += common;
						len_insert -= common;
						len_delete -= common;
					}
				}
				/* merge deletes */
				if (del)
					del->len = len_delete;
				/* merge inserts */
				if (ins)
					ins->len = len_insert;
			}
			else if (last && last->op == DMP_DIFF_EQUAL) {
				/* merge this equality with the previous one */
				last->len += node->len;
				last->next = node->next;
				dmp_node_release(pool, i);
			}

			count_insert = count_delete = 0;
			len_insert = len_delete = 0;
			ins = del = NULL;
			before = i;
			break;
		default:
			/* skip me */
			break;
		}

		last = node;
	}

	/* second pass - look for single edits surrounded by equalities
	 * which can be shifted sideways to eliminate an equality
	 */
	last = dmp_node_at(pool, list->start);
	next = (last->next < 0) ? NULL : dmp_node_at(pool, last->next);
	changes = 0;

	for (i = last->next; next != NULL && i != -1; i = node->next) {
		node = next;
		if (node->next < 0)
			break;
		next = dmp_node_at(pool, node->next);

		if (last->op == DMP_DIFF_EQUAL && next->op == DMP_DIFF_EQUAL) {
			if (last->len > 0 &&
				dmp_has_suffix(node->text, node->len, last->text, last->len))
			{
				node->text -= last->len;
				next->text -= last->len;
				next->len += last->len;
				last->len = 0;
				changes++;
			}
			else if (next->len > 0 &&
				dmp_has_prefix(node->text, node->len, next->text, next->len))
			{
				last->len += next->len;
				node->text += next->len;
				next->len = 0;
				changes++;
			}
		}

		last = node;
	}

	/* remove 0-len nodes */
	dmp_range_normalize(pool, list);

	/* if shifts were made, diff needs reordering and another shift sweep */
	if (changes > 0)
		return diff_cleanup_merge(diff, list);

	return pool->error;
}

void dmp_diff_free(dmp_diff *diff)
{
	free(diff->v1);
	free(diff->v2);
	dmp_pool_free(&diff->pool);
	free(diff);
}

int dmp_diff_foreach(
	const dmp_diff *diff,
	dmp_diff_callback cb,
	void *cb_ref)
{
	int pos, rval = 0;
	const dmp_node *node;

	dmp_range_foreach(&diff->pool, &diff->list, pos, node) {
		if ((rval = cb(cb_ref, (dmp_operation_t)node->op, node->text, node->len)) != 0)
			break;
	}

	return rval;
}

uint32_t dmp_diff_hunks(const dmp_diff *diff)
{
	int pos;
	const dmp_node *node;
	uint32_t count = 0;

	dmp_range_foreach(&diff->pool, &diff->list, pos, node)
		count++;

	return count;
}

static void print_bytes(FILE *fp, const char *bytes, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; ++i) {
		char ch = bytes[i];
		if (isprint(ch))
			fprintf(fp, "%c", ch);
		else
			fprintf(fp, "\\x%02x", ((unsigned int)ch) & 0x00ffu);
	}
}

void dmp_diff_print_raw(FILE *fp, const dmp_diff *diff)
{
	int pos, ct = 0, ct0 = 0;
	const dmp_node *node;

	fputs("\n> \"", fp);
	print_bytes(fp, diff->t1, diff->l1);
	fputs("\"\n", fp);

	for (pos = diff->list.start; pos >= 0; pos = node->next) {
		node = dmp_node_at(&diff->pool,pos);
		ct0++;
		if (node->len > 0)
			ct++;
		fprintf(fp, "%c\"", (node->op < 0) ? '-' : (node->op > 0) ? '+' : '=');
		print_bytes(fp, node->text, node->len);
		fputs(node->next >= 0 ? "\", " : "\"\n", fp);
	}

	fputs("< \"", fp);
	print_bytes(fp, diff->t2, diff->l2);
	fputs("\"\n", fp);
}

int dmp_options_init(dmp_options *opts)
{
	opts->timeout = 1.0F;
	opts->edit_cost = 4;
	opts->match_threshold = 0.5F;
	opts->match_distance = 1000.0F;
	opts->patch_delete_threshold = 0.5F;
	opts->patch_margin = 4;
	opts->match_maxbits = 32;
	opts->check_lines = 1;
	opts->trim_common_prefix = 1;
	opts->trim_common_suffix = 1;
	return 0;
}

uint32_t dmp_common_prefix(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2)
{
	const char *start = t1;
	const char *end   = t1 + dmp_min(l1, l2);

	for (; t1 < end && *t1 == *t2; t1++, t2++);

	return (uint32_t)(t1 - start);
}

uint32_t dmp_common_suffix(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2)
{
	const char *start;

	if (l1 > l2) {
		const char *tswap = t1; t1 = t2; t2 = tswap;
		uint32_t lswap = l1; l1 = l2; l2 = lswap;
	}

	start = t1;

	for (t1 = t1+l1-1, t2 = t2+l2-1; t1 >= start && *t1 == *t2; t1--, t2--);

	return (uint32_t)((start + l1 - 1) - t1);
}

int dmp_strcmp(
	const char *t1, uint32_t l1, const char *t2, uint32_t l2)
{
	int cmp = memcmp(t1, t2, dmp_min(l1, l2));
	return (cmp != 0) ? cmp : dmp_num_cmp(l1, l2);
}

int dmp_has_prefix(
	const char *text, uint32_t tlen, const char *pfx, uint32_t plen)
{
	if (plen > tlen)
		return 0;

	for (; plen > 0; --plen, ++text, ++pfx)
		if (*text != *pfx)
			return 0;

	return 1;
}

int dmp_has_suffix(
	const char *text, uint32_t tlen, const char *sfx, uint32_t slen)
{
	if (slen > tlen)
		return 0;

	for (text = text + tlen - 1, sfx = sfx + slen - 1;
		 slen > 0; --slen, --text, --sfx)
		if (*text != *sfx)
			return 0;

	return 1;
}

/* Railgun is a fast memmem search */

/* All Railgun variants are written by Georgi 'Kaze', they are free,
 * however I expect the user to mention its homepage, that is:
 * http://www.sanmayce.com/Railgun/index.html
 *
 * Author's email: sanmayce@sanmayce.com
 *
 * Caution: For better speed the case 'if (cbPattern==1)' was removed,
 * so Pattern must be longer than 1 char.
 */
static const char *Railgun_Doublet(
	const char * pbTarget, const char * pbPattern,
	uint32_t cbTarget, uint32_t cbPattern)
{
	const char * pbTargetMax = pbTarget + cbTarget;
	register uint32_t ulHashPattern;
	uint32_t count, countSTATIC;

	if (cbPattern > cbTarget) return(NULL);

	countSTATIC = cbPattern-2;

	pbTarget = pbTarget+cbPattern;
	ulHashPattern = (*(uint16_t *)(pbPattern));

	for ( ;; ) {
		if ( ulHashPattern == (*(uint16_t *)(pbTarget-cbPattern)) ) {
			count = countSTATIC;
			while ( count && *(char *)(pbPattern+2+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+2+(countSTATIC-count)) ) {
				count--;
			}
			if ( count == 0 ) return((pbTarget-cbPattern));
		}
		pbTarget++;
		if (pbTarget > pbTargetMax) return(NULL);
	}
}

const char *dmp_strstr(
	const char *haystack, uint32_t lh, const char *needle, uint32_t ln)
{
	switch (ln) {
	case 0:
		return haystack;
	case 1:
		return (const char*)memchr(haystack, *needle, lh);
	default:
		return Railgun_Doublet(haystack, needle, lh, ln);
	}
}

/*
 * Platform specific stuff
 */

#ifdef _WIN32

#include <windows.h>

static double dmp_time(void)
{
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}

#else

#include <sys/time.h>

static double dmp_time(void)
{
	struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (double)tv.tv_sec + tv.tv_usec * 1E-6;
}
#endif // _WIN32

// dmp_pool.c
#define MIN_POOL	2
#define MAX_POOL_INCREMENT	128

int dmp_pool_alloc(dmp_pool *pool, uint32_t start_pool)
{
	memset(pool, 0, sizeof(*pool));

	if (start_pool < MIN_POOL)
		start_pool = MIN_POOL;

	pool->pool = (dmp_node*)calloc(start_pool, sizeof(dmp_node));
	if (!pool->pool)
		return -1;

	pool->pool_size = start_pool;
	pool->pool_used = 1; /* set aside first item */
	pool->free_list = -1;

	return 0;
}

void dmp_pool_free(dmp_pool *pool)
{
	free(pool->pool);
}

void dmp_node_release(dmp_pool *pool, dmp_pos idx)
{
	dmp_node *node = dmp_node_at(pool, idx);
	node->next = pool->free_list;
	pool->free_list = idx;
}

static dmp_pos grow_pool(dmp_pool *pool)
{
	uint32_t new_size;
	dmp_node *new_pool;

	if (pool->pool_size > MAX_POOL_INCREMENT)
		new_size = pool->pool_size + MAX_POOL_INCREMENT;
	else
		new_size = pool->pool_size * 2;

	new_pool = (dmp_node*)realloc(pool->pool, new_size * sizeof(dmp_node));
	if (!new_pool) {
		pool->error = -1;
		return -1;
	}

	pool->pool = new_pool;
	pool->pool_size = new_size;

	return pool->pool_used;
}

static dmp_pos alloc_node(
	dmp_pool *pool, int op, const char *data, uint32_t offset, uint32_t len)
{
	dmp_pos   pos;
	dmp_node *node;

	assert(pool && data && op >= -1 && op <= 1);

	/* don't insert zero length INSERT or DELETE ops */
	if (len == 0 && op != 0)
		return -1;

	if (pool->free_list > 0) {
		pos = pool->free_list;
		node = dmp_node_at(pool, pos);
		pool->free_list = node->next;
	}
	else {
		if (pool->pool_used >= pool->pool_size)
			(void)grow_pool(pool);

		pos = pool->pool_used;
		pool->pool_used += 1;
		node = dmp_node_at(pool, pos);
	}

	node->text = data + offset;
	node->len  = len;
	node->op   = op;
	node->next = -1;

#ifdef BUGALICIOUS
	if (len > 0)
		fprintf(stderr, "adding <%c'%.*s'> (len %d) %02x\n",
				!node->op ? '=' : node->op < 0 ? '-' : '+',
				node->len, node->text, node->len, (int)*node->text);
#endif

	return pos;
}

dmp_pos dmp_range_init(
	dmp_pool *pool, dmp_range *run,
	int op, const char *data, uint32_t offset, uint32_t len)
{
	run->start = run->end = alloc_node(pool, op, data, offset, len);
	return run->start;
}

dmp_pos dmp_range_insert(
	dmp_pool *pool, dmp_range *run, dmp_pos pos,
	int op, const char *data, uint32_t offset, uint32_t len)
{
	dmp_node *node;
	dmp_pos added_at = alloc_node(pool, op, data, offset, len);
	if (added_at < 0)
		return pos;

	node = dmp_node_at(pool, added_at);

	if (pos == -1) {
		dmp_node *end = dmp_node_at(pool, run->end);
		node->next = end->next;
		end->next  = added_at;
		run->end   = added_at;
	}
	else if (pos == 0) {
		node->next = run->start;
		run->start = added_at;
	}
	else {
		dmp_node *after = dmp_node_at(pool, pos);
		node->next  = after->next;
		after->next = added_at;
	}

	return added_at;
}

void dmp_range_splice(
	dmp_pool *pool, dmp_range *onto, dmp_pos pos, dmp_range *from)
{
	dmp_node *tail;

	dmp_range_normalize(pool, from);

	tail = dmp_node_at(pool, from->end);

	if (pos == -1) {
		dmp_node *after = dmp_node_at(pool, onto->end);
		tail->next  = after->next;
		after->next = from->start;
		onto->end   = from->end;
	}
	else if (pos == 0) {
		tail->next  = onto->start;
		onto->start = from->start;
	}
	else {
		dmp_node *after = dmp_node_at(pool, pos);
		tail->next  = after->next;
		after->next = from->start;
	}
}

int dmp_range_len(dmp_pool *pool, dmp_range *run)
{
	int count = 0;
	dmp_pos scan;

	for (scan = run->start; scan != -1; ) {
		dmp_node *node = dmp_node_at(pool, scan);
		count++;
		scan = node->next;
	}

	return count;
}

void dmp_range_normalize(dmp_pool *pool, dmp_range *range)
{
	dmp_pos last_nonzero = -1, *pos = &range->start;

	while (*pos != -1) {
		dmp_node *node = dmp_node_at(pool, *pos);
		if (!node->len) {
			*pos = node->next;
			dmp_node_release(pool, dmp_node_pos(pool, node));
		} else {
			last_nonzero = *pos;
			pos = &node->next;
		}
	}

	if (last_nonzero >= 0)
		range->end = last_nonzero;
}

#ifdef __cplusplus
}
#endif

#endif
