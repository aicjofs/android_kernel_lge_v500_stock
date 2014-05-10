/*
                                                                 
 */

/*                    */
/*                                                                   
                                                                            
                              
 */
/*
                                    
 */
#define UMAL_CFG1		(PKUNITY_UMAL_BASE + 0x0000)
/*
                                       
 */
#define UMAL_CFG2		(PKUNITY_UMAL_BASE + 0x0004)
/*
                                     
 */
#define UMAL_IPGIFG		(PKUNITY_UMAL_BASE + 0x0008)
/*
                                             
 */
#define UMAL_HALFDUPLEX		(PKUNITY_UMAL_BASE + 0x000c)
/*
                                     
 */
#define UMAL_MAXFRAME		(PKUNITY_UMAL_BASE + 0x0010)
/*
                             
 */
#define UMAL_TESTREG		(PKUNITY_UMAL_BASE + 0x001c)
/*
                                       
 */
#define UMAL_MIICFG		(PKUNITY_UMAL_BASE + 0x0020)
/*
                                     
 */
#define UMAL_MIICMD		(PKUNITY_UMAL_BASE + 0x0024)
/*
                                      
 */
#define UMAL_MIIADDR		(PKUNITY_UMAL_BASE + 0x0028)
/*
                                      
 */
#define UMAL_MIICTRL		(PKUNITY_UMAL_BASE + 0x002c)
/*
                                       
 */
#define UMAL_MIISTATUS		(PKUNITY_UMAL_BASE + 0x0030)
/*
                                        
 */
#define UMAL_MIIIDCT		(PKUNITY_UMAL_BASE + 0x0034)
/*
                                
 */
#define UMAL_IFCTRL		(PKUNITY_UMAL_BASE + 0x0038)
/*
                                 
 */
#define UMAL_IFSTATUS		(PKUNITY_UMAL_BASE + 0x003c)
/*
                                          
 */
#define UMAL_STADDR1		(PKUNITY_UMAL_BASE + 0x0040)
/*
                                         
 */
#define UMAL_STADDR2		(PKUNITY_UMAL_BASE + 0x0044)

/*                     */
/*                                                                    
             
 */
#define UMAL_FIFOCFG0		(PKUNITY_UMAL_BASE + 0x0048)
#define UMAL_FIFOCFG1		(PKUNITY_UMAL_BASE + 0x004c)
#define UMAL_FIFOCFG2		(PKUNITY_UMAL_BASE + 0x0050)
#define UMAL_FIFOCFG3		(PKUNITY_UMAL_BASE + 0x0054)
#define UMAL_FIFOCFG4		(PKUNITY_UMAL_BASE + 0x0058)
#define UMAL_FIFOCFG5		(PKUNITY_UMAL_BASE + 0x005c)
#define UMAL_FIFORAM0		(PKUNITY_UMAL_BASE + 0x0060)
#define UMAL_FIFORAM1		(PKUNITY_UMAL_BASE + 0x0064)
#define UMAL_FIFORAM2		(PKUNITY_UMAL_BASE + 0x0068)
#define UMAL_FIFORAM3		(PKUNITY_UMAL_BASE + 0x006c)
#define UMAL_FIFORAM4		(PKUNITY_UMAL_BASE + 0x0070)
#define UMAL_FIFORAM5		(PKUNITY_UMAL_BASE + 0x0074)
#define UMAL_FIFORAM6		(PKUNITY_UMAL_BASE + 0x0078)
#define UMAL_FIFORAM7		(PKUNITY_UMAL_BASE + 0x007c)

/*                      */
/*                                                                            
                                                                         
                                          
 */
/*
                                  
 */
#define UMAL_DMATxCtrl		(PKUNITY_UMAL_BASE + 0x0180)
/*
                                                
 */
#define UMAL_DMATxDescriptor	(PKUNITY_UMAL_BASE + 0x0184)
/*
                                                 
 */
#define UMAL_DMATxStatus	(PKUNITY_UMAL_BASE + 0x0188)
/*
                                 
 */
#define UMAL_DMARxCtrl		(PKUNITY_UMAL_BASE + 0x018c)
/*
                                                
 */
#define UMAL_DMARxDescriptor	(PKUNITY_UMAL_BASE + 0x0190)
/*
                                                 
 */
#define UMAL_DMARxStatus	(PKUNITY_UMAL_BASE + 0x0194)
/*
                                  
 */
#define UMAL_DMAIntrMask	(PKUNITY_UMAL_BASE + 0x0198)
/*
                                          
 */
#define UMAL_DMAInterrupt	(PKUNITY_UMAL_BASE + 0x019c)

