#include "nds32.h"
#include "cache.h"
#include "string.h"

enum cache_t{ICACHE, DCACHE};

static inline unsigned long CACHE_SET(enum cache_t cache){

	if(cache == ICACHE)
		return 64 << ((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskISET) >> ICM_CFG_offISET);
	else
		return 64 << ((__nds32__mfsr(NDS32_SR_DCM_CFG) & DCM_CFG_mskDSET) >> DCM_CFG_offDSET);
}

static inline unsigned long CACHE_WAY(enum cache_t cache){

	if(cache == ICACHE)
		return 1 + ((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskIWAY) >> ICM_CFG_offIWAY);
	else
		return 1 + ((__nds32__mfsr(NDS32_SR_DCM_CFG) & DCM_CFG_mskDWAY) >> DCM_CFG_offDWAY);
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache){

	if(cache == ICACHE)
		return 8 << (((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskISZ) >> ICM_CFG_offISZ) - 1);
	else
		return 8 << (((__nds32__mfsr(NDS32_SR_DCM_CFG) & DCM_CFG_mskDSZ) >> DCM_CFG_offDSZ) - 1);
}

void nds32_dcache_invalidate(void){

#ifdef CONFIG_CPU_DCACHE_ENABLE
	__nds32__cctl_l1d_invalall();

	__nds32__msync_store();
	__nds32__dsb();
#endif
}

void nds32_dcache_flush(void){

#ifdef CONFIG_CPU_DCACHE_ENABLE
    __nds32__cctl_l1d_wball_one_lvl();
    __nds32__cctl_l1d_invalall();
	__nds32__msync_store();
	__nds32__dsb();
#endif
}

void nds32_dcache_clean(void){

#ifdef CONFIG_CPU_DCACHE_ENABLE
    __nds32__cctl_l1d_wball_one_lvl();
	__nds32__msync_store();
	__nds32__dsb();
#endif
}

void nds32_icache_flush(void){

#ifdef CONFIG_CPU_ICACHE_ENABLE
	unsigned long end;
	unsigned long cache_line = CACHE_LINE_SIZE(ICACHE);

	end = CACHE_WAY(ICACHE) * CACHE_SET(ICACHE) * CACHE_LINE_SIZE(ICACHE);

	do {
		end -= cache_line;
		__nds32__cctlidx_wbinval(NDS32_CCTL_L1I_IX_INVAL, end);
	} while (end > 0);

	__nds32__isb();
#endif
}

#ifdef CONFIG_CHECK_RANGE_ALIGNMENT
#define BUG_ON(condition) do { if (condition) ; } while(0)
#define chk_range_alignment(start, end, line_size) do {	\
							\
	BUG_ON((start) & ((line_size) - 1));		\
	BUG_ON((end) & ((line_size) - 1));		\
	BUG_ON((start) == (end));			\
							\
} while (0);
#else
#define chk_range_alignment(start, end, line_size)
#endif
/* ================================ D-CACHE =============================== */
/*
 * nds32_dcache_clean_range(start, end)
 *
 * For the specified virtual address range, ensure that all caches contain
 * clean data, such that peripheral accesses to the physical RAM fetch
 * correct data.
 */
void nds32_dcache_clean_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
		start += line_size;
	}

	__nds32__msync_store();
	__nds32__dsb();
#endif
#endif
}

void nds32_dma_clean_range(unsigned long start, unsigned long end){

	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;

	nds32_dcache_clean_range(start, end);
}

/*
 * nds32_dcache_invalidate_range(start, end)
 *
 * throw away all D-cached data in specified region without an obligation
 * to write them back. Note however that we must clean the D-cached entries
 * around the boundaries if the start and/or end address are not cache
 * aligned.
 */
void nds32_dcache_invalidate_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_INVAL, (void *)start);
		start += line_size;
	}
#endif
}

void nds32_dcache_flush_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
#endif
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_INVAL, (void *)start);
		start += line_size;
	}
#endif
}

void nds32_dcache_writeback_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
		start += line_size;
	}
#endif
#endif
}
void unaligned_cache_line_move(unsigned char* src, unsigned char* dst, unsigned long len )
{
         unsigned int i;
         unsigned char* src_p = (unsigned char*)src;
         unsigned char* dst_p = (unsigned char*)dst;
         for( i = 0 ;i < len; ++i)
                 *(dst_p+i)=*(src_p+i);
}



void nds32_dma_inv_range(unsigned long start, unsigned long end){
	unsigned long line_size;
	unsigned long old_start=start;
	unsigned long old_end=end;
	line_size = CACHE_LINE_SIZE(DCACHE);
	unsigned char h_buf[line_size];
	unsigned char t_buf[line_size];
	memset((void*)h_buf,0,line_size);
	memset((void*)t_buf,0,line_size);

	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;
	if (start != old_start)
	{
		//nds32_dcache_flush_range(start, start + line_size);
		unaligned_cache_line_move((unsigned char*)start, h_buf, old_start - start);
	}
	if (end != old_end)
	{
		//nds32_dcache_flush_range(end - line_size ,end);
		unaligned_cache_line_move((unsigned char*)old_end, t_buf, end - old_end);

	}
	nds32_dcache_invalidate_range(start, end);

	//handle cache line unaligned problem
	if(start != old_start)
		unaligned_cache_line_move(h_buf,(unsigned char*)start, old_start - start);

	if( end != old_end )
		unaligned_cache_line_move(t_buf,(unsigned char*)old_end, end - old_end);

}


void nds32_dma_flush_range(unsigned long start, unsigned long end){

	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1 ) & (~(line_size-1));
	if (start == end)
		return;

	nds32_dcache_flush_range(start, end);
}

/* ================================ I-CACHE =============================== */
/*
 * nds32_icache_invalidate_range(start, end)
 *
 * invalidate a range of virtual addresses from the Icache
 *
 * This is a little misleading, it is not intended to clean out
 * the i-cache but to make sure that any data written to the
 * range is made consistant. This means that when we execute code
 * in that region, everything works as we expect.
 *
 * This generally means writing back data in the Dcache and
 * write buffer and flushing the Icache over that region
 *
 * start: virtual start address
 * end: virtual end address
 */
void nds32_icache_invalidate_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_ICACHE_ENABLE

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(ICACHE);
	//chk_range_alignment(start, end, line_size);
	start &= (~(line_size-1));
	end = ( end + line_size - 1 )&(~(line_size-1));
	if (end == start)
		end += line_size;

	while (end > start){

		end -= line_size;
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1I_VA_INVAL, (void *)end);
	}
#endif
}
