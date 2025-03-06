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
**   None
**
*/

/*
** Includes
*/

#include <string.h>
#include "jmsg_demo_app.h"
#include "jmsg_demo_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ    (&(JMsgDemoApp.IniTbl))
#define  CMDMGR_OBJ    (&(JMsgDemoApp.CmdMgr))
#define  CHILDMGR_OBJ  (&(JMsgDemoApp.ChildMgr))

/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void DispatchCsvTlmConverter(const CFE_MSG_Message_t *TopicCsvCmd);
static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusPkt(void);
static bool UseCaseChildTask(CHILDMGR_Class_t *ChildMgr);
static const char *UseCaseStr(JMSG_DEMO_UseCaseTlm_Enum_t UseCase);


/**********************/
/** Global File Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  

static CFE_EVS_BinFilter_t  EventFilters[] =
{  
   /* Event ID                           Mask */
   {PKTUTIL_CSV_PARSE_ERR_EID,             CFE_EVS_FIRST_4_STOP},
   {MQTT_DEMO_CSV_TO_DISCRETE_TLM_EID,     CFE_EVS_FIRST_4_STOP},
   {MQTT_DEMO_CREATE_DISCRETE_CSV_CMD_EID, CFE_EVS_FIRST_4_STOP},
   {MQTT_DEMO_CSV_TO_RATE_TLM_EID,         CFE_EVS_FIRST_4_STOP},
   {MQTT_DEMO_CREATE_RATE_CSV_CMD_EID,     CFE_EVS_FIRST_4_STOP},
   {UDP_DEMO_CREATE_RPI_CSV_CMD_EID,       CFE_EVS_FIRST_4_STOP},
   {UDP_DEMO_CSV_TO_RPI_TLM_EID,           CFE_EVS_FIRST_4_STOP}
};

// See jmsg_demo.xml UseCase enumeration
static const char *UseCaseEnumStr[] = 
{
   "MQTT_DISCRETE",
   "MQTT_RATE",
   "UDP_RPI",
   "UDP_SCRIPT",
   "NONE" 
};


/*****************/
/** Global Data **/
/*****************/

JMSG_DEMO_APP_Class_t   JMsgDemoApp;


