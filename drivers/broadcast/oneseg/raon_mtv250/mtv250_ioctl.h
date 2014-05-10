#ifndef __MTV250_IOCTL_H__
#define __MTV250_IOCTL_H__


#ifdef __cplusplus 
extern "C"{ 
#endif  

//                                   
//                                        

#define RAONTV_DEV_NAME		"isdbt"
#define RAONTV_IOC_MAGIC	'R'


typedef enum
{	
	DMB_TV_MODE_TDMB   = 0,     //          
	DMB_TV_MODE_1SEG   = 1, //    
	DMB_TV_MODE_FM     = 2,       //   
} E_DMB_TV_MODE_TYPE;



/*                                                                              
                                 
                                                                              */
#define IOCTL_TEST_DMB_POWER_ON		_IO(RAONTV_IOC_MAGIC, 0)
#define IOCTL_TEST_DMB_POWER_OFF		_IO(RAONTV_IOC_MAGIC, 1)
#define IOCTL_TEST_TDMB_SET_FREQ		_IOW(RAONTV_IOC_MAGIC, 2, unsigned int)

typedef struct
{
	unsigned int page; //      
	unsigned int Addr; //      
	unsigned int data[100]; //       

} RTV_IOCTL_REGISTER_ACCESS;

#define IOCTL_REGISTER_READ     _IOWR(RAONTV_IOC_MAGIC, 3, RTV_IOCTL_REGISTER_ACCESS)
#define IOCTL_REGISTER_WRITE    _IOW(RAONTV_IOC_MAGIC, 4, RTV_IOCTL_REGISTER_ACCESS)

typedef struct
{
	unsigned int pin; //      
	unsigned int value; //      
} RTV_IOCTL_TEST_GPIO_INFO;
#define IOCTL_TEST_GPIO_SET	_IOW(RAONTV_IOC_MAGIC, 5, RTV_IOCTL_TEST_GPIO_INFO)
#define IOCTL_TEST_GPIO_GET	_IOWR(RAONTV_IOC_MAGIC, 6, RTV_IOCTL_TEST_GPIO_INFO)



/*                                                                              
                                    
                                                                              */
typedef struct
{
	unsigned int	ber; //       
	unsigned int	cnr;  //       
	unsigned int	per;  //       
	int 			rssi;  //       
} IOCTL_ISDBT_SIGNAL_INFO;

typedef struct
{
	unsigned short LOCK;                /*                                          */
	unsigned short CNo;                /*                          */
	unsigned int BER;                  /*                           */
	unsigned int PER;                  /*                             */
	unsigned short AGC;      /*                  */
	int RSSI;                  	/*                                                    */

//                                              
unsigned long start_ts_int_cnt;
unsigned long ovf_err_cnt;

} IOCTL_ISDBT_LGE_TUNER_INFO;


#define IOCTL_ISDBT_POWER_ON			_IO(RAONTV_IOC_MAGIC,10)
#define IOCTL_ISDBT_POWER_OFF			_IO(RAONTV_IOC_MAGIC, 11)
#define IOCTL_ISDBT_SCAN_FREQ			_IOW(RAONTV_IOC_MAGIC,12, unsigned int)
#define IOCTL_ISDBT_SET_FREQ			_IOW(RAONTV_IOC_MAGIC,13, unsigned int)
#define IOCTL_ISDBT_GET_LOCK_STATUS    	_IOR(RAONTV_IOC_MAGIC,14, unsigned int)
#define IOCTL_ISDBT_GET_TMCC			_IOR(RAONTV_IOC_MAGIC,15, RTV_ISDBT_TMCC_INFO)
#define IOCTL_ISDBT_GET_SIGNAL_INFO		_IOR(RAONTV_IOC_MAGIC,16, IOCTL_ISDBT_SIGNAL_INFO)
#define IOCTL_ISDBT_START_TS			_IO(RAONTV_IOC_MAGIC,17)
#define IOCTL_ISDBT_STOP_TS				_IO(RAONTV_IOC_MAGIC,18)
#define IOCTL_ISDBT_LGE_GET_TUNER_INFO	_IOR(RAONTV_IOC_MAGIC,19, IOCTL_ISDBT_LGE_TUNER_INFO)



/*                                                                              
                                    
                                                                              */ 
typedef struct
{
	unsigned int ch_freq_khz; //      
	unsigned int sub_ch_id;  //      
	E_RTV_TDMB_SERVICE_TYPE service_type;   //      
} IOCTL_TDMB_SUB_CH_INFO;

typedef struct
{
	unsigned int 	ber; //       
	unsigned int 	cer; //       
	unsigned int 	cnr;  //       
	unsigned int 	per;  //       
	int 			rssi;  //       
} IOCTL_TDMB_SIGNAL_INFO;


typedef struct
{
//                          
//                            

	unsigned int    msc_size[RTV_MAX_NUM_MULTI_SUB_CHANNEL];  
	unsigned int    msc_subch_id[RTV_MAX_NUM_MULTI_SUB_CHANNEL]; 
	unsigned char msc_buf[RTV_MAX_NUM_MULTI_SUB_CHANNEL][16 * 188]; 
} TDMB_CIF_TS_INFO;


#define IOCTL_TDMB_POWER_ON				_IOW(RAONTV_IOC_MAGIC, 30, E_RTV_COUNTRY_BAND_TYPE)
#define IOCTL_TDMB_POWER_OFF				_IO(RAONTV_IOC_MAGIC, 31)
#define IOCTL_TDMB_SCAN_FREQ				_IOW(RAONTV_IOC_MAGIC,32, unsigned int)
#define IOCTL_TDMB_SCAN_STOP				_IO(RAONTV_IOC_MAGIC,33)
#define IOCTL_TDMB_READ_FIC				_IOR(RAONTV_IOC_MAGIC,34, unsigned char)
#define IOCTL_TDMB_OPEN_SUBCHANNEL		_IOW(RAONTV_IOC_MAGIC,35, IOCTL_TDMB_SUB_CH_INFO)
#define IOCTL_TDMB_CLOSE_SUBCHANNEL		_IOW(RAONTV_IOC_MAGIC,36, unsigned int)
#define IOCTL_TDMB_GET_LOCK_STATUS		_IOR(RAONTV_IOC_MAGIC,37, unsigned int)
#define IOCTL_TDMB_GET_SIGNAL_INFO		_IOR(RAONTV_IOC_MAGIC,38, IOCTL_TDMB_SIGNAL_INFO)


/*                                                                              
                              
                                                                              */
#define MAX_NUM_FM_EXIST_CHANNEL 256

typedef struct
{
	unsigned int start_freq; //      
	unsigned int end_freq;   //      
	unsigned int num_ch_buf;  //      
	unsigned int ch_buf[MAX_NUM_FM_EXIST_CHANNEL]; //       
	int num_deteced_ch; //       
} IOCTL_FM_SCAN_INFO;

#define IOCTL_FM_POWER_ON				_IOW(RAONTV_IOC_MAGIC,50, E_RTV_ADC_CLK_FREQ_TYPE)
#define IOCTL_FM_POWER_OFF			_IO(RAONTV_IOC_MAGIC, 51)
#define IOCTL_FM_SET_FREQ				_IOW(RAONTV_IOC_MAGIC,52, unsigned int)
#define IOCTL_FM_SCAN_FREQ				_IOW(RAONTV_IOC_MAGIC,53, IOCTL_FM_SCAN_INFO)
#define IOCTL_FM_GET_LOCK_STATUS  		_IOR(RAONTV_IOC_MAGIC,54, unsigned int)


#ifdef __cplusplus 
} 
#endif 

#endif /*                   */




