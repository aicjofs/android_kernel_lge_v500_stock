/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*===========================================================================
  @file vos_memory.c

  @brief Virtual Operating System Services Memory API

  
  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                            
    
                                             
   
                         
                                                                        
                                                                 
   
   
                                    
   
   
                                      
                                                                             
     
                                                                           */ 

/*                                                                           
                
                                                                           */

#include "vos_memory.h"
#include "vos_trace.h"

#ifdef MEMORY_DEBUG
#include "wlan_hdd_dp_utils.h"

hdd_list_t vosMemList;

static v_U8_t WLAN_MEM_HEADER[] =  {0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68 };
static v_U8_t WLAN_MEM_TAIL[]   =  {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87};

struct s_vos_mem_struct
{
   hdd_list_node_t pNode;
   char* fileName;
   unsigned int lineNum;
   unsigned int size;
   v_U8_t header[8];
};
#endif

/*                                                                           
                                         
                                                                           */

/*                                                                           
                    
                                                                           */
  
/*                                                                           
                   
                                                                           */

/*                                                                           
                                   
                                                                           */
#ifdef MEMORY_DEBUG
void vos_mem_init()
{
   /*                                                 */
   hdd_list_init(&vosMemList, 60000);  
   return; 
}

void vos_mem_clean()
{
    v_SIZE_t listSize;
    hdd_list_size(&vosMemList, &listSize);

    if(listSize)
    {
       hdd_list_node_t* pNode;
       VOS_STATUS vosStatus;

       struct s_vos_mem_struct* memStruct;
 
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             "%s: List is not Empty. listSize %d ", __FUNCTION__, (int)listSize);

       do
       {
          spin_lock(&vosMemList.lock);
          vosStatus = hdd_list_remove_front(&vosMemList, &pNode);
          spin_unlock(&vosMemList.lock);
          if(VOS_STATUS_SUCCESS == vosStatus)
          {
             memStruct = (struct s_vos_mem_struct*)pNode;
             VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "Memory Leak@ File %s, @Line %d, size %d", 
                   memStruct->fileName, (int)memStruct->lineNum, memStruct->size);
             kfree((v_VOID_t*)memStruct);
          }
       }while(vosStatus == VOS_STATUS_SUCCESS);
    }
}

void vos_mem_exit()
{
    vos_mem_clean();    
    hdd_list_destroy(&vosMemList);
}

v_VOID_t * vos_mem_malloc_debug( v_SIZE_t size, char* fileName, v_U32_t lineNum)
{
   struct s_vos_mem_struct* memStruct;
   v_VOID_t* memPtr = NULL;
   v_SIZE_t new_size;

   if (size > (1024*1024))
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: called with arg > 1024K; passed in %d !!!", __FUNCTION__,size); 
       return NULL;
   }
   if (in_interrupt())
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
               "%s is being called in interrupt context, using GPF_ATOMIC.", __FUNCTION__);
       return kmalloc(size, GFP_ATOMIC);
      
   }

   new_size = size + sizeof(struct s_vos_mem_struct) + 8; 

   memStruct = (struct s_vos_mem_struct*)kmalloc(new_size,GFP_KERNEL);

   if(memStruct != NULL)
   {
      VOS_STATUS vosStatus;

      memStruct->fileName = fileName;
      memStruct->lineNum  = lineNum;
      memStruct->size     = size;

      vos_mem_copy(&memStruct->header[0], &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));
      vos_mem_copy( (v_U8_t*)(memStruct + 1) + size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

      spin_lock(&vosMemList.lock);
      vosStatus = hdd_list_insert_front(&vosMemList, &memStruct->pNode);
      spin_unlock(&vosMemList.lock);
      if(VOS_STATUS_SUCCESS != vosStatus)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
             "%s: Unable to insert node into List vosStatus %d\n", __FUNCTION__, vosStatus);
      }

      memPtr = (v_VOID_t*)(memStruct + 1); 
   }
   return memPtr;
}

v_VOID_t vos_mem_free( v_VOID_t *ptr )
{
    if (ptr != NULL)
    {
        VOS_STATUS vosStatus;
        struct s_vos_mem_struct* memStruct = ((struct s_vos_mem_struct*)ptr) - 1;

        spin_lock(&vosMemList.lock);
        vosStatus = hdd_list_remove_node(&vosMemList, &memStruct->pNode);
        spin_unlock(&vosMemList.lock);

        if(VOS_STATUS_SUCCESS == vosStatus)
        {
            if(0 == vos_mem_compare(memStruct->header, &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER)) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Header is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            if(0 == vos_mem_compare( (v_U8_t*)ptr + memStruct->size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL ) ) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Trailer is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            kfree((v_VOID_t*)memStruct);
        }
        else
        {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                      "%s: Unallocated memory (double free?)", __FUNCTION__);
            VOS_ASSERT(0);
        }
    }
}
#else
v_VOID_t * vos_mem_malloc( v_SIZE_t size )
{
   if (size > (1024*1024))
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: called with arg > 1024K; passed in %d !!!", __FUNCTION__,size); 
       return NULL;
   }
   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
      return NULL;
   }
   return kmalloc(size, GFP_KERNEL);
}   

v_VOID_t vos_mem_free( v_VOID_t *ptr )
{
    if (ptr == NULL)
      return;
    kfree(ptr);
}
#endif