/******************************************************************************
** Function: JMSG_DEMO_AppMain
**
*/
void JMSG_DEMO_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;
   
   CFE_EVS_Register(EventFilters, sizeof(EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                    CFE_EVS_EventFilter_BINARY);

   if (InitApp() == CFE_SUCCESS)      /* Performs initial CFE_ES_PerfLogEntry() call */
   {
      RunStatus = CFE_ES_RunStatus_APP_RUN; 
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {
      
      /*
      ** The Rx and Tx child tasks manage translating and transferring the JSON
      ** messages. This loop only needs to service commands.
      */ 
      
      RunStatus = ProcessCommands();
      
   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("JMSG Demo App terminating, run status = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(JMSG_DEMO_APP_EXIT_EID, CFE_EVS_EventType_CRITICAL, "JMSG Demo App terminating, run status = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */


} /* End of JMSG_DEMO_AppMain() */



/******************************************************************************
** Function: JMSG_DEMO_APP_NoOpCmd
**
*/

bool JMSG_DEMO_APP_NoOpCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (JMSG_DEMO_APP_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for JMSG Demo App version %d.%d.%d",
                      JMSG_DEMO_APP_MAJOR_VER, JMSG_DEMO_APP_MINOR_VER, JMSG_DEMO_APP_PLATFORM_REV);

   return true;


} /* End JMSG_DEMO_APP_NoOpCmd() */


/******************************************************************************
** Function: JMSG_DEMO_APP_ResetAppCmd
**
*/

bool JMSG_DEMO_APP_ResetAppCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_ResetAllFilters();

   CMDMGR_ResetStatus(CMDMGR_OBJ);
   
   MQTT_DEMO_ResetStatus();
   UDP_DEMO_ResetStatus();
	  
   return true;

} /* End JMSG_DEMO_APP_ResetAppCmd() */


/******************************************************************************
** Function: MQTT_DEMO_APP_StartUseCase
**
** Notes:
**   None
**
*/
bool MQTT_DEMO_APP_StartUseCase(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{
         
   const JMSG_DEMO_StartUseCase_CmdPayload_t *Cmd = CMDMGR_PAYLOAD_PTR(MsgPtr, JMSG_DEMO_StartUseCase_t);
   bool RetStatus = false;
  
   if (Cmd->UseCase >= JMSG_DEMO_UseCaseCmd_Enum_t_MIN &&
       Cmd->UseCase <= JMSG_DEMO_UseCaseCmd_Enum_t_MAX)
   {
      
      if (JMsgDemoApp.UseCaseRunning)
      {
         CFE_EVS_SendEvent(JMSG_DEMO_APP_START_USE_CASE_EID, CFE_EVS_EventType_INFORMATION,
                           "Terminating use case %s", UseCaseStr(JMsgDemoApp.UseCase));           
      }

      CFE_EVS_SendEvent(JMSG_DEMO_APP_START_USE_CASE_EID, CFE_EVS_EventType_INFORMATION,
                        "Starting use case %s", UseCaseStr(Cmd->UseCase));           
    
      JMsgDemoApp.UseCase        = Cmd->UseCase;
      JMsgDemoApp.InitUseCase    = true;
      JMsgDemoApp.UseCaseRunning = true;
      
      RetStatus = true;
  
   } /* End if valid UseCase */
   else
   {
      CFE_EVS_SendEvent(JMSG_DEMO_APP_START_USE_CASE_EID, CFE_EVS_EventType_ERROR,
                        "Start use case command rejected, invalid use case %d",
                        Cmd->UseCase);       
   }
  
   return RetStatus; 

} /* End MQTT_DEMO_APP_StartUseCase() */


/******************************************************************************
** Function: MQTT_DEMO_APP_StopUseCase
**
** Notes:
**   1. If a use case is active it will be stopped.
**   2. It is not considered an error if no use case is running.  
**
*/
bool MQTT_DEMO_APP_StopUseCase(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{
   bool RetStatus = true;
  
   if (JMsgDemoApp.UseCaseRunning)
   {
      CFE_EVS_SendEvent(JMSG_DEMO_APP_STOP_USE_CASE_EID, CFE_EVS_EventType_INFORMATION,
                        "Stopped use case %s", UseCaseStr(JMsgDemoApp.UseCase));           
      
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_DEMO_APP_STOP_USE_CASE_EID, CFE_EVS_EventType_INFORMATION,
                        "Stop use case command received with no use case running");       
   }
    
   JMsgDemoApp.InitUseCase    = true;
   JMsgDemoApp.UseCaseRunning = false;
   JMsgDemoApp.UseCase        = JMSG_DEMO_UseCaseTlm_NONE;
  
   return RetStatus;    
   
} /* End MQTT_DEMO_APP_StopUseCase() */


/******************************************************************************
** Function: DispatchCsvTlmConverter
**
**  Calls the 
**
** Notes:
**   None
**
*/
static void DispatchCsvTlmConverter(const CFE_MSG_Message_t *TopicCsvCmd)
{
   
   if (JMsgDemoApp.UseCaseRunning)
   {
      switch(JMsgDemoApp.UseCase)
      {
         case JMSG_DEMO_UseCaseTlm_MQTT_DISCRETE:
            MQTT_DEMO_CsvToDiscreteTlm(TopicCsvCmd);
            break;
         case JMSG_DEMO_UseCaseTlm_MQTT_RATE:
            MQTT_DEMO_CsvToRateTlm(TopicCsvCmd);
            break;
         case JMSG_DEMO_UseCaseTlm_UDP_RPI:
            UDP_DEMO_CsvToRpiTlm(TopicCsvCmd);
            break;
         default:
            CFE_EVS_SendEvent(JMSG_DISPATCH_CSV_TLM_CONVERTER_EID, CFE_EVS_EventType_ERROR,
                              "Corrupted use case identifier. Value=%d. Terminating use case execution.", 
                              JMsgDemoApp.UseCase);
            JMsgDemoApp.UseCaseRunning = false;
            JMsgDemoApp.UseCase        = JMSG_DEMO_UseCaseTlm_NONE;
            break;                              
      }
   
   } /* End if use case running */

} /* End DispatchCsvTlmConverter() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 RetStatus = APP_C_FW_CFS_ERROR;
       
   CHILDMGR_TaskInit_t ChildTaskInit;  
      
   memset(&JMsgDemoApp, 0, sizeof(JMSG_DEMO_APP_Class_t));
   JMsgDemoApp.UseCase = JMSG_DEMO_UseCaseTlm_NONE;
   
   /*
   ** Read JSON INI Table & class variable defaults defined in JSON  
   */

   if (INITBL_Constructor(INITBL_OBJ, JMSG_DEMO_APP_INI_FILENAME, &IniCfgEnum))
   {
   
      JMsgDemoApp.PerfId = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_MAIN_PERF_ID);
      CFE_ES_PerfLogEntry(JMsgDemoApp.PerfId);

      JMsgDemoApp.InitUseCase = true;
      JMsgDemoApp.UseCaseTaskDelay = INITBL_GetIntConfig(INITBL_OBJ, CFG_USE_CASE_TASK_DELAY);
   
      MQTT_DEMO_Constructor(&(JMsgDemoApp.MqttDemo), INITBL_OBJ);
      UDP_DEMO_Constructor(&(JMsgDemoApp.UdpDemo), INITBL_OBJ);

      JMsgDemoApp.CmdMid             = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_DEMO_CMD_TOPICID));
      JMsgDemoApp.SendStatusMid      = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_SEND_STATUS_TLM_TOPICID));
      JMsgDemoApp.JmsgTopicCsvTlmMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, JMSG_LIB_TOPIC_CSV_TLM_TOPICID));

      /* Constructor sends error events */    
      ChildTaskInit.TaskName  = INITBL_GetStrConfig(INITBL_OBJ, CFG_CHILD_NAME);
      ChildTaskInit.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PERF_ID);
      ChildTaskInit.StackSize = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_STACK_SIZE);
      ChildTaskInit.Priority  = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PRIORITY);
      CHILDMGR_Constructor(CHILDMGR_OBJ, ChildMgr_TaskMainCallback, UseCaseChildTask, &ChildTaskInit);       
      /*
      ** Initialize app level interfaces
      */
 
      CFE_SB_CreatePipe(&JMsgDemoApp.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_APP_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(JMsgDemoApp.CmdMid, JMsgDemoApp.CmdPipe);
      CFE_SB_Subscribe(JMsgDemoApp.SendStatusMid, JMsgDemoApp.CmdPipe);
      CFE_SB_Subscribe(JMsgDemoApp.JmsgTopicCsvTlmMid, JMsgDemoApp.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_DEMO_NOOP_CC,  NULL, JMSG_DEMO_APP_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_DEMO_RESET_CC, NULL, JMSG_DEMO_APP_ResetAppCmd, 0);

      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_DEMO_START_USE_CASE_CC, NULL, MQTT_DEMO_APP_StartUseCase, sizeof(JMSG_DEMO_StartUseCase_CmdPayload_t));
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_DEMO_STOP_USE_CASE_CC,  NULL, MQTT_DEMO_APP_StopUseCase,  0);
      
      CFE_MSG_Init(CFE_MSG_PTR(JMsgDemoApp.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_DEMO_STATUS_TLM_TOPICID)), sizeof(JMSG_DEMO_StatusTlm_t));

      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(JMSG_DEMO_APP_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG Demo App Initialized. Version %d.%d.%d",
                        JMSG_DEMO_APP_MAJOR_VER, JMSG_DEMO_APP_MINOR_VER, JMSG_DEMO_APP_PLATFORM_REV);
                        
      RetStatus = CFE_SUCCESS;
      
   } /* End if INITBL Constructed */
   
   return RetStatus;

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
** 
*/
static int32 ProcessCommands(void)
{
   
   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t  *SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;


   CFE_ES_PerfLogExit(JMsgDemoApp.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, JMsgDemoApp.CmdPipe, CFE_SB_PEND_FOREVER);
   CFE_ES_PerfLogEntry(JMsgDemoApp.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);

      if (SysStatus == CFE_SUCCESS)
      {

         if (CFE_SB_MsgId_Equal(MsgId, JMsgDemoApp.CmdMid))
         {
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, JMsgDemoApp.SendStatusMid))
         {   
            SendStatusPkt();
         }
         else if (CFE_SB_MsgId_Equal(MsgId, JMsgDemoApp.JmsgTopicCsvTlmMid))
         {   
            DispatchCsvTlmConverter(&SbBufPtr->Msg);
         }
         else
         {   
            CFE_EVS_SendEvent(JMSG_DEMO_APP_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%04X(%d)", 
                              CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId));
         }

      } /* End if got message ID */
   } /* End if received buffer */
   else
   {
      if (SysStatus != CFE_SB_NO_MESSAGE)
      {
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
      }
   } 

   return RetStatus;
   
} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusPkt
**
*/
void SendStatusPkt(void)
{
   
   JMSG_DEMO_StatusTlm_Payload_t *Payload = &JMsgDemoApp.StatusTlm.Payload;

   /*
   ** Framework Data
   */

   Payload->ValidCmdCnt    = JMsgDemoApp.CmdMgr.ValidCmdCnt;
   Payload->InvalidCmdCnt  = JMsgDemoApp.CmdMgr.InvalidCmdCnt;

   /*
   ** JMSG Demo App
   */
   
   Payload->UseCaseRunning = JMsgDemoApp.UseCaseRunning;
   Payload->UseCase        = JMsgDemoApp.UseCase;
   
   /*
   ** MQTT Demo
   */
   
   Payload->MqttCreateDiscreteCsvCmdCnt = JMsgDemoApp.MqttDemo.CreateDiscreteCsvCmdCnt;
   Payload->MqttCsvToDiscreteCnt        = JMsgDemoApp.MqttDemo.CsvToDiscreteCnt;
   Payload->MqttCreateRateCsvCmdCnt     = JMsgDemoApp.MqttDemo.CreateRateCsvCmdCnt;
   Payload->MqttCsvToRateCnt            = JMsgDemoApp.MqttDemo.CsvToRateCnt;
   
   /*
   ** UDP Demo
   */
   
   Payload->UdpCreateRpiCnt = JMsgDemoApp.UdpDemo.CreateRpiCsvCmdCnt;
   Payload->UdpCsvToRpiCnt  = JMsgDemoApp.UdpDemo.CsvToRpiCnt;
       
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgDemoApp.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgDemoApp.StatusTlm.TelemetryHeader), true);

} /* End SendStatusPkt() */


