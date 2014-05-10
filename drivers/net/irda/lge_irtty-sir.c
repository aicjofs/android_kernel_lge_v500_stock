/*                                                                    
  
                                 
                     
                                             
  
                                                                    */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#ifdef CONFIG_LGE_IRDA_FACTORY
#include <linux/proc_fs.h>
#endif /*                         */

#include <net/irda/irda.h>
#include <net/irda/irda_device.h>

#include "sir-dev.h"
#include "lge_irtty-sir.h"

static int qos_mtt_bits = 0x03;      /*              */
/*           
        
                                    
                                
                                                   
                                 
 */
#ifdef CONFIG_LGE_IRDA_FACTORY
/*                                               */
typedef enum {
	IRDA_FACTORY_MODE_OFF = 0,		/*             */
	IRDA_FACTORY_MODE_MASTER,		/*                      */
	IRDA_FACTORY_MODE_SLAVE,		/*                                  */
	IRDA_FACTORY_MODE_PASS			/*                                       */
} irda_factory_mode;

/*                                       */
#define PROC_CMD_MAX_LEN	20
#define PROC_CMD_SEND	"send"						/*                               */
#define PROC_CMD_MODE_OFF	"mode_off"				/*                                       */
#define PROC_CMD_MODE_MASTER	"mode_master"		/*                       */
#define PROC_CMD_MODE_SLAVE	"mode_slave"			/*                      */

/*                                             
         
                                                                
                                                              
                                          
                                                 
                                                          
 */
#define MAGIC_NUM_LENGTH	8
const unsigned char MAGIC_NUM_CODE[] =
{
	0x08, 0x04, 0x00, 0x08, 0x06, 0x09, 0x00, 0x01
};
const unsigned char MAGIC_NUM_RESPONSE[] =
{
	0x09, 0x00, 0x08, 0x09, 0x06, 0x06, 0x04, 0x09
};

/*                           
                                                         
                                                                   
 */
static struct sir_dev *_dev = NULL;
irda_factory_mode factory_mode = IRDA_FACTORY_MODE_OFF;
#endif /*                         */

module_param(qos_mtt_bits, int, 0);
MODULE_PARM_DESC(qos_mtt_bits, "Minimum Turn Time");

/*                                                         */

/*                                                                        */

/*                                                     
                                                         
                                                         
                                                       
                                  
 */

