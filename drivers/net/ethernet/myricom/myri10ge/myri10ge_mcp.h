#ifndef __MYRI10GE_MCP_H__
#define __MYRI10GE_MCP_H__

#define MXGEFW_VERSION_MAJOR	1
#define MXGEFW_VERSION_MINOR	4

/*         */
struct mcp_dma_addr {
	__be32 high;
	__be32 low;
};

/*         */
struct mcp_slot {
	__sum16 checksum;
	__be16 length;
};

/*          */
struct mcp_cmd {
	__be32 cmd;
	__be32 data0;		/*                                       */
	/*   */
	__be32 data1;		/*                                        */
	__be32 data2;		/*                    */
	/*    */
	struct mcp_dma_addr response_addr;
	/*    */
	u8 pad[40];
};

/*         */
struct mcp_cmd_response {
	__be32 data;
	__be32 result;
};

/*
                                       
  
                                                                   
                                                      
  
                                              
  
                                                                   
                                
  
                                                            
                                           
 */

#define MXGEFW_FLAGS_SMALL      0x1
#define MXGEFW_FLAGS_TSO_HDR    0x1
#define MXGEFW_FLAGS_FIRST      0x2
#define MXGEFW_FLAGS_ALIGN_ODD  0x4
#define MXGEFW_FLAGS_CKSUM      0x8
#define MXGEFW_FLAGS_TSO_LAST   0x8
#define MXGEFW_FLAGS_NO_TSO     0x10
#define MXGEFW_FLAGS_TSO_CHOP   0x10
#define MXGEFW_FLAGS_TSO_PLD    0x20

#define MXGEFW_SEND_SMALL_SIZE  1520
#define MXGEFW_MAX_MTU          9400

union mcp_pso_or_cumlen {
	u16 pseudo_hdr_offset;
	u16 cum_len;
};

#define	MXGEFW_MAX_SEND_DESC 12
#define MXGEFW_PAD	    2

/*          */
struct mcp_kreq_ether_send {
	__be32 addr_high;
	__be32 addr_low;
	__be16 pseudo_hdr_offset;
	__be16 length;
	u8 pad;
	u8 rdma_count;
	u8 cksum_offset;	/*                                */
	u8 flags;		/*                  */
};

/*         */
struct mcp_kreq_ether_recv {
	__be32 addr_high;
	__be32 addr_low;
};

/*          */

#define	MXGEFW_BOOT_HANDOFF	0xfc0000
#define	MXGEFW_BOOT_DUMMY_RDMA	0xfc01c0

#define	MXGEFW_ETH_CMD		0xf80000
#define	MXGEFW_ETH_SEND_4	0x200000
#define	MXGEFW_ETH_SEND_1	0x240000
#define	MXGEFW_ETH_SEND_2	0x280000
#define	MXGEFW_ETH_SEND_3	0x2c0000
#define	MXGEFW_ETH_RECV_SMALL	0x300000
#define	MXGEFW_ETH_RECV_BIG	0x340000
#define	MXGEFW_ETH_SEND_GO	0x380000
#define	MXGEFW_ETH_SEND_STOP	0x3C0000

#define	MXGEFW_ETH_SEND(n)		(0x200000 + (((n) & 0x03) * 0x40000))
#define	MXGEFW_ETH_SEND_OFFSET(n)	(MXGEFW_ETH_SEND(n) - MXGEFW_ETH_SEND_4)

enum myri10ge_mcp_cmd_type {
	MXGEFW_CMD_NONE = 0,
	/*                                                   
                                             */
	MXGEFW_CMD_RESET = 1,

	/*                                                 
                                               */
	MXGEFW_GET_MCP_VERSION = 2,

	/*                                                         
                                                             
                               */

	MXGEFW_CMD_SET_INTRQ_DMA = 3,
	/*                                
                                   
                                                    
  */

	MXGEFW_CMD_SET_BIG_BUFFER_SIZE = 4,	/*                      */
	MXGEFW_CMD_SET_SMALL_BUFFER_SIZE = 5,	/*          */

	/*                                                         
                                                    */

	MXGEFW_CMD_GET_SEND_OFFSET = 6,
	MXGEFW_CMD_GET_SMALL_RX_OFFSET = 7,
	MXGEFW_CMD_GET_BIG_RX_OFFSET = 8,
	/*                                                  */

	MXGEFW_CMD_GET_IRQ_ACK_OFFSET = 9,
	MXGEFW_CMD_GET_IRQ_DEASSERT_OFFSET = 10,

	/*                                                   
                                            */

	MXGEFW_CMD_GET_SEND_RING_SIZE = 11,	/*          */
	MXGEFW_CMD_GET_RX_RING_SIZE = 12,	/*          */

	/*                                                    
                                                        
                                                      
                                     */

