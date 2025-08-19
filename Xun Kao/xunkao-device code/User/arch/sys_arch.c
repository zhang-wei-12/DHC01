/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#include "debug.h"

#include <lwip/opt.h>
#include <lwip/arch.h>

#include "tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sio.h"
#include "ethernetif.h"

#if !NO_SYS
#include "sys_arch.h"
#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>
#include "lwip/dhcp.h"
#include <string.h>
#include "elab/common/elab_log.h"

ELAB_TAG("sys_arch");

int errno;


u32_t lwip_sys_now;

struct sys_timeouts {
  struct sys_timeo *next;
};

struct timeoutlist
{
	struct sys_timeouts timeouts;
	xTaskHandle pid;
};

#define SYS_THREAD_MAX 4

static struct timeoutlist s_timeoutlist[SYS_THREAD_MAX];

static u16_t s_nextthread = 0;

u32_t
sys_jiffies(void)
{
  lwip_sys_now = xTaskGetTickCount();
  return lwip_sys_now;
}

u32_t
sys_now(void)
{
  lwip_sys_now = xTaskGetTickCount();
  return lwip_sys_now;
}

void
sys_init(void)
{
	int i;
	// Initialize the the per-thread sys_timeouts structures
	// make sure there are no valid pids in the list
	for(i = 0; i < SYS_THREAD_MAX; i++)
	{
		s_timeoutlist[i].pid = 0;
		s_timeoutlist[i].timeouts.next = NULL;
	}
	// keep track of how many threads have been created
	s_nextthread = 0;
}

struct sys_timeouts *sys_arch_timeouts(void)
{
	int i;
	xTaskHandle pid;
	struct timeoutlist *tl;
	pid = xTaskGetCurrentTaskHandle( );
	for(i = 0; i < s_nextthread; i++)
	{
		tl = &(s_timeoutlist[i]);
		if(tl->pid == pid)
		{
			return &(tl->timeouts);
		}
	}
	return NULL;
}

sys_prot_t sys_arch_protect(void)
{
	vPortEnterCritical();
	return 1;
}

void sys_arch_unprotect(sys_prot_t pval)
{
	( void ) pval;
	vPortExitCritical();
}

#if !NO_SYS

//test_sys_arch_waiting_fn the_waiting_fn;

//void
//test_sys_arch_wait_callback(test_sys_arch_waiting_fn waiting_fn)
//{
//  the_waiting_fn = waiting_fn;
//}

err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
  /* ���� sem */
  if(count <= 1)
  {    
    *sem = xSemaphoreCreateBinary();
    if(count == 1)
    {
      sys_sem_signal(sem);
    }
  }
  else
    *sem = xSemaphoreCreateCounting(count,count);
  
#if SYS_STATS
	++lwip_stats.sys.sem.used;
 	if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
		lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
	}
#endif /* SYS_STATS */
  
  if(*sem != SYS_SEM_NULL)
    return ERR_OK;
  else
  {
#if SYS_STATS
    ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
    printf("[sys_arch]:new sem fail!\n");
    return ERR_MEM;
  }
}

void
sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
   --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
  /* ɾ�� sem */
  vSemaphoreDelete(*sem);
  *sem = SYS_SEM_NULL;
}


int sys_sem_valid(sys_sem_t *sem)                                               
{
  return (*sem != SYS_SEM_NULL);                                    
}


void
sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}

/* 
 ���timeout������Ϊ�㣬�򷵻�ֵΪ
 �ȴ��ź��������ѵĺ����������
 �ź���δ��ָ��ʱ���ڷ����źţ�����ֵΪ
 SYS_ARCH_TIMEOUT������̲߳��صȴ��ź���
 �ú��������㡣 */
u32_t
sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;
  
  //�����ź����Ƿ���Ч
  if(*sem == SYS_SEM_NULL)
    return SYS_ARCH_TIMEOUT;
  
  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = xTaskGetTickCount();
  
  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  else
    wait_tick = portMAX_DELAY;  //һֱ����
  
  //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
  if(xSemaphoreTake(*sem, wait_tick) == pdTRUE)
    return ((xTaskGetTickCount()-start_tick)*portTICK_RATE_MS);
  else
    return SYS_ARCH_TIMEOUT;
}

