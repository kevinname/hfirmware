#ifndef __CACHE_H__
#define __CACHE_H__

extern void nds32_dcache_invalidate(void);
extern void nds32_dcache_flush(void);
extern void nds32_dcache_clean(void);
extern void nds32_icache_flush(void);
extern void nds32_dcache_clean_range(unsigned long start, unsigned long end);
extern void nds32_dma_clean_range(unsigned long start, unsigned long end);
extern void nds32_dcache_invalidate_range(unsigned long start, unsigned long end);
extern void nds32_dcache_flush_range(unsigned long start, unsigned long end);
extern void nds32_dcache_writeback_range(unsigned long start, unsigned long end);
extern void nds32_dma_inv_range(unsigned long start, unsigned long end);
extern void nds32_dma_flush_range(unsigned long start, unsigned long end);
extern void nds32_icache_invalidate_range(unsigned long start, unsigned long end);

#endif /* __CACHE_H__ */
