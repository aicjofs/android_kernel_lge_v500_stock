/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/qmi_encdec.h>

#include "qmi_encdec_priv.h"

#define TLV_LEN_SIZE sizeof(uint16_t)
#define TLV_TYPE_SIZE sizeof(uint8_t)

static int _qmi_kernel_encode(struct elem_info *ei_array,
			      void *out_buf, void *in_c_struct,
			      int enc_level);

static int _qmi_kernel_decode(struct elem_info *ei_array,
			      void *out_c_struct,
			      void *in_buf, uint32_t in_buf_len,
			      int dec_level);

/* 
                                                          
                                          
                                                    
                                          
                                           
  
                                                              
 */
int qmi_kernel_encode(struct msg_desc *desc,
		      void *out_buf, uint32_t out_buf_len,
		      void *in_c_struct)
{
	int enc_level = 1;

	if (!desc || !desc->ei_array)
		return -EINVAL;

	if (!out_buf || !in_c_struct)
		return -EINVAL;

	if (desc->max_msg_len < out_buf_len)
		return -ETOOSMALL;

	return _qmi_kernel_encode(desc->ei_array, out_buf,
				  in_c_struct, enc_level);
}
EXPORT_SYMBOL(qmi_kernel_encode);

/* 
                                                                        
                                                     
                                                          
                                                                
                                                                      
  
                                                   
  
                                                                        
                                                                         
                                                                            
                                                                         
                                                               
 */
static int qmi_encode_basic_elem(void *buf_dst, void *buf_src,
				 uint32_t elem_len, uint32_t elem_size)
{
	uint32_t i, rc = 0;

	for (i = 0; i < elem_len; i++) {
		QMI_ENCDEC_ENCODE_N_BYTES(buf_dst, buf_src, elem_size);
		rc += elem_size;
	}

	return rc;
}

/* 
                                                                  
                                                             
                                                     
                                                          
                                                                
                                                                     
  
                                                               
                         
  
                                                                          
                                                                        
                                                                          
                                                                        
                                                               
 */
static int qmi_encode_struct_elem(struct elem_info *ei_array,
				  void *buf_dst, void *buf_src,
				  uint32_t elem_len, int enc_level)
{
	int i, rc, encoded_bytes = 0;
	struct elem_info *temp_ei = ei_array;

	for (i = 0; i < elem_len; i++) {
		rc = _qmi_kernel_encode(temp_ei->ei_array,
					buf_dst, buf_src, enc_level);
		if (rc < 0) {
			pr_err("%s: STRUCT Encode failure\n", __func__);
			return rc;
		}
		buf_dst = buf_dst + rc;
		buf_src = buf_src + temp_ei->elem_size;
		encoded_bytes += rc;
	}

	return encoded_bytes;
}

/* 
                                                                            
                                                               
  
                                                                
  
                                                                      
                                                                     
                                                                        
                  
 */
static struct elem_info *skip_to_next_elem(struct elem_info *ei_array)
{
	struct elem_info *temp_ei = ei_array;
	uint8_t tlv_type;

	do {
		tlv_type = temp_ei->tlv_type;
		temp_ei = temp_ei + 1;
	} while (tlv_type == temp_ei->tlv_type);

	return temp_ei;
}

/* 
                                              
                                                                       
                                                    
                                                          
                                                                          
                                                        
  
                                                               
                         
 */
