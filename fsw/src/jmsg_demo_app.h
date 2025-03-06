/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Demonstrate different use cases for using the JMSG facility
**
** Notes:
**   Nobne
**
*/
#ifndef _jmsg_demo_app_
#define _jmsg_demo_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_demo.h"
#include "udp_demo.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define JMSG_DEMO_APP_INIT_APP_EID          (JMSG_DEMO_APP_BASE_EID + 0)
#define JMSG_DEMO_APP_NOOP_EID              (JMSG_DEMO_APP_BASE_EID + 1)
#define JMSG_DEMO_APP_EXIT_EID              (JMSG_DEMO_APP_BASE_EID + 2)
#define JMSG_DEMO_APP_INVALID_MID_EID       (JMSG_DEMO_APP_BASE_EID + 3)
#define JMSG_DEMO_APP_START_USE_CASE_EID    (JMSG_DEMO_APP_BASE_EID + 4)
#define JMSG_DEMO_APP_STOP_USE_CASE_EID     (JMSG_DEMO_APP_BASE_EID + 5)
#define JMSG_DEMO_CHILD_TASK_EID            (JMSG_DEMO_APP_BASE_EID + 6)
#define JMSG_DISPATCH_CSV_TLM_CONVERTER_EID (JMSG_DEMO_APP_BASE_EID + 7)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** App Class
*/
typedef struct 
{

   /* 
   ** App Framework
   */ 

   INITBL_Class_t    IniTbl; 
   CFE_SB_PipeId_t   CmdPipe;
   CMDMGR_Class_t    CmdMgr;
   CHILDMGR_Class_t  ChildMgr;
         
   /*
   ** Telemetry Packets
   */
   
   JMSG_DEMO_StatusTlm_t   StatusTlm;

   
   /*
   ** JMSG_DEMO State & Contained Objects
   */ 
   
   uint32  PerfId;
   
   bool    InitUseCase;
   bool    UseCaseRunning;
   uint32  UseCaseTaskDelay;
   JMSG_DEMO_UseCaseTlm_Enum_t UseCase;

   CFE_SB_MsgId_t  CmdMid;
   CFE_SB_MsgId_t  SendStatusMid;
   CFE_SB_MsgId_t  JmsgTopicCsvTlmMid;
   
   MQTT_DEMO_Class_t MqttDemo;
   UDP_DEMO_Class_t  UdpDemo;

} JMSG_DEMO_APP_Class_t;


/*******************/
/** Exported Data **/
/*******************/

extern JMSG_DEMO_APP_Class_t  JMsgDemoApp;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_DEMO_AppMain
**
*/
void JMSG_DEMO_AppMain(void);


#endif /* _jmsg_demo_app_ */