	MXGEFW_CMD_SET_INTRQ_SIZE = 13,	/*          */
#define MXGEFW_CMD_SET_INTRQ_SIZE_FLAG_NO_STRICT_SIZE_CHECK  (1 << 31)

	/*                                                          
                                                           
                             */
	MXGEFW_CMD_ETHERNET_UP = 14,

	/*                                                            
                                                                
                                                             
                            */

	MXGEFW_CMD_ETHERNET_DOWN = 15,

	/*                                                      
                                                        
                                                           
                                                         
                         */

	MXGEFW_CMD_SET_MTU = 16,
	MXGEFW_CMD_GET_INTR_COAL_DELAY_OFFSET = 17,	/*                 */
	MXGEFW_CMD_SET_STATS_INTERVAL = 18,	/*                 */
	MXGEFW_CMD_SET_STATS_DMA_OBSOLETE = 19,	/*                              */

	MXGEFW_ENABLE_PROMISC = 20,
	MXGEFW_DISABLE_PROMISC = 21,
	MXGEFW_SET_MAC_ADDRESS = 22,

	MXGEFW_ENABLE_FLOW_CONTROL = 23,
	MXGEFW_DISABLE_FLOW_CONTROL = 24,

	/*              
                             
                                                      
                                                               
  */
	MXGEFW_DMA_TEST = 25,

	MXGEFW_ENABLE_ALLMULTI = 26,
	MXGEFW_DISABLE_ALLMULTI = 27,

	/*                                   
                                    
                                               */
	MXGEFW_JOIN_MULTICAST_GROUP = 28,
	/*                                   
                                       
                                    
                                               */
	MXGEFW_LEAVE_MULTICAST_GROUP = 29,
	MXGEFW_LEAVE_ALL_MULTICAST_GROUPS = 30,

	MXGEFW_CMD_SET_STATS_DMA_V2 = 31,
	/*                         
                                                                         
                                                             
   
                                                                    
                                                         
                           
  */

	MXGEFW_CMD_UNALIGNED_TEST = 32,
	/*                                                                     
            */

	MXGEFW_CMD_UNALIGNED_STATUS = 33,
	/*                                                                     */

	MXGEFW_CMD_ALWAYS_USE_N_BIG_BUFFERS = 34,
	/*                                                                     
                                                                          
                                             
                                                                        
                                                  
                                                                      
                                                        
  */

	MXGEFW_CMD_GET_MAX_RSS_QUEUES = 35,
	MXGEFW_CMD_ENABLE_RSS_QUEUES = 36,
	/*                                                      
                                                             
                         
                              
                                                               
                                                    
                                          
                                                                    
                                                       
  */
#define MXGEFW_SLICE_INTR_MODE_SHARED          0x0
#define MXGEFW_SLICE_INTR_MODE_ONE_PER_SLICE   0x1
#define MXGEFW_SLICE_ENABLE_MULTIPLE_TX_QUEUES 0x2

	MXGEFW_CMD_GET_RSS_SHARED_INTERRUPT_MASK_OFFSET = 37,
	MXGEFW_CMD_SET_RSS_SHARED_INTERRUPT_DMA = 38,
	/*                                     */
	MXGEFW_CMD_GET_RSS_TABLE_OFFSET = 39,
	/*                                         */
	MXGEFW_CMD_SET_RSS_TABLE_SIZE = 40,
	/*                                       */
	MXGEFW_CMD_GET_RSS_KEY_OFFSET = 41,
	/*                                  */
	MXGEFW_CMD_RSS_KEY_UPDATED = 42,
	/*                                             */
	MXGEFW_CMD_SET_RSS_ENABLE = 43,
	/*                           
                                                             
                                                                 
                     
                                        
                                        
                                        
                  
                                     
  */
#define MXGEFW_RSS_HASH_TYPE_IPV4      0x1
#define MXGEFW_RSS_HASH_TYPE_TCP_IPV4  0x2
#define MXGEFW_RSS_HASH_TYPE_SRC_PORT  0x4
#define MXGEFW_RSS_HASH_TYPE_SRC_DST_PORT 0x5
#define MXGEFW_RSS_HASH_TYPE_MAX 0x5

	MXGEFW_CMD_GET_MAX_TSO6_HDR_SIZE = 44,
	/*                                                                        
                                                                        
                                            
                                                   
                                                                       
                                                                   
  */

	MXGEFW_CMD_SET_TSO_MODE = 45,
	/* data0 = TSO mode.
	 * 0: Linux/FreeBSD style (NIC default)
	 * 1: NDIS/NetBSD style
	 */
#define MXGEFW_TSO_MODE_LINUX  0
#define MXGEFW_TSO_MODE_NDIS   1