static int _qmi_kernel_encode(struct elem_info *ei_array,
			      void *out_buf, void *in_c_struct,
			      int enc_level)
{
	struct elem_info *temp_ei = ei_array;
	uint8_t opt_flag_value = 0;
	uint32_t data_len_value = 0, data_len_sz;
	uint8_t *buf_dst = (uint8_t *)out_buf;
	uint8_t *tlv_pointer;
	uint32_t tlv_len;
	uint8_t tlv_type;
	uint32_t encoded_bytes = 0;
	void *buf_src;
	int encode_tlv = 0;
	int rc;

	tlv_pointer = buf_dst;
	tlv_len = 0;
	buf_dst = buf_dst + (TLV_LEN_SIZE + TLV_TYPE_SIZE);

	while (temp_ei->data_type != QMI_EOTI) {
		buf_src = in_c_struct + temp_ei->offset;
		tlv_type = temp_ei->tlv_type;

		if (temp_ei->is_array == NO_ARRAY) {
			data_len_value = 1;
		} else if (temp_ei->is_array == STATIC_ARRAY) {
			data_len_value = temp_ei->elem_len;
		} else if (data_len_value <= 0 ||
			    temp_ei->elem_len < data_len_value) {
			pr_err("%s: Invalid data length\n", __func__);
			return -EINVAL;
		}

		switch (temp_ei->data_type) {
		case QMI_OPT_FLAG:
			rc = qmi_encode_basic_elem(&opt_flag_value, buf_src,
						   1, sizeof(uint8_t));
			if (opt_flag_value)
				temp_ei = temp_ei + 1;
			else
				temp_ei = skip_to_next_elem(temp_ei);
			break;

		case QMI_DATA_LEN:
			memcpy(&data_len_value, buf_src, temp_ei->elem_size);
			data_len_sz = temp_ei->elem_size == sizeof(uint8_t) ?
					sizeof(uint8_t) : sizeof(uint16_t);
			rc = qmi_encode_basic_elem(buf_dst, &data_len_value,
						   1, data_len_sz);
			if (data_len_value) {
				UPDATE_ENCODE_VARIABLES(temp_ei, buf_dst,
					encoded_bytes, tlv_len, encode_tlv, rc);
				encode_tlv = 0;
			} else {
				temp_ei = skip_to_next_elem(temp_ei);
			}
			break;

		case QMI_UNSIGNED_1_BYTE:
		case QMI_UNSIGNED_2_BYTE:
		case QMI_UNSIGNED_4_BYTE:
		case QMI_UNSIGNED_8_BYTE:
		case QMI_SIGNED_2_BYTE_ENUM:
		case QMI_SIGNED_4_BYTE_ENUM:
			rc = qmi_encode_basic_elem(buf_dst, buf_src,
				data_len_value, temp_ei->elem_size);
			UPDATE_ENCODE_VARIABLES(temp_ei, buf_dst,
				encoded_bytes, tlv_len, encode_tlv, rc);
			break;

		case QMI_STRUCT:
			rc = qmi_encode_struct_elem(temp_ei, buf_dst, buf_src,
				data_len_value, (enc_level + 1));
			if (rc < 0)
				return rc;
			UPDATE_ENCODE_VARIABLES(temp_ei, buf_dst,
				encoded_bytes, tlv_len, encode_tlv, rc);
			break;

		default:
			pr_err("%s: Unrecognized data type\n", __func__);
			return -EINVAL;

		}

		if (encode_tlv && enc_level == 1) {
			QMI_ENCDEC_ENCODE_TLV(tlv_type, tlv_len, tlv_pointer);
			encoded_bytes += (TLV_TYPE_SIZE + TLV_LEN_SIZE);
			tlv_pointer = buf_dst;
			tlv_len = 0;
			buf_dst = buf_dst + TLV_LEN_SIZE + TLV_TYPE_SIZE;
			encode_tlv = 0;
		}
	}
	return encoded_bytes;
}

/* 
                                                     
                                          
                                                         
                                                          
                                                   
  
                                       
 */
int qmi_kernel_decode(struct msg_desc *desc, void *out_c_struct,
		      void *in_buf, uint32_t in_buf_len)
{
	int dec_level = 1;
	int rc = 0;

	if (!desc || !desc->ei_array)
		return -EINVAL;

	if (!out_c_struct || !in_buf || !in_buf_len)
		return -EINVAL;

	if (desc->max_msg_len < in_buf_len)
		return -EINVAL;

	rc = _qmi_kernel_decode(desc->ei_array, out_c_struct,
				in_buf, in_buf_len, dec_level);
	if (rc < 0)
		return rc;
	else
		return 0;
}
EXPORT_SYMBOL(qmi_kernel_decode);

/* 
                                                                        
                                                 
                                                               
                                               
                                                                      
  
                                                              
  
                                                                              
                                                                             
                                                                             
                                                                         
                                                               
 */
static int qmi_decode_basic_elem(void *buf_dst, void *buf_src,
				 uint32_t elem_len, uint32_t elem_size)
{
	uint32_t i, rc = 0;

	for (i = 0; i < elem_len; i++) {
		QMI_ENCDEC_DECODE_N_BYTES(buf_dst, buf_src, elem_size);
		rc += elem_size;
	}

	return rc;
}

/* 
                                                                  
                                                             
                                                 
                                                               
                                               
                                                                  
                                 
                                                                     
  
                                                                
                         
  
                                                                              
                                                                           
                                                                           
                                                                        
                                                               
 */
static int qmi_decode_struct_elem(struct elem_info *ei_array, void *buf_dst,
				  void *buf_src, uint32_t elem_len,
				  uint32_t tlv_len, int dec_level)
{
	int i, rc, decoded_bytes = 0;
	struct elem_info *temp_ei = ei_array;

	for (i = 0; i < elem_len; i++) {
		rc = _qmi_kernel_decode(temp_ei->ei_array, buf_dst, buf_src,
					(tlv_len/elem_len), dec_level);
		if (rc < 0)
			return rc;
		if (rc != (tlv_len/elem_len)) {
			pr_err("%s: Fault in decoding\n", __func__);
			return -EFAULT;
		}
		buf_src = buf_src + rc;
		buf_dst = buf_dst + temp_ei->elem_size;
		decoded_bytes += rc;
	}

	return decoded_bytes;
}