/*
                                  
 */
#define UMAL_CFG1_TXENABLE	FIELD(1, 1, 0)
#define UMAL_CFG1_RXENABLE	FIELD(1, 1, 2)
#define UMAL_CFG1_TXFLOWCTL	FIELD(1, 1, 4)
#define UMAL_CFG1_RXFLOWCTL	FIELD(1, 1, 5)
#define UMAL_CFG1_CONFLPBK	FIELD(1, 1, 8)
#define UMAL_CFG1_RESET		FIELD(1, 1, 31)
#define UMAL_CFG1_CONFFLCTL	(MAC_TX_FLOW_CTL | MAC_RX_FLOW_CTL)

/*
                                  
 */
#define UMAL_CFG2_FULLDUPLEX	FIELD(1, 1, 0)
#define UMAL_CFG2_CRCENABLE	FIELD(1, 1, 1)
#define UMAL_CFG2_PADCRC	FIELD(1, 1, 2)
#define UMAL_CFG2_LENGTHCHECK	FIELD(1, 1, 4)
#define UMAL_CFG2_MODEMASK	FMASK(2, 8)
#define UMAL_CFG2_NIBBLEMODE	FIELD(1, 2, 8)
#define UMAL_CFG2_BYTEMODE	FIELD(2, 2, 8)
#define UMAL_CFG2_PREAMBLENMASK	FMASK(4, 12)
#define UMAL_CFG2_DEFPREAMBLEN	FIELD(7, 4, 12)
#define UMAL_CFG2_FD100		(UMAL_CFG2_DEFPREAMBLEN | UMAL_CFG2_NIBBLEMODE \
				| UMAL_CFG2_LENGTHCHECK | UMAL_CFG2_PADCRC \
				| UMAL_CFG2_CRCENABLE | UMAL_CFG2_FULLDUPLEX)
#define UMAL_CFG2_FD1000	(UMAL_CFG2_DEFPREAMBLEN | UMAL_CFG2_BYTEMODE \
				| UMAL_CFG2_LENGTHCHECK | UMAL_CFG2_PADCRC \
				| UMAL_CFG2_CRCENABLE | UMAL_CFG2_FULLDUPLEX)
#define UMAL_CFG2_HD100		(UMAL_CFG2_DEFPREAMBLEN | UMAL_CFG2_NIBBLEMODE \
				| UMAL_CFG2_LENGTHCHECK | UMAL_CFG2_PADCRC \
				| UMAL_CFG2_CRCENABLE)

/*
                                   
 */
#define UMAL_IFCTRL_RESET	FIELD(1, 1, 31)

/*
                                   
 */
#define UMAL_MIICFG_RESET	FIELD(1, 1, 31)

/*
                                   
 */
#define UMAL_MIICMD_READ	FIELD(1, 1, 0)

/*
                                    
 */
#define UMAL_MIIIDCT_BUSY	FIELD(1, 1, 0)
#define UMAL_MIIIDCT_NOTVALID	FIELD(1, 1, 2)

/*
                                   
 */
#define UMAL_DMA_Enable		FIELD(1, 1, 0)

/*
                                   
 */
#define UMAL_DMAIntrMask_ENABLEHALFWORD	FIELD(1, 1, 16)

/*
                          
 */
#define CLR_RX_BUS_ERR		FIELD(1, 1, 3)
#define CLR_RX_OVERFLOW		FIELD(1, 1, 2)
#define CLR_RX_PKT		FIELD(1, 1, 0)

/*
                          
 */
#define CLR_TX_BUS_ERR		FIELD(1, 1, 3)
#define CLR_TX_UNDERRUN		FIELD(1, 1, 1)
#define CLR_TX_PKT		FIELD(1, 1, 0)

/*
                                                     
 */
#define INT_RX_MASK		FIELD(0xd, 4, 4)
#define INT_TX_MASK		FIELD(0xb, 4, 0)

#define INT_RX_BUS_ERR		FIELD(1, 1, 7)
#define INT_RX_OVERFLOW		FIELD(1, 1, 6)
#define INT_RX_PKT		FIELD(1, 1, 4)
#define INT_TX_BUS_ERR		FIELD(1, 1, 3)
#define INT_TX_UNDERRUN		FIELD(1, 1, 1)
#define INT_TX_PKT		FIELD(1, 1, 0)

/*
                               
 */
#define UMAL_DESC_PACKETSIZE_EMPTY	FIELD(1, 1, 31)
#define UMAL_DESC_PACKETSIZE_NONEMPTY	FIELD(0, 1, 31)
#define UMAL_DESC_PACKETSIZE_SIZEMASK	FMASK(12, 0)