void
sys_sem_signal(sys_sem_t *sem)
{
  if(xSemaphoreGive( *sem ) != pdTRUE)
    printf("[sys_arch]:sem signal fail!\n");
}

err_t
sys_mutex_new(sys_mutex_t *mutex)
{
  /* ���� sem */   
  *mutex = xSemaphoreCreateMutex();
  if(*mutex != SYS_MRTEX_NULL)
    return ERR_OK;
  else
  {
    printf("[sys_arch]:new mutex fail!\n");
    return ERR_MEM;
  }
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
  vSemaphoreDelete(*mutex);
}

void
sys_mutex_set_invalid(sys_mutex_t *mutex)
{
  *mutex = SYS_MRTEX_NULL;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
  xSemaphoreTake(*mutex,/* ��������� */
                 portMAX_DELAY); /* �ȴ�ʱ�� */
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
  xSemaphoreGive( *mutex );//����������
}


sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  sys_thread_t handle = NULL;
  BaseType_t xReturn = pdPASS;
  /* ����MidPriority_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )function,  /* ������ں��� */
                        (const char*    )name,/* �������� */
                        (uint16_t       )stacksize,  /* ����ջ��С */
                        (void*          )arg,/* ������ں������� */
                        (UBaseType_t    )prio, /* ��������ȼ� */
                        (TaskHandle_t*  )&handle);/* ������ƿ�ָ�� */ 
  if(xReturn != pdPASS)
  {
    printf("[sys_arch]:create task fail!err:%#lx\n",xReturn);
    return NULL;
  }
  return handle;
}

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
    /* ����Test_Queue */
  *mbox = xQueueCreate((UBaseType_t ) size,/* ��Ϣ���еĳ��� */
                       (UBaseType_t ) sizeof(void *));/* ��Ϣ�Ĵ�С */
#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
      if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
         lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
	  }
#endif /* SYS_STATS */
	if(NULL == *mbox)
    return ERR_MEM;
  
  return ERR_OK;
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
  if( uxQueueMessagesWaiting( *mbox ) )
	{
		/* Line for breakpoint.  Should never break here! */
		portNOP();
#if SYS_STATS
	    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */

		// TODO notify the user of failure.
	}
  
  vQueueDelete(*mbox);
  
#if SYS_STATS
     --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

int sys_mbox_valid(sys_mbox_t *mbox)          
{      
  if (*mbox == SYS_MBOX_NULL) 
    return 0;
  else
    return 1;
}   

void
sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL; 
}

void
sys_mbox_post(sys_mbox_t *q, void *msg)
{
  while(xQueueSend( *q, /* ��Ϣ���еľ�� */
                    &msg,/* ���͵���Ϣ���� */
                    portMAX_DELAY) != pdTRUE); /* �ȴ�ʱ�� */
}

err_t
sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  if(xQueueSend(*q,&msg,0) == pdPASS)  
    return ERR_OK;
  else
    return ERR_MEM;
}

err_t
sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
  return sys_mbox_trypost(q, msg);
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  void *dummyptr;
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;
  
  if ( msg == NULL )  //�����洢��Ϣ�ĵط��Ƿ���Ч
		msg = &dummyptr;
  
  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = sys_now();
  
  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  //һֱ����
  else
    wait_tick = portMAX_DELAY;
  
  //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
  if(xQueueReceive(*q,&(*msg), wait_tick) == pdTRUE)
    return ((sys_now() - start_tick)*portTICK_PERIOD_MS);
  else
  {
    *msg = NULL;
    return SYS_ARCH_TIMEOUT;
  }
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
	void *dummyptr;
	if ( msg == NULL )
		msg = &dummyptr;
  
  //�ȴ��ɹ�������ȴ���ʱ��
  if(xQueueReceive(*q,&(*msg), 0) == pdTRUE)
    return ERR_OK;
  else
    return SYS_MBOX_EMPTY;
}

