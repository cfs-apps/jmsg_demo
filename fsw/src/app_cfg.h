/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Define platform configurations for the JMSG Demo application
**
** Notes:
**   1. These macros can only be build with the application and can't
**      have a platform scope because the same app_cfg.h file name is used for
**      all applications following the object-based application design.
**
*/

#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/

#include "jmsg_demo_eds_typedefs.h"
#include "jmsg_demo_platform_cfg.h"
#include "app_c_fw.h"


/******************************************************************************
** Application Macros
*/

/*
** Versions:
**
** 1.0 - Initial release tested with jmsg_lib 1.0 and jmsg_udp 2.0
*/

#define  JMSG_DEMO_APP_MAJOR_VER   0
#define  JMSG_DEMO_APP_MINOR_VER   9

#define JMSG_DEMO_CSV_PARAM_TEXT_LEN  sizeof(JMSG_LIB_TopicCsvCmd_Payload_t)

/******************************************************************************
** Init File declarations create:
**
**  typedef enum {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigEnum;
**    
**  typedef struct {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigStruct;
**
**   const char *GetConfigStr(value);
**   ConfigEnum GetConfigVal(const char *str);
**
** XX(name,type)
*/

#define CFG_APP_CFE_NAME        APP_CFE_NAME
#define CFG_APP_MAIN_PERF_ID    APP_MAIN_PERF_ID
#define CFG_APP_CMD_PIPE_NAME   APP_CMD_PIPE_NAME
#define CFG_APP_CMD_PIPE_DEPTH  APP_CMD_PIPE_DEPTH

#define CFG_JMSG_DEMO_CMD_TOPICID                JMSG_DEMO_CMD_TOPICID
#define CFG_JMSG_DEMO_STATUS_TLM_TOPICID         JMSG_DEMO_STATUS_TLM_TOPICID
#define CFG_JMSG_DEMO_MQTT_DISCRETE_TLM_TOPICID  JMSG_DEMO_MQTT_DISCRETE_TLM_TOPICID
#define CFG_JMSG_DEMO_MQTT_RATE_TLM_TOPICID      JMSG_DEMO_MQTT_RATE_TLM_TOPICID
#define CFG_JMSG_DEMO_UDP_RPI_TLM_TOPICID        JMSG_DEMO_UDP_RPI_TLM_TOPICID
#define CFG_JMSG_LIB_TOPIC_CSV_CMD_TOPICID       JMSG_LIB_TOPIC_CSV_CMD_TOPICID
#define CFG_JMSG_LIB_TOPIC_CSV_TLM_TOPICID       JMSG_LIB_TOPIC_CSV_TLM_TOPICID
#define CFG_JMSG_LIB_TOPIC_SCRIPT_CMD_TOPICID    JMSG_LIB_TOPIC_SCRIPT_CMD_TOPICID
#define CFG_SEND_STATUS_TLM_TOPICID              BC_SCH_2_SEC_TOPICID
      
#define CFG_CHILD_NAME       CHILD_NAME
#define CFG_CHILD_PERF_ID    CHILD_PERF_ID
#define CFG_CHILD_STACK_SIZE CHILD_STACK_SIZE
#define CFG_CHILD_PRIORITY   CHILD_PRIORITY

#define CFG_USE_CASE_TASK_DELAY  USE_CASE_TASK_DELAY


#define APP_CONFIG(XX) \
   XX(APP_CFE_NAME,char*) \
   XX(APP_MAIN_PERF_ID,uint32) \
   XX(APP_CMD_PIPE_NAME,char*) \
   XX(APP_CMD_PIPE_DEPTH,uint32) \
   XX(JMSG_DEMO_CMD_TOPICID,uint32) \
   XX(JMSG_DEMO_STATUS_TLM_TOPICID,uint32) \
   XX(JMSG_DEMO_MQTT_DISCRETE_TLM_TOPICID,uint32) \
   XX(JMSG_DEMO_MQTT_RATE_TLM_TOPICID,uint32) \
   XX(JMSG_DEMO_UDP_RPI_TLM_TOPICID,uint32) \
   XX(JMSG_LIB_TOPIC_CSV_CMD_TOPICID,uint32) \
   XX(JMSG_LIB_TOPIC_CSV_TLM_TOPICID,uint32) \
   XX(JMSG_LIB_TOPIC_SCRIPT_CMD_TOPICID,uint32) \
   XX(BC_SCH_2_SEC_TOPICID,uint32) \
   XX(CHILD_NAME,char*) \
   XX(CHILD_PERF_ID,uint32) \
   XX(CHILD_STACK_SIZE,uint32) \
   XX(CHILD_PRIORITY,uint32) \
   XX(USE_CASE_TASK_DELAY,uint32) \
   
DECLARE_ENUM(Config,APP_CONFIG)


/******************************************************************************
** App level definitions that don't need to be in the ini file
**
*/

#define JMSG_DEMO_UNDEF_TLM_STR "Undefined"


/******************************************************************************
** Event Macros
**
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
*/

#define JMSG_DEMO_APP_BASE_EID  (APP_C_FW_APP_BASE_EID +  0)
#define MQTT_DEMO_BASE_EID      (APP_C_FW_APP_BASE_EID + 20)
#define UDP_DEMO_BASE_EID       (APP_C_FW_APP_BASE_EID + 40)

#endif /* _app_cfg_ */
