#include "paging.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/*  --------------------------------- PROBLEM 6 --------------------------------------
    Checkout PDF section 7 for this problem

    Page fault handler.

    When the CPU encounters an invalid address mapping in a page table,
    it invokes the OS via this handler.

    Your job is to put a mapping in place so that the translation can
    succeed. You can use free_frame() to make an available frame.
    Update the page table with the new frame, and don't forget
    to fill in the frame table.

    Lastly, you must fill your newly-mapped page with data. If the page
    has never mapped before, just zero the memory out. Otherwise, the
    data will have been swapped to the disk when the page was
    evicted. Call swap_read() to pull the data back in.

    HINTS:
         - You will need to use the global variable current_process when
           setting the frame table entry.

    ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t address) {
    /* First, split the faulting address and locate the page table entry.
       Remember to keep a pointer to the entry so you can modify it later. */
    stats.page_faults++;
    vpn_t currVPN = vaddr_vpn(address);
    pte_t *currPageEntry = ((pte_t*) (mem + (PTBR * PAGE_SIZE))) + currVPN;

    /* It's a page fault, so the entry obviously won't be valid. Grab
       a frame to use by calling free_frame(). */
    pfn_t tempFrame = free_frame();

    /* Update the page table entry. Make sure you set any relevant values. */
    currPageEntry->valid = 1;
    currPageEntry->dirty = 0;
    currPageEntry->pfn = tempFrame;

    /* Update the frame table. Make sure you set any relevant values. */
    frame_table[tempFrame].mapped = 1;
    frame_table[tempFrame].timestamp = get_current_timestamp();
    frame_table[tempFrame].vpn = currVPN;
    frame_table[tempFrame].process = current_process;

    /* Initialize the page's memory. On a page fault, it is not enough
     * just to allocate a new frame. We must load in the old data from
     * disk into the frame. If there was no old data on disk, then
     * we need to clear out the memory (why?).
     *
     * 1) Get a pointer to the new frame in memory.
     * 2) If the page has swap set, then we need to load in data from memory
     *    using swap_read().
     * 3) Else, just zero the page's memory. If the page is later written
     *    back, swap_write() will automatically allocate a swap entry.
     */
    // fte_t *currFrame = frame_table + tempFrame;
    if (currPageEntry->swap > 0) {
        swap_read(currPageEntry, (mem + (tempFrame * PAGE_SIZE)));
    } else {
        memset((uint8_t*)(mem + (tempFrame * PAGE_SIZE)), 0, PAGE_SIZE);
    }
}

#pragma GCC diagnostic pop
