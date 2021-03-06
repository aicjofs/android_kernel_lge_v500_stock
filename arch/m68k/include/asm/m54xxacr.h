/*
                                                          
 */

#ifndef	m54xxacr_h
#define m54xxacr_h

/*
                                   
 */
#define CACR_DEC	0x80000000	/*                   */
#define CACR_DWP	0x40000000	/*                       */
#define CACR_DESB	0x20000000	/*                          */
#define CACR_DDPI	0x10000000	/*                                */
#define CACR_DHCLK	0x08000000	/*                           */
#define CACR_DDCM_WT	0x00000000	/*                    */
#define CACR_DDCM_CP	0x02000000	/*                */
#define CACR_DDCM_P	0x04000000	/*                   */
#define CACR_DDCM_IMP	0x06000000	/*                     */
#define CACR_DCINVA	0x01000000	/*                       */
#define CACR_BEC	0x00080000	/*                     */
#define CACR_BCINVA	0x00040000	/*                         */
#define CACR_IEC	0x00008000	/*                          */
#define CACR_DNFB	0x00002000	/*                       */
#define CACR_IDPI	0x00001000	/*                */
#define CACR_IHLCK	0x00000800	/*                            */
#define CACR_IDCM	0x00000400	/*                          */
#define CACR_ICINVA	0x00000100	/*                        */
#define CACR_EUSP	0x00000020	/*                         */

#define ACR_BASE_POS	24		/*              */
#define ACR_MASK_POS	16		/*              */
#define ACR_ENABLE	0x00008000	/*                */
#define ACR_USER	0x00000000	/*                       */
#define ACR_SUPER	0x00002000	/*                      */
#define ACR_ANY		0x00004000	/*                       */
#define ACR_CM_WT	0x00000000	/*                    */
#define ACR_CM_CP	0x00000020	/*               */
#define ACR_CM_OFF_PRE	0x00000040	/*                   */
#define ACR_CM_OFF_IMP	0x00000060	/*                     */
#define ACR_CM		0x00000060	/*                 */
#define ACR_SP		0x00000008	/*                    */
#define ACR_WPROTECT	0x00000004	/*               */

#define ACR_BA(x)	((x) & 0xff000000)
#define ACR_ADMSK(x)	((((x) - 1) & 0xff000000) >> 8)

#if defined(CONFIG_M5407)

#define ICACHE_SIZE 0x4000	/*                   */
#define DCACHE_SIZE 0x2000	/*           */

#elif defined(CONFIG_M54xx)

#define ICACHE_SIZE 0x8000	/*                   */
#define DCACHE_SIZE 0x8000	/*            */

#endif

#define CACHE_LINE_SIZE 0x0010	/*          */
#define CACHE_WAYS 4		/*        */

#define ICACHE_SET_MASK	((ICACHE_SIZE / 64 - 1) << CACHE_WAYS)
#define DCACHE_SET_MASK	((DCACHE_SIZE / 64 - 1) << CACHE_WAYS)
#define ICACHE_MAX_ADDR	ICACHE_SET_MASK
#define DCACHE_MAX_ADDR	DCACHE_SET_MASK

/*
                                                                 
                                                                        
                                  
 */
/*                                                                           */
/*                                             */

	/*                   */
	/*                          */
	/*                                  */
	/*                                  */
#if defined(CONFIG_M5407)
#define CACHE_MODE (CACR_DEC+CACR_DESB+CACR_DDCM_P+CACR_BEC+CACR_IEC)
#else
#define CACHE_MODE (CACR_DEC+CACR_DESB+CACR_DDCM_P+CACR_BEC+CACR_IEC+CACR_EUSP)
#endif
#define CACHE_INIT (CACR_DCINVA+CACR_BCINVA+CACR_ICINVA)

#if defined(CONFIG_MMU)
/*
                                                                   
                                                                   
                                        
 */
#define ACR0_MODE	(ACR_BA(CONFIG_MBAR)+ACR_ADMSK(0x1000000)+ \
			 ACR_ENABLE+ACR_SUPER+ACR_CM_OFF_PRE+ACR_SP)
#define ACR1_MODE	(ACR_BA(CONFIG_RAMBASE)+ACR_ADMSK(CONFIG_RAMSIZE)+ \
			 ACR_ENABLE+ACR_SUPER+ACR_SP)
#define ACR2_MODE	0
#define ACR3_MODE	(ACR_BA(CONFIG_RAMBASE)+ACR_ADMSK(CONFIG_RAMSIZE)+ \
			 ACR_ENABLE+ACR_SUPER+ACR_SP)

#else

/*
                                                               
 */
#if defined(CONFIG_CACHE_COPYBACK)
#define DATA_CACHE_MODE (ACR_ENABLE+ACR_ANY+ACR_CM_CP)
#else
#define DATA_CACHE_MODE (ACR_ENABLE+ACR_ANY+ACR_CM_WT)
#endif
#define INSN_CACHE_MODE (ACR_ENABLE+ACR_ANY)

#define CACHE_INVALIDATE  (CACHE_MODE+CACR_DCINVA+CACR_BCINVA+CACR_ICINVA)
#define CACHE_INVALIDATEI (CACHE_MODE+CACR_BCINVA+CACR_ICINVA)
#define CACHE_INVALIDATED (CACHE_MODE+CACR_DCINVA)
#define ACR0_MODE	(0x000f0000+DATA_CACHE_MODE)
#define ACR1_MODE	0
#define ACR2_MODE	(0x000f0000+INSN_CACHE_MODE)
#define ACR3_MODE	0

#if ((DATA_CACHE_MODE & ACR_CM) == ACR_CM_CP)
/*                                                       */
#define	CACHE_PUSH
#endif

#endif /*            */
#endif	/*            */