#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];

void TCPIP_Init(void)
{
  tcpip_init(NULL, NULL);
  
  /* IP addresses initialization */
  /* USER CODE BEGIN 0 */
#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP4_ADDR(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP4_ADDR(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP4_ADDR(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  /* USER CODE END 0 */
  /* Initilialize the LwIP stack without RTOS */
  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }
  
  /* Set the link callback function, this function is called on change of link status �������ӻص�������������״̬�ı�ʱ���øú���*/
  //netif_set_link_callback(&gnetif, ethernetif_update_config);
  
#if LWIP_DHCP	   			//��ʹ����DHCP
  int err;
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  
  int dhcp_ok = 0, dhcp_cnt = 0;
   if (netif_is_link_up(&gnetif)){
      err = dhcp_start(&gnetif);      //����dhcp
      if(err == ERR_OK){
      
          ;//elog_debug("lwip dhcp init success...\n\n");
          while(ip_addr_cmp(&(gnetif.ip_addr),&ipaddr)){//�ȴ�dhcp�����ip��Ч
          
             vTaskDelay(1000);
             dhcp_cnt++;
             
             if (dhcp_cnt > 5)
                  break;
          } 
          
          if (dhcp_cnt <= 5)//�ȴ�dhcp�����ipʧ��
              dhcp_ok = 1;
       
      }
      else
          elog_debug("lwip dhcp init fail...\n\n");       
   }
   else
       elog_debug("Wait for the cable to be plugged in...\r\n");

   // DHCPʧ�ܣ����þ�̬IP
   if (1 != dhcp_ok){
        IP4_ADDR(&gnetif.ip_addr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
        IP4_ADDR(&gnetif.netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
        IP4_ADDR(&gnetif.gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
        netif_set_up(&gnetif);   
   }
  
#endif
  elog_debug("ip is: %d.%d.%d.%d\n\n",  \
        ((gnetif.ip_addr.addr)&0x000000ff),       \
        (((gnetif.ip_addr.addr)&0x0000ff00)>>8),  \
        (((gnetif.ip_addr.addr)&0x00ff0000)>>16), \
        ((gnetif.ip_addr.addr)&0xff000000)>>24);
}

/**
  * @brief  This function notify user about link status changement.
  * @param  netif: the network interface
  * @retval None
  */
//__weak void ethernetif_notify_conn_changed(struct netif *netif)
//{
//  /* NOTE : This is function could be implemented in user file 
//            when the callback is needed,
//  */
//  #if LWIP_DHCP
//    int err;
//    ipaddr.addr = 0;
//  
//    elog_debug("ethernetif_notify_conn_changed...\r\n");
//    if (netif_is_link_up(netif))
//    {
//      elog_debug("netif is link up\r\n");
//	  /* When the netif is fully configured this function must be called ��netif��ȫ����ʱ��������ô˺���*/
//	  netif_set_up(netif);

//      elog_debug("starting dhcp...\n");
//      err = dhcp_start(&gnetif);
//      
//      if (err == ERR_OK)
//           elog_debug("starting dhcp success!\n");
//      else 
//         elog_debug("starting dhcp fail!\n");
// 
//      int res = 0;
//      do{
//        res = ip_addr_cmp(&(gnetif.ip_addr),&ipaddr);
//        if (res){
//            vTaskDelay(1);
//        }
//      } while (res);
//  
//      elog_debug("dhcp get local ip :%d.%d.%d.%d\n\n",  \
//                  ((gnetif.ip_addr.addr)&0x000000ff),       \
//                  (((gnetif.ip_addr.addr)&0x0000ff00)>>8),  \
//                  (((gnetif.ip_addr.addr)&0x00ff0000)>>16), \
//                  ((gnetif.ip_addr.addr)&0xff000000)>>24);
//   } 
//   else 
//   {
//      elog_debug("net link is down\r\n");
//      /* When the netif link is down this function must be called ��netif���ӶϿ�ʱ����������������*/
//      netif_set_down(netif);
//   }
//  
//  #endif
//}