/* 
                                                          
                                                             
                                                 
  
                                            
  
                                                                     
                                                                  
                                                                      
                                              
 */
static struct elem_info *find_ei(struct elem_info *ei_array,
				   uint32_t type)
{
	struct elem_info *temp_ei = ei_array;
	while (temp_ei->data_type != QMI_EOTI) {
		if (temp_ei->tlv_type == (uint8_t)type)
			return temp_ei;
		temp_ei = temp_ei + 1;
	}
	return NULL;
}

/* 
                                              
                                                                       
                                                     
                                                           
                                                       
                                                                          
                                                       
  
                                                              
                         
 */
static int _qmi_kernel_decode(struct elem_info *ei_array,
			      void *out_c_struct,
			      void *in_buf, uint32_t in_buf_len,
			      int dec_level)
{
	struct elem_info *temp_ei = ei_array;
	uint8_t opt_flag_value = 1;
	uint32_t data_len_value = 0, data_len_sz = 0;
	uint8_t *buf_dst = out_c_struct;
	uint8_t *tlv_pointer;
	uint32_t tlv_len = 0;
	uint32_t tlv_type;
	uint32_t decoded_bytes = 0;
	void *buf_src = in_buf;
	int rc;

	while (decoded_bytes < in_buf_len) {
		if (dec_level == 1) {
			tlv_pointer = buf_src;
			QMI_ENCDEC_DECODE_TLV(&tlv_type,
					      &tlv_len, tlv_pointer);
			buf_src += (TLV_TYPE_SIZE + TLV_LEN_SIZE);
			decoded_bytes += (TLV_TYPE_SIZE + TLV_LEN_SIZE);
			temp_ei = find_ei(ei_array, tlv_type);
			if (!temp_ei) {
				pr_err("%s: Inval element info\n", __func__);
				return -EINVAL;
			}
		}

		buf_dst = out_c_struct + temp_ei->offset;
		if (temp_ei->data_type == QMI_OPT_FLAG) {
			memcpy(buf_dst, &opt_flag_value, sizeof(uint8_t));
			temp_ei = temp_ei + 1;
			buf_dst = out_c_struct + temp_ei->offset;
		}

		if (temp_ei->data_type == QMI_DATA_LEN) {
			data_len_sz = temp_ei->elem_size == sizeof(uint8_t) ?
					sizeof(uint8_t) : sizeof(uint16_t);
			rc = qmi_decode_basic_elem(&data_len_value, buf_src,
						   1, data_len_sz);
			memcpy(buf_dst, &data_len_value, sizeof(uint32_t));
			temp_ei = temp_ei + 1;
			buf_dst = out_c_struct + temp_ei->offset;
			UPDATE_DECODE_VARIABLES(buf_src, decoded_bytes, rc);
		}

		if (temp_ei->is_array == NO_ARRAY) {
			data_len_value = 1;
		} else if (temp_ei->is_array == STATIC_ARRAY) {
			data_len_value = temp_ei->elem_len;
		} else if (data_len_value > temp_ei->elem_len) {
			pr_err("%s: Data len %d > max spec %d\n",
				__func__, data_len_value, temp_ei->elem_len);
			return -ETOOSMALL;
		}

		switch (temp_ei->data_type) {
		case QMI_UNSIGNED_1_BYTE:
		case QMI_UNSIGNED_2_BYTE:
		case QMI_UNSIGNED_4_BYTE:
		case QMI_UNSIGNED_8_BYTE:
		case QMI_SIGNED_2_BYTE_ENUM:
		case QMI_SIGNED_4_BYTE_ENUM:
			rc = qmi_decode_basic_elem(buf_dst, buf_src,
				data_len_value, temp_ei->elem_size);
			UPDATE_DECODE_VARIABLES(buf_src, decoded_bytes, rc);
			break;

		case QMI_STRUCT:
			rc = qmi_decode_struct_elem(temp_ei, buf_dst, buf_src,
				data_len_value, tlv_len, (dec_level + 1));
			if (rc < 0)
				return rc;
			UPDATE_DECODE_VARIABLES(buf_src, decoded_bytes, rc);
			break;
		default:
			pr_err("%s: Unrecognized data type\n", __func__);
			return -EINVAL;
		}
		temp_ei = temp_ei + 1;
	}
	return decoded_bytes;
}
MODULE_DESCRIPTION("QMI kernel enc/dec");
MODULE_LICENSE("GPL v2");