	MXGEFW_CMD_MDIO_READ = 46,
	/*                                                              */
	MXGEFW_CMD_MDIO_WRITE = 47,
	/*                                                          */

	MXGEFW_CMD_I2C_READ = 48,
	/*                                                                       
                                                      
                                                   
                                                 
                                            
                                                                           
                                                                       
                                    
                                                                                          
                                                                                  
                                      
  */
	MXGEFW_CMD_I2C_BYTE = 49,
	/*                                                                   
                                                              
                                              
                                                                      
  */

	MXGEFW_CMD_GET_VPUMP_OFFSET = 50,
	/*                                                            */
	MXGEFW_CMD_RESET_VPUMP = 51,
	/*                        */

	MXGEFW_CMD_SET_RSS_MCP_SLOT_TYPE = 52,
	/*                              
                               
                     
  */
#define MXGEFW_RSS_MCP_SLOT_TYPE_MIN        0
#define MXGEFW_RSS_MCP_SLOT_TYPE_WITH_HASH  1

	MXGEFW_CMD_SET_THROTTLE_FACTOR = 53,
	/*                                     
                           
                                                     
                                                     
   
                                          
                                         
   
                                                                           
                                                                      
   
                                                                                 
                                                                               
  */

	MXGEFW_CMD_VPUMP_UP = 54,
	/*                                                                              */
	MXGEFW_CMD_GET_VPUMP_CLK = 55,
	/*                     */

	MXGEFW_CMD_GET_DCA_OFFSET = 56,
	/*                                 */

	/*                          */
	MXGEFW_CMD_NETQ_GET_FILTERS_PER_QUEUE = 57,
	MXGEFW_CMD_NETQ_ADD_FILTER = 58,
	/*                                             */
	/*                         */
	/*                                  */
	MXGEFW_CMD_NETQ_DEL_FILTER = 59,
	/*                   */
	MXGEFW_CMD_NETQ_QUERY1 = 60,
	MXGEFW_CMD_NETQ_QUERY2 = 61,
	MXGEFW_CMD_NETQ_QUERY3 = 62,
	MXGEFW_CMD_NETQ_QUERY4 = 63,

	MXGEFW_CMD_RELAX_RXBUFFER_ALIGNMENT = 64,
	/*                                                           
                                                                
                                                                  
  */
};

enum myri10ge_mcp_cmd_status {
	MXGEFW_CMD_OK = 0,
	MXGEFW_CMD_UNKNOWN = 1,
	MXGEFW_CMD_ERROR_RANGE = 2,
	MXGEFW_CMD_ERROR_BUSY = 3,
	MXGEFW_CMD_ERROR_EMPTY = 4,
	MXGEFW_CMD_ERROR_CLOSED = 5,
	MXGEFW_CMD_ERROR_HASH_ERROR = 6,
	MXGEFW_CMD_ERROR_BAD_PORT = 7,
	MXGEFW_CMD_ERROR_RESOURCES = 8,
	MXGEFW_CMD_ERROR_MULTICAST = 9,
	MXGEFW_CMD_ERROR_UNALIGNED = 10,
	MXGEFW_CMD_ERROR_NO_MDIO = 11,
	MXGEFW_CMD_ERROR_I2C_FAILURE = 12,
	MXGEFW_CMD_ERROR_I2C_ABSENT = 13,
	MXGEFW_CMD_ERROR_BAD_PCIE_LINK = 14
};

#define MXGEFW_OLD_IRQ_DATA_LEN 40

struct mcp_irq_data {
	/*                                   */
	__be32 future_use[1];
	__be32 dropped_pause;
	__be32 dropped_unicast_filtered;
	__be32 dropped_bad_crc32;
	__be32 dropped_bad_phy;
	__be32 dropped_multicast_filtered;
	/*          */
	__be32 send_done_count;

#define MXGEFW_LINK_DOWN 0
#define MXGEFW_LINK_UP 1
#define MXGEFW_LINK_MYRINET 2
#define MXGEFW_LINK_UNKNOWN 3
	__be32 link_up;
	__be32 dropped_link_overflow;
	__be32 dropped_link_error_or_filtered;
	__be32 dropped_runt;
	__be32 dropped_overrun;
	__be32 dropped_no_small_buffer;
	__be32 dropped_no_big_buffer;
	__be32 rdma_tags_available;

	u8 tx_stopped;
	u8 link_down;
	u8 stats_updated;
	u8 valid;
};

/*                                  */
#define MXGEFW_NETQ_FILTERTYPE_NONE 0
#define MXGEFW_NETQ_FILTERTYPE_MACADDR 1
#define MXGEFW_NETQ_FILTERTYPE_VLAN 2
#define MXGEFW_NETQ_FILTERTYPE_VLANMACADDR 3

#endif				/*                    */