static int irtty_chars_in_buffer(struct sir_dev *dev)
{
	struct sirtty_cb *priv = dev->priv;
	IRDA_ASSERT(priv != NULL, return -1;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

	return tty_chars_in_buffer(priv->tty);
}

/*                                                              
                                    
                                                                       
  
                                                                    
                                                                 
  
                                                                        
                                                                        
                                                       
  
                                                 
                                              
 */

#define USBSERIAL_TX_DONE_DELAY	60

static void irtty_wait_until_sent(struct sir_dev *dev)
{
	struct sirtty_cb *priv = dev->priv;
	struct tty_struct *tty;
	IRDA_ASSERT(priv != NULL, return;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);

	tty = priv->tty;
	if (tty->ops->wait_until_sent) {
		tty->ops->wait_until_sent(tty, msecs_to_jiffies(100));
	}
	else {
		msleep(USBSERIAL_TX_DONE_DELAY);
	}
}

/*
                                            
  
                                          
  
                                                                 
                                                        
                                                                         
 */

static int irtty_change_speed(struct sir_dev *dev, unsigned speed)
{
	struct sirtty_cb *priv = dev->priv;
	struct tty_struct *tty;
        struct ktermios old_termios;
	int cflag;
	IRDA_ASSERT(priv != NULL, return -1;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

	tty = priv->tty;

	mutex_lock(&tty->termios_mutex);
	old_termios = *(tty->termios);
	cflag = tty->termios->c_cflag;
	tty_encode_baud_rate(tty, speed, speed);
	if (tty->ops->set_termios)
		tty->ops->set_termios(tty, &old_termios);
	priv->io.speed = speed;
	mutex_unlock(&tty->termios_mutex);

	return 0;
}

/*
                                             
  
                                                                          
                              
 */

static int irtty_set_dtr_rts(struct sir_dev *dev, int dtr, int rts)
{
	struct sirtty_cb *priv = dev->priv;
	int set = 0;
	int clear = 0;
	IRDA_ASSERT(priv != NULL, return -1;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

	if (rts)
		set |= TIOCM_RTS;
	else
		clear |= TIOCM_RTS;
	if (dtr)
		set |= TIOCM_DTR;
	else
		clear |= TIOCM_DTR;

	/*
                                                                      
                                
                                                           
                               
  */
	IRDA_ASSERT(priv->tty->ops->tiocmset != NULL, return -1;);
	priv->tty->ops->tiocmset(priv->tty, set, clear);

	return 0;
}

/*                                                         */

/*                                                    
                                                                     
                                                      
 */

static int irtty_do_write(struct sir_dev *dev, const unsigned char *ptr, size_t len)
{
	struct sirtty_cb *priv = dev->priv;
	struct tty_struct *tty;
	int writelen;
	IRDA_ASSERT(priv != NULL, return -1;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

	tty = priv->tty;
	if (!tty->ops->write)
		return 0;
	set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	writelen = tty_write_room(tty);
	if (writelen > len)
		writelen = len;
	return tty->ops->write(tty, ptr, writelen);
}

/*                                                         */

/*                                */

/*
                                               
  
                                                                          
                                                                        
                                                                    
                        
  
                                                                     
                                    
                                     
                                              
 */

static void irtty_receive_buf(struct tty_struct *tty, const unsigned char *cp,
			      char *fp, int count)
{
	struct sir_dev *dev;
	struct sirtty_cb *priv = tty->disc_data;
	int	i;
/*           
                                                                        
                                                                               
                                                                                      
                                 
 */
#ifdef CONFIG_LGE_IRDA_FACTORY
	if (factory_mode == IRDA_FACTORY_MODE_MASTER) {
		int j = 0;
		for (j = 0; j < MAGIC_NUM_LENGTH; j++) {
			if (*(cp + j) != MAGIC_NUM_CODE[j])
				return;
		}
		dev = priv->dev;
		irtty_do_write(dev, MAGIC_NUM_RESPONSE, MAGIC_NUM_LENGTH);
		return;
	} else if (factory_mode == IRDA_FACTORY_MODE_SLAVE) {
		int j = 0;
		for (j = 0; j < MAGIC_NUM_LENGTH; j++) {
			if (*(cp + j) != MAGIC_NUM_RESPONSE[j])
				return;
		}
		factory_mode = IRDA_FACTORY_MODE_PASS;
		return;
	}
#endif /*                         */
	IRDA_ASSERT(priv != NULL, return;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);

	if (unlikely(count==0))		/*                   */
		return;

	dev = priv->dev;
	if (!dev) {
		IRDA_WARNING("%s(), not ready yet!\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		/*
                                                   
   */
 		if (fp && *fp++) {
			IRDA_DEBUG(0, "Framing or parity error!\n");
			sirdev_receive(dev, NULL, 0);	/*                                 */
			return;
 		}
	}

	sirdev_receive(dev, cp, count);
}

/*
                                    
  
                                                                       
                                              
  
 */
static void irtty_write_wakeup(struct tty_struct *tty)
{
	struct sirtty_cb *priv = tty->disc_data;
	IRDA_ASSERT(priv != NULL, return;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);

	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	if (priv->dev)
		sirdev_write_complete(priv->dev);
}

/*                                                         */

/*
                                           
  
 */

static inline void irtty_stop_receiver(struct tty_struct *tty, int stop)
{
	struct ktermios old_termios;
	int cflag;
	mutex_lock(&tty->termios_mutex);
	old_termios = *(tty->termios);
	cflag = tty->termios->c_cflag;

	if (stop)
		cflag &= ~CREAD;
	else
		cflag |= CREAD;

	tty->termios->c_cflag = cflag;
	if (tty->ops->set_termios)
		tty->ops->set_termios(tty, &old_termios);
	mutex_unlock(&tty->termios_mutex);
}

/*                                                               */

/*                                         */
static DEFINE_MUTEX(irtty_mutex);

/*                                                            */

static int irtty_start_dev(struct sir_dev *dev)
{
	struct sirtty_cb *priv;
	struct tty_struct *tty;
	/*                                 */
	mutex_lock(&irtty_mutex);

	priv = dev->priv;
	if (unlikely(!priv || priv->magic!=IRTTY_MAGIC)) {
		mutex_unlock(&irtty_mutex);
		return -ESTALE;
	}

	tty = priv->tty;

	if (tty->ops->start)
		tty->ops->start(tty);
	/*                                    */
	irtty_stop_receiver(tty, FALSE);

	mutex_unlock(&irtty_mutex);
	return 0;
}

/*                                                              */

static int irtty_stop_dev(struct sir_dev *dev)
{
	struct sirtty_cb *priv;
	struct tty_struct *tty;
	/*                                 */
	mutex_lock(&irtty_mutex);

	priv = dev->priv;
	if (unlikely(!priv || priv->magic!=IRTTY_MAGIC)) {
		mutex_unlock(&irtty_mutex);
		return -ESTALE;
	}

	tty = priv->tty;

	/*                                      */
	irtty_stop_receiver(tty, TRUE);
	if (tty->ops->stop)
		tty->ops->stop(tty);

	mutex_unlock(&irtty_mutex);

	return 0;
}

/*                                                         */

static struct sir_driver sir_tty_drv = {
	.owner			= THIS_MODULE,
	.driver_name		= "sir_tty",
	.start_dev		= irtty_start_dev,
	.stop_dev		= irtty_stop_dev,
	.do_write		= irtty_do_write,
	.chars_in_buffer	= irtty_chars_in_buffer,
	.wait_until_sent	= irtty_wait_until_sent,
	.set_speed		= irtty_change_speed,
	.set_dtr_rts		= irtty_set_dtr_rts,
};

/*                                                         */

/*
                                             
  
                                               
  
 */
static int irtty_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct irtty_info { char name[6]; } info;
	struct sir_dev *dev;
	struct sirtty_cb *priv = tty->disc_data;
	int err = 0;
	IRDA_ASSERT(priv != NULL, return -ENODEV;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -EBADR;);

	IRDA_DEBUG(3, "%s(cmd=0x%X)\n", __func__, cmd);

	dev = priv->dev;
	IRDA_ASSERT(dev != NULL, return -1;);

	switch (cmd) {
	case IRTTY_IOCTDONGLE:
		/*                                 */
		err = sirdev_set_dongle(dev, (IRDA_DONGLE) arg);
		break;

	case IRTTY_IOCGET:
		IRDA_ASSERT(dev->netdev != NULL, return -1;);

		memset(&info, 0, sizeof(info));
		strncpy(info.name, dev->netdev->name, sizeof(info.name)-1);

		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			err = -EFAULT;
		}
		break;
	default:
		err = tty_mode_ioctl(tty, file, cmd, arg);
		break;
	}
	return err;
}

#ifdef CONFIG_LGE_IRDA_FACTORY
static int lge_irda_read_proc(
	char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = buf;

	if (factory_mode == IRDA_FACTORY_MODE_PASS) {
		count = sprintf(p, "pass");
	} else {
		count = sprintf(p, "fail");
	}

	return count;
}

static int lge_irda_write_proc(struct file *file, const char __user *buffer,
	unsigned long count, void *data)
{
	char buf[PROC_CMD_MAX_LEN];
	int ret;

	if (count > PROC_CMD_MAX_LEN) {
		ret = -EINVAL;
		goto write_proc_failed;
	}

	if (copy_from_user(buf, buffer, PROC_CMD_MAX_LEN)) {
		ret = -EFAULT;
		goto write_proc_failed;
	}

	if ((memcmp(buf, PROC_CMD_SEND, strlen(PROC_CMD_SEND)) == 0) &&
		(factory_mode == IRDA_FACTORY_MODE_SLAVE)) {
		if (_dev != NULL) {
			irtty_do_write(_dev, MAGIC_NUM_CODE, MAGIC_NUM_LENGTH);
		}
	} else if (memcmp(buf, PROC_CMD_MODE_OFF, strlen(PROC_CMD_MODE_OFF)) == 0) {
		factory_mode = IRDA_FACTORY_MODE_OFF;
	} else if (memcmp(buf, PROC_CMD_MODE_MASTER, strlen(PROC_CMD_MODE_MASTER)) == 0) {
		factory_mode = IRDA_FACTORY_MODE_MASTER;
	} else if (memcmp(buf, PROC_CMD_MODE_SLAVE, strlen(PROC_CMD_MODE_SLAVE)) == 0) {
		factory_mode = IRDA_FACTORY_MODE_SLAVE;
	} else {
		printk("%s, unknown command\n", __func__);
		count = 0;
	}
	return count;

write_proc_failed:
	return ret;
}
#endif /*                         */
/*
                            
  
                                                                  
                                                                         
                                                     
 */
static int irtty_open(struct tty_struct *tty)
{
	struct sir_dev *dev;
	struct sirtty_cb *priv;
	int ret = 0;
	/*                                                     */

	/*                                              */
	if (tty->disc_data != NULL) {
		priv = tty->disc_data;
		if (priv && priv->magic == IRTTY_MAGIC) {
			ret = -EEXIST;
			goto out;
		}
		tty->disc_data = NULL;		/*     */
	}

	/*                             */
	irtty_stop_receiver(tty, TRUE);
	if (tty->ops->stop)
		tty->ops->stop(tty);

	tty_driver_flush_buffer(tty);

	/*                    */
	sir_tty_drv.qos_mtt_bits = qos_mtt_bits;

	/*                                           */
	dev = sirdev_get_instance(&sir_tty_drv, tty->name);
	if (!dev) {
		ret = -ENODEV;
		goto out;
	}

	/*                                    */
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		goto out_put;

	priv->magic = IRTTY_MAGIC;
	priv->tty = tty;
	priv->dev = dev;

	/*                                                             */
	mutex_lock(&irtty_mutex);

	dev->priv = priv;
	tty->disc_data = priv;
	tty->receive_room = 65536;

	mutex_unlock(&irtty_mutex);

	IRDA_DEBUG(0, "%s - %s: irda line discipline opened\n", __func__, tty->name);

	/*                                                 
                                 
  */
	gpio_set_value_cansleep(GPIO_IRDA_PWDN, GPIO_IRDA_PWDN_ENABLE);
	gpio_set_value_cansleep(GPIO_IRDA_SW_EN, GPIO_IRDA_SW_EN_ENABLE);

#ifdef CONFIG_LGE_IRDA_FACTORY
	_dev = dev;
#endif /*                         */
	return 0;

out_put:
	sirdev_put_instance(dev);
out:
	return ret;
}

/*
                              
  
                                                                            
                                                                             
                                                  
 */
static void irtty_close(struct tty_struct *tty)
{
	struct sirtty_cb *priv = tty->disc_data;

	IRDA_ASSERT(priv != NULL, return;);
	IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);
	/*                                                   
                                                   
                                                      
                                                       
                          
                                                        
                                                             
                                                                 
                                                                 
                                                           
                                      
   
                      
  */

	/*                 */
	tty->disc_data = NULL;

	sirdev_put_instance(priv->dev);

	/*          */
	irtty_stop_receiver(tty, TRUE);
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	if (tty->ops->stop)
		tty->ops->stop(tty);

	kfree(priv);

	IRDA_DEBUG(0, "%s - %s: irda line discipline closed\n", __func__, tty->name);

	/*                                                 
                                 
  */
	gpio_set_value_cansleep(GPIO_IRDA_SW_EN, GPIO_IRDA_SW_EN_DISABLE);
	gpio_set_value_cansleep(GPIO_IRDA_PWDN, GPIO_IRDA_PWDN_DISABLE);

#ifdef CONFIG_LGE_IRDA_FACTORY
	_dev = NULL;
#endif /*                         */
}

/*                                                         */

static struct tty_ldisc_ops irda_ldisc = {
	.magic		= TTY_LDISC_MAGIC,
 	.name		= "irda",
	.flags		= 0,
	.open		= irtty_open,
	.close		= irtty_close,
	.read		= NULL,
	.write		= NULL,
	.ioctl		= irtty_ioctl,
 	.poll		= NULL,
	.receive_buf	= irtty_receive_buf,
	.write_wakeup	= irtty_write_wakeup,
	.owner		= THIS_MODULE,
};

/*                                                         */

static int __init irtty_sir_init(void)
{
	int err;

#ifdef CONFIG_LGE_IRDA_FACTORY
	struct proc_dir_entry *d_entry;
#endif
	printk("%s\n", __FUNCTION__);
	if ((err = tty_register_ldisc(N_IRDA, &irda_ldisc)) != 0)
		IRDA_ERROR("IrDA: can't register line discipline (err = %d)\n",
			   err);
	/*                                                 
                                 
  */

	if((err = gpio_request_one(GPIO_IRDA_SW_EN, GPIOF_OUT_INIT_LOW, "IrDA_SW_EN")) != 0) {
		IRDA_DEBUG(4, "%s : level shifter open error\n", __func__);
		return err;
	}
	if((err = gpio_request_one(GPIO_IRDA_PWDN, GPIOF_OUT_INIT_HIGH, "IrDA_PWDN")) != 0) {
		IRDA_DEBUG(4, "%s : irda HW module open error\n", __func__);
		return err;
	}

/*           
                                             
                                 
 */
#ifdef CONFIG_LGE_IRDA_FACTORY
	d_entry = create_proc_entry("lge_irda_factory",
			S_IRUGO | S_IWUSR | S_IWGRP, NULL);
	if (d_entry) {
		d_entry->read_proc = lge_irda_read_proc;
		d_entry->write_proc = lge_irda_write_proc;
		d_entry->data = NULL;
	}
#endif

	return err;
}

static void __exit irtty_sir_cleanup(void)
{
	int err;
	printk("%s\n", __FUNCTION__);
	gpio_free(GPIO_IRDA_SW_EN);
	gpio_free(GPIO_IRDA_PWDN);

	if ((err = tty_unregister_ldisc(N_IRDA))) {
		IRDA_ERROR("%s(), can't unregister line discipline (err = %d)\n",
			   __func__, err);
	}
}

module_init(irtty_sir_init);
module_exit(irtty_sir_cleanup);

MODULE_AUTHOR("Dag Brattli <dagb@cs.uit.no>");
MODULE_DESCRIPTION("IrDA TTY device driver");
MODULE_ALIAS_LDISC(N_IRDA);
MODULE_LICENSE("GPL");

