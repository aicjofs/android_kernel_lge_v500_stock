/*                                                                           
        
               
  
               

     
                
                            
                                                                                
                                              
  
                                                                                            
*/


#ifndef __LINUX_USB_DDM_BRIDGE_H__
#define __LINUX_USB_DDM_BRIDGE_H__

//                            
#define LG_FW_DDM_PM
#define LG_FW_REMOTE_WAKEUP

#define DDM_LOG_FORMAT_LEN_FILE_NAME	(50)
#define DDM_LOG_FORMAT_LEN_LINE			(4)
#define DDM_LOG_FORMAT_LEN_MSG		(80)
#define DDM_LOG_FORMAT_LEN_SW_VER		(50)
#define HS_INTERVAL 7
#define FS_LS_INTERVAL 3

struct ddm_bridge_ops {
	void *ctxt;
	int tmp; //     
	void (*read_complete_cb)(void *ctxt,void *buf, 
			int buf_size, int actual);
	void (*write_complete_cb)(void *ctxt, void *buf,
			int buf_size, int actual);
	int (*suspend)(void *ctxt);
	void (*resume)(void *ctxt);
};

struct ddm_dev {
	char *read_buf;
	char *write_buf;
	struct work_struct read_w;
	struct work_struct write_w;
	struct workqueue_struct *ddm_wq;
#ifdef LG_FW_WAIT_QUEUE	
	wait_queue_head_t	read_wait_queue;
#endif
	unsigned long	flags;

	int hsic_device_enabled;
	int hsic_suspend;
/*                                              */
	int in_busy_hsic_write;
	int in_busy_hsic_read;

	u8 loopback_mode;

	struct ddm_bridge_ops	ops;
};

extern struct ddm_dev *ddm_drv;

/*                                 */

typedef struct
{
	u16 mode; //                                             
	u16 total_size;
	u32 arch_type;
} sddm_RX_Header;

typedef struct
{
	sddm_RX_Header header;
	char filename[DDM_LOG_FORMAT_LEN_FILE_NAME];
	int line;
	char msg[DDM_LOG_FORMAT_LEN_MSG];
} sddm_RX_Data;
typedef struct 
{
	bool				valid;		//                     
	sddm_RX_Data		rx_data;	//                               
} sddm_PRx_Data;
/*                               */

typedef struct
{
	u16 mode; //                                             
	u16 total_size;
	u32 arch_type;
} sddm_TX_Header;

typedef struct
{
	sddm_RX_Header header;
	char filename[DDM_LOG_FORMAT_LEN_FILE_NAME];
	int line;
	char msg[DDM_LOG_FORMAT_LEN_MSG];
} sddm_TX_Data;

typedef struct 
{
	bool				valid;		//                     
	sddm_TX_Data		tx_data;	//                               
} sddm_PTx_Data;


extern int ddm_bridge_read(char *data, int size);
extern int ddm_bridge_write(char *data, int size);
extern int ddm_bridge_open(struct ddm_bridge_ops *ops);
extern void ddm_bridge_close(void);

#endif