/******************************************************************************
** Function: UseCaseChildTask
**
** Notes:
**   1. Returning false causes the child task to terminate.
**   2. Information events are sent because this is instructional code and the
**      events provide feedback. The events are filtered so they won't flood
**      the ground. A reset app command resets the event filter.  
**
*/
bool UseCaseChildTask(CHILDMGR_Class_t *ChildMgr)
{
   
   bool RetStatus = true;
   
   if (JMsgDemoApp.UseCaseRunning)
   {
      switch(JMsgDemoApp.UseCase)
      {
         case JMSG_DEMO_UseCaseTlm_MQTT_DISCRETE:
            MQTT_DEMO_CreateDiscreteCsvCmd(JMsgDemoApp.InitUseCase);
            break;
         case JMSG_DEMO_UseCaseTlm_MQTT_RATE:
            MQTT_DEMO_CreateRateCsvCmd(JMsgDemoApp.InitUseCase);
            break;
         case JMSG_DEMO_UseCaseTlm_UDP_RPI:
            UDP_DEMO_CreateRpiCsvCmd(JMsgDemoApp.InitUseCase);
            break;
         case JMSG_DEMO_UseCaseTlm_UDP_SCRIPT:
            UDP_DEMO_CreateScriptCmd(JMsgDemoApp.InitUseCase);
            break;

         default:
            CFE_EVS_SendEvent(JMSG_DEMO_CHILD_TASK_EID, CFE_EVS_EventType_ERROR,
                              "Corrupted use case identifier. Value=%d. Terminating use case execution.", 
                              JMsgDemoApp.UseCase);
            JMsgDemoApp.UseCaseRunning = false;
            JMsgDemoApp.UseCase        = JMSG_DEMO_UseCaseTlm_NONE;
            break;                              
      }
      JMsgDemoApp.InitUseCase = false;
   
   } /* End if use case running */
   
   OS_TaskDelay(JMsgDemoApp.UseCaseTaskDelay);   
   
   return RetStatus;

} /* End UseCaseChildTask() */


/******************************************************************************
** Function: UseCaseStr
**
** Purpose: Return a pointer to a string describing a use case
**
** Notes:
**   1. See jmsg_demo.xml UseCase enumeration
*/
static const char *UseCaseStr(JMSG_DEMO_UseCaseTlm_Enum_t UseCase)
{
   
   uint8 i = JMSG_DEMO_UseCaseTlm_NONE;
   
   if ( UseCase >= JMSG_DEMO_UseCaseCmd_Enum_t_MIN &&
        UseCase <= JMSG_DEMO_UseCaseCmd_Enum_t_MAX)
   {
   
      i = UseCase;
   
   }
        
   return UseCaseEnumStr[i];

} /* End UseCaseStr() */