v_VOID_t vos_mem_set( v_VOID_t *ptr, v_SIZE_t numBytes, v_BYTE_t value )
{
   if (ptr == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter ptr", __FUNCTION__);
      return;
   }
   memset(ptr, value, numBytes);
}

v_VOID_t vos_mem_zero( v_VOID_t *ptr, v_SIZE_t numBytes )
{
   if (0 == numBytes)
   {
      //                                   
      return;
   }

   if (ptr == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter ptr", __FUNCTION__);
      return;
   }
   memset(ptr, 0, numBytes);
   
}


//                                                                                                              
//                        
extern int csrCheckValidateLists(void * dest, const void *src, v_SIZE_t num, int idx);

v_VOID_t vos_mem_copy( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if (0 == numBytes)
   {
      //                                            
      return;
   }

   if ((pDst == NULL) || (pSrc==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s called with NULL parameter, source:%p destination:%p",
                __FUNCTION__, pSrc, pDst);
      VOS_ASSERT(0);
      return;
   }
   //                                                                                           
   csrCheckValidateLists(pDst, pSrc, numBytes, 1);
   memcpy(pDst, pSrc, numBytes);
   csrCheckValidateLists(pDst, pSrc, numBytes, 2);
}

v_VOID_t vos_mem_move( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if (0 == numBytes)
   {
      //                                            
      return;
   }

   if ((pDst == NULL) || (pSrc==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s called with NULL parameter, source:%p destination:%p",
                __FUNCTION__, pSrc, pDst);
      VOS_ASSERT(0);
      return;
   }
   memmove(pDst, pSrc, numBytes);
}

v_BOOL_t vos_mem_compare( v_VOID_t *pMemory1, v_VOID_t *pMemory2, v_U32_t numBytes )
{ 
   if (0 == numBytes)
   {
      //                                                    
      return VOS_TRUE;
   }

   if ((pMemory1 == NULL) || (pMemory2==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s called with NULL parameter, p1:%p p2:%p",
                __FUNCTION__, pMemory1, pMemory2);
      VOS_ASSERT(0);
      return VOS_FALSE;
   }
   return (memcmp(pMemory1, pMemory2, numBytes)?VOS_FALSE:VOS_TRUE);
}   


v_SINT_t vos_mem_compare2( v_VOID_t *pMemory1, v_VOID_t *pMemory2, v_U32_t numBytes )

{ 
   return( (v_SINT_t) memcmp( pMemory1, pMemory2, numBytes ) );
}

/*                                                                            
  
                                                          

                                                                            
                                                                              
                                                                         
                                                              

                                                                           
                

                                                                        
                                                 
  
                                                            
  
                                                                      
                                                                          
                                                    
    
     
  
                                                                            */
#ifdef MEMORY_DEBUG
v_VOID_t * vos_mem_dma_malloc_debug( v_SIZE_t size, char* fileName, v_U32_t lineNum)
{
   struct s_vos_mem_struct* memStruct;
   v_VOID_t* memPtr = NULL;
   v_SIZE_t new_size;

   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
      return NULL;
   }

   new_size = size + sizeof(struct s_vos_mem_struct) + 8; 

   memStruct = (struct s_vos_mem_struct*)kmalloc(new_size,GFP_KERNEL);

   if(memStruct != NULL)
   {
      VOS_STATUS vosStatus;

      memStruct->fileName = fileName;
      memStruct->lineNum  = lineNum;
      memStruct->size     = size;

      vos_mem_copy(&memStruct->header[0], &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));
      vos_mem_copy( (v_U8_t*)(memStruct + 1) + size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

      spin_lock(&vosMemList.lock);
      vosStatus = hdd_list_insert_front(&vosMemList, &memStruct->pNode);
      spin_unlock(&vosMemList.lock);
      if(VOS_STATUS_SUCCESS != vosStatus)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
             "%s: Unable to insert node into List vosStatus %d\n", __FUNCTION__, vosStatus);
      }

      memPtr = (v_VOID_t*)(memStruct + 1); 
   }

   return memPtr;
}

v_VOID_t vos_mem_dma_free( v_VOID_t *ptr )
{
    if (ptr != NULL)
    {
        VOS_STATUS vosStatus;
        struct s_vos_mem_struct* memStruct = ((struct s_vos_mem_struct*)ptr) - 1;

        spin_lock(&vosMemList.lock);
        vosStatus = hdd_list_remove_node(&vosMemList, &memStruct->pNode);
        spin_unlock(&vosMemList.lock);

        if(VOS_STATUS_SUCCESS == vosStatus)
        {
            if(0 == vos_mem_compare(memStruct->header, &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER)) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Header is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            if(0 == vos_mem_compare( (v_U8_t*)ptr + memStruct->size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL ) ) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Trailer is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            kfree((v_VOID_t*)memStruct);
        }
    }
}
#else
v_VOID_t* vos_mem_dma_malloc( v_SIZE_t size )
{
   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
      return NULL;
   }
   return kmalloc(size, GFP_KERNEL);
}

/*                                                                            
  
                                                  

                                                                          

                                                                          
                                               
  
                                                                   
                        
  
                 
    
     
  
                                                                            */
v_VOID_t vos_mem_dma_free( v_VOID_t *ptr )
{
    if (ptr == NULL)
      return;
    kfree(ptr);
}
#endif