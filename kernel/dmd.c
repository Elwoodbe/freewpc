
#include <freewpc.h>

#if (MACHINE_DMD == 1)

dmd_pagenum_t dmd_free_page, 
	dmd_low_page, 
	dmd_high_page, 
	dmd_visible_page;


void dmd_init (void)
{
	wpc_dmd_firq_row = 0xFF;
	dmd_low_page = wpc_dmd_low_page = 0;
	dmd_high_page = wpc_dmd_high_page = 0;
	dmd_visible_page = wpc_dmd_visible_page = 0;
	dmd_free_page = 1;
}


void dmd_rtt (void)
{
}


static dmd_pagenum_t dmd_alloc (void)
{
	dmd_pagenum_t page = dmd_free_page;
	dmd_free_page += 1;
	/* TODO - compiler is doing 16-bit here.  Need rules for
	 * treating values like DMD_PAGE_COUNT that are less than
	 * 255 as byte values */
	dmd_free_page %= DMD_PAGE_COUNT;
	return page;
}

void dmd_alloc_low (void)
{
	dmd_low_page = wpc_dmd_low_page = dmd_alloc ();
}

void dmd_alloc_high (void)
{
	dmd_high_page = wpc_dmd_high_page = dmd_alloc ();
}

void dmd_alloc_low_high (void)
{
	dmd_alloc_low ();
	dmd_alloc_high ();
}

void dmd_show_low (void)
{
	dmd_visible_page = wpc_dmd_visible_page = dmd_low_page;
}

void dmd_show_high (void)
{
	dmd_visible_page = wpc_dmd_visible_page = dmd_high_page;
}

void dmd_flip_low_high (void)
{
	dmd_pagenum_t tmp = dmd_low_page;
	dmd_low_page = dmd_high_page;
	dmd_high_page = tmp;
	/* TODO - shouldn't this always rewrite the I/O registers */
}


void dmd_show_other (void)
{
	if (dmd_visible_page == dmd_low_page)
		dmd_show_high ();
	else
		dmd_show_low ();
}


void dmd_swap_low_high (void)
{
	__lda (dmd_high_page);
	__ldb (dmd_low_page);
	__asm__ volatile ("exg a,b");
	__sta (&dmd_high_page);
	__stb (&dmd_low_page);
}


void dmd_clean_page (dmd_buffer_t *dbuf)
{
	register int8_t count asm ("d") = DMD_PAGE_SIZE / (2 * 4);
	register uint16_t *dbuf16 = (uint16_t *)dbuf;
	register uint16_t zero = 0;
	while (--count >= 0)
	{
		*dbuf16++ = zero;
		*dbuf16++ = zero;
		*dbuf16++ = zero;
		*dbuf16++ = zero;
	}
}


void dmd_clean_page_low (void)
{
	dmd_clean_page (dmd_low_buffer);
}


void dmd_clean_page_high (void)
{
	dmd_clean_page (dmd_high_buffer);
}


void dmd_invert_page (dmd_buffer_t *dbuf)
{
	register int16_t count asm ("u") = DMD_PAGE_SIZE / (2 * 4);
	register uint16_t *dbuf16 = (uint16_t *)dbuf;
	while (--count >= 0)
	{
		*dbuf16 = ~*dbuf16;
		dbuf16++;
		*dbuf16 = ~*dbuf16;
		dbuf16++;
		*dbuf16 = ~*dbuf16;
		dbuf16++;
		*dbuf16 = ~*dbuf16;
		dbuf16++;
	}
}


static inline void dmd_copy_page (dmd_buffer_t *dst, dmd_buffer_t *src)
{
	register int8_t count asm ("d") = DMD_PAGE_SIZE / (2 * 4);
	register uint16_t *dst16 = (uint16_t *)dst;
	register uint16_t *src16 = (uint16_t *)src;
	while (--count >= 0)
	{
		*dst16++ = *src16++;
		*dst16++ = *src16++;
		*dst16++ = *src16++;
		*dst16++ = *src16++;
	}
}

void dmd_copy_low_to_high (void)
{
	dmd_copy_page (dmd_high_buffer, dmd_low_buffer);
}

void dmd_alloc_low_clean (void)
{
	dmd_alloc_low ();
	dmd_clean_page (dmd_low_buffer);
}

void dmd_alloc_high_clean (void)
{
	dmd_alloc_high ();
	dmd_clean_page (dmd_high_buffer);
}

void dmd_draw_border (char *dbuf)
{
	dmd_buffer_t *dbuf_bot = (dmd_buffer_t *)((char *)dbuf + 480);
	uint16_t i;
	for (i=0; i < 16; i++)
		*((uint16_t *)dbuf_bot)++ = *((uint16_t *)dbuf)++ = 0xFFFF;
	for (i=0; i < 28; i++)
	{
		dbuf[0] = 0x03;
		dbuf[15] = 0xC0;
		dbuf += 16;
	}
}

void dmd_shift_up (dmd_buffer_t *dbuf)
{
	uint16_t i;
	for (i=(31 * 16 / 2); i != 0; --i)
	{
		dbuf[0] = dbuf[8];
		dbuf++;
	}

	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
	*dbuf++ = 0;
}

#endif /* MACHINE_DMD */

