#include "PointerRemapper.h"
#include "StringHelper.h"
#include "BubbleSort.h"


void _quicksort(void* const pbase, size total_elems, size size, DataCompare cmp);


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void QuickSort(DataCompare fCompare, void* pvArray, size uiStride, int iNumElements)
{
    _quicksort(pvArray, iNumElements, uiStride, fCompare);
}


/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4


/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
{
    char* lo;
    char* hi;
} stack_node;


/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size).  */
#define         STACK_SIZE        ((sizeof(char) * 8) * sizeof (size))
#define         PUSH(low, high)   ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define         POP(low, high)     ((void) (--top, (low = top->lo), (high = top->hi)))
#define         STACK_NOT_EMPTY    (stack < top)


/* Order size using quicksort.  This implementation incorporates
    four optimizations discussed in Sedgewick:
    1. Non-recursive, using an explicit stack of pointer that store the
        next array partition to sort.  To save time, this maximum amount
        of space required to store an array of SIZE_MAX is allocated on the
        stack.  Assuming a 32-bit (64 bit) integer for size, this needs
        only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
        Pretty cheap, actually.
    2. Chose the pivot element using a median-of-three decision tree.
        This reduces the probability of selecting a bad pivot value and
        eliminates certain extraneous comparisons.
    3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
        insertion sort to order the MAX_THRESH items within each partition.
        This is a big win, since insertion sort is faster for small, mostly
        sorted array segments.
    4. The larger of the two sub-partitions is always pushed onto the
        stack first, with the algorithm then concentrating on the
        smaller partition.  This *guarantees* no more than log (total_elems)
        stack size is needed (actually O(1) in this case)!  */

void _quicksort(void* const pbase, size total_elems, size uiSize, DataCompare cmp)
{
    char* base_ptr = (char*)pbase;
    const size max_thresh = MAX_THRESH * uiSize;
    if (total_elems == 0)
        /* Avoid lossage with uint32 arithmetic below.  */
        return;
    if (total_elems > MAX_THRESH)
    {
        char* lo = base_ptr;
        char* hi = &lo[uiSize * (total_elems - 1)];
        stack_node stack[STACK_SIZE];
        stack_node* top = stack;
        PUSH(NULL, NULL);
        while (STACK_NOT_EMPTY)
        {
            char* left_ptr;
            char* right_ptr;
            /* Select median value from among LO, MID, and HI. Rearrange
               LO and HI so the three values are sorted. This lowers the
               probability of picking a pathological pivot value and
               skips a comparison for both the LEFT_PTR and RIGHT_PTR in
               the while loops. */
            char* mid = lo + uiSize * ((hi - lo) / uiSize >> 1);
            if ((*cmp) ((void*)mid, (void*)lo) < 0)
                MemSwp(mid, lo, uiSize);
            if ((*cmp) ((void*)hi, (void*)mid) < 0)
                MemSwp(mid, hi, uiSize);
            else
                goto jump_over;
            if ((*cmp) ((void*)mid, (void*)lo) < 0)
                MemSwp(mid, lo, uiSize);
        jump_over:;
            left_ptr = lo + uiSize;
            right_ptr = hi - uiSize;
            /* Here's the famous ``collapse the walls'' section of quicksort.
               Gotta like those tight inner loops!  They are the main reason
               that this algorithm runs much faster than others. */
            do
            {
                while ((*cmp) ((void*)left_ptr, (void*)mid) < 0)
                    left_ptr += uiSize;
                while ((*cmp) ((void*)mid, (void*)right_ptr) < 0)
                    right_ptr -= uiSize;
                if (left_ptr < right_ptr)
                {
                    MemSwp(left_ptr, right_ptr, uiSize);
                    if (mid == left_ptr)
                        mid = right_ptr;
                    else if (mid == right_ptr)
                        mid = left_ptr;
                    left_ptr += uiSize;
                    right_ptr -= uiSize;
                }
                else if (left_ptr == right_ptr)
                {
                    left_ptr += uiSize;
                    right_ptr -= uiSize;
                    break;
                }
            } while (left_ptr <= right_ptr);
            /* Set up pointers for next iteration.  First determine whether
               left and right partitions are below the threshold uiSize.  If so,
               ignore one or both.  Otherwise, push the larger partition's
               bounds on the stack and continue sorting the smaller one. */
            if ((size)(right_ptr - lo) <= max_thresh)
            {
                if ((size)(hi - left_ptr) <= max_thresh)
                    /* Ignore both small partitions. */
                    POP(lo, hi);
                else
                    /* Ignore small left partition. */
                    lo = left_ptr;
            }
            else if ((size)(hi - left_ptr) <= max_thresh)
                /* Ignore small right partition. */
                hi = right_ptr;
            else if ((right_ptr - lo) > (hi - left_ptr))
            {
                /* Push larger left partition indices. */
                PUSH(lo, right_ptr);
                lo = left_ptr;
            }
            else
            {
                /* Push larger right partition indices. */
                PUSH(left_ptr, hi);
                hi = right_ptr;
            }
        }
    }

    /* Once the BASE_PTR array is partially sorted by quicksort the rest
       is completely sorted using insertion sort, since this is efficient
       for partitions below MAX_THRESH uiSize. BASE_PTR points to the beginning
       of the array to sort, and END_PTR points at the very last element in
       the array (*not* one beyond it!). */
#define min(x, y) ((x) < (y) ? (x) : (y))
    {
        char* const end_ptr = &base_ptr[uiSize * (total_elems - 1)];
        char* tmp_ptr = base_ptr;
        char* thresh = min(end_ptr, base_ptr + max_thresh);
        char* run_ptr;
        /* Find smallest element in first threshold and place it at the
           array's beginning.  This is the smallest array element,
           and the operation speeds up insertion sort's inner loop. */
        for (run_ptr = tmp_ptr + uiSize; run_ptr <= thresh; run_ptr += uiSize)
            if ((*cmp) ((void*)run_ptr, (void*)tmp_ptr) < 0)
                tmp_ptr = run_ptr;
        if (tmp_ptr != base_ptr)
            MemSwp(tmp_ptr, base_ptr, uiSize);
        /* Insertion sort, running from left-hand-side up to right-hand-side.  */
        run_ptr = base_ptr + uiSize;
        while ((run_ptr += uiSize) <= end_ptr)
        {
            tmp_ptr = run_ptr - uiSize;
            while ((*cmp) ((void*)run_ptr, (void*)tmp_ptr) < 0)
                tmp_ptr -= uiSize;
            tmp_ptr += uiSize;
            if (tmp_ptr != run_ptr)
            {
                char* trav;
                trav = run_ptr + uiSize;
                while (--trav >= run_ptr)
                {
                    char c = *trav;
                    char* hi, * lo;
                    for (hi = lo = trav; (lo -= uiSize) >= tmp_ptr; hi = lo)
                        *hi = *lo;
                    *hi = c;
                }
            }
        }
    }
}

