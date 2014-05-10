/*                                                      */
 
/*                                                                               */
 

#ifndef DRIVER_ATM_IDT77105_H
#define DRIVER_ATM_IDT77105_H

#include <linux/atmdev.h>
#include <linux/atmioc.h>


/*                    */

#define IDT77105_MCR		0x0	/*                         */
#define IDT77105_ISTAT	        0x1	/*                  */
#define IDT77105_DIAG   	0x2	/*                    */
#define IDT77105_LEDHEC		0x3	/*                                 */
#define IDT77105_CTRLO		0x4	/*                           */
#define IDT77105_CTRHI		0x5	/*                            */
#define IDT77105_CTRSEL		0x6	/*                              */

/*                          */

/*     */
#define IDT77105_MCR_UPLO	0x80	/*                                */
#define IDT77105_MCR_DREC	0x40	/*                                  */
#define IDT77105_MCR_ECEIO	0x20	/*                                  
                                                */
#define IDT77105_MCR_TDPC	0x10	/*                                 */
#define IDT77105_MCR_DRIC	0x08	/*                                  */
#define IDT77105_MCR_HALTTX	0x04	/*              */
#define IDT77105_MCR_UMODE	0x02	/*                              */
#define IDT77105_MCR_EIP	0x01	/*                           */

/*       */
#define IDT77105_ISTAT_GOODSIG	0x40	/*                    */
#define IDT77105_ISTAT_HECERR	0x20	/*                  */
#define IDT77105_ISTAT_SCR	0x10	/*                             */
#define IDT77105_ISTAT_TPE	0x08	/*                               */
#define IDT77105_ISTAT_RSCC	0x04	/*                                    */
#define IDT77105_ISTAT_RSE	0x02	/*                         */
#define IDT77105_ISTAT_RFO	0x01	/*                         */

/*      */
#define IDT77105_DIAG_FTD	0x80	/*                            */
#define IDT77105_DIAG_ROS	0x40	/*                              */
#define IDT77105_DIAG_MPCS	0x20	/*                                */
#define IDT77105_DIAG_RFLUSH	0x10	/*                         */
#define IDT77105_DIAG_ITPE	0x08	/*                              */
#define IDT77105_DIAG_ITHE	0x04	/*                          */
#define IDT77105_DIAG_UMODE	0x02	/*                              */
#define IDT77105_DIAG_LCMASK	0x03	/*                       */

#define IDT77105_DIAG_LC_NORMAL         0x00	/*                      */
#define IDT77105_DIAG_LC_PHY_LOOPBACK	0x02
#define IDT77105_DIAG_LC_LINE_LOOPBACK	0x03

/*        */
#define IDT77105_LEDHEC_DRHC	0x40	/*                           */
#define IDT77105_LEDHEC_DTHC	0x20	/*                                 */
#define IDT77105_LEDHEC_RPWMASK	0x18	/*                               */
#define IDT77105_LEDHEC_TFS	0x04	/*                             */
#define IDT77105_LEDHEC_TLS	0x02	/*                          */
#define IDT77105_LEDHEC_RLS	0x01	/*                          */

#define IDT77105_LEDHEC_RPW_1	0x00	/*                                */
#define IDT77105_LEDHEC_RPW_2	0x08	/*                                */
#define IDT77105_LEDHEC_RPW_4	0x10	/*                                */
#define IDT77105_LEDHEC_RPW_8	0x18	/*                                */

/*        */
#define IDT77105_CTRSEL_SEC	0x08	/*                         */
#define IDT77105_CTRSEL_TCC	0x04	/*                    */
#define IDT77105_CTRSEL_RCC	0x02	/*                    */
#define IDT77105_CTRSEL_RHEC	0x01	/*                         */

#ifdef __KERNEL__
int idt77105_init(struct atm_dev *dev);
#endif

/*
                     
 */
 
/*                                                                        */
#define IDT77105_STATS_TIMER_PERIOD     (HZ) 
/*                                                               */
#define IDT77105_RESTART_TIMER_PERIOD   (5 * HZ)

#endif
