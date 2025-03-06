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
**   Provide examples for how an app can use the JMSG_LIB_TOPIC_CSV_CMD
**   and JMSG_LIB_TOPIC_CSV_TLM_TOPICID to customize the command and
**   telmeetry CSV parameters
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "udp_demo.h"
#include "jmsg_lib_eds_typedefs.h"
#include "jmsg_platform_eds_defines.h"

/***********************/
/** Macro Definitions **/
/***********************/

#define RATE_TEST_90_DEG_TIME  9.0   /* Number of seconds to rotate 90 degrees */

/**********************/
/** Type Definitions **/
/**********************/


/********************************** **/
/** Local File Function Prototypes **/
/************************************/


/**********************/
/** File Global Data **/
/**********************/

static UDP_DEMO_Class_t *UdpDemo;


static JMSG_DEMO_UdpRpiTlm_Payload_t RpiTlm; /* Working buffer for loads */
static PKTUTIL_CSV_Entry_t JMsgRpiCsvEntry[] = 
{
   { &RpiTlm.RateX,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.RateY,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.RateZ,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.Lux,     PKTUTIL_CSV_INTEGER, PKTUTIL_CSV_INT_LEN}
};

static char TestPyScript[] = "print('Hello World')\\nprint('Hello Universe')"; 
      
/******************************************************************************
** Function: UDP_DEMO_Constructor
**
** Notes:
**   1. This must be called prior to any other member functions.
**
*/
void UDP_DEMO_Constructor(UDP_DEMO_Class_t *UdpDemoPtr, const INITBL_Class_t *IniTbl)
{

   UdpDemo = UdpDemoPtr;
   
   memset(UdpDemo, 0, sizeof(UDP_DEMO_Class_t));
   
   UdpDemo->CreateRpiCsvPeriod = INITBL_GetIntConfig(IniTbl, CFG_USE_CASE_TASK_DELAY);
   
   // See jmsg_lib.xml comments for why TopicCsvCmd is defined as a telemetry message
   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_LIB_TOPIC_CSV_CMD_TOPICID)),
                sizeof(JMSG_LIB_TopicCsvCmd_t));

   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_LIB_TOPIC_SCRIPT_CMD_TOPICID)),
                sizeof(JMSG_LIB_TopicScriptCmd_t));
                
   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_DEMO_UDP_RPI_TLM_TOPICID)),
                sizeof(JMSG_DEMO_UdpRpiTlm_t));
   
} /* End UDP_DEMO_Constructor() */


/******************************************************************************
** Function: UDP_DEMO_CreateRpiCsvCmd
**
** Create a JMSG_LIB_TopicCsvCmd message with the ParamText containing
** JMSG_DEMO's RPI telemetry.  
**
** Notes:
**   1. Saying it's a command is a little wonky because it contains rate
**      telemetry data. However, this is for demonstration purposes.
**
*/
void UDP_DEMO_CreateRpiCsvCmd(bool Init)
{

   char ParamText[JMSG_DEMO_CSV_PARAM_TEXT_LEN];
   double RateTest90DegPeriod = RATE_TEST_90_DEG_TIME/((double)UdpDemo->CreateRpiCsvPeriod/1000.0);
      
   if (Init)
   {
      
      /*
      ** Goal is to rotate 90 degree per axis in a short enough time to keep the 
      ** user interested if they're watching a graphic
      ** - 9s for 90 degree rotation seems reasonable
      ** - Delta time = 9s / (child task delay / 1000)
      ** - Constant rate = 90 deg / Delta Time
      */
      
      UdpDemo->TestRate = (90*0.0174533)/RateTest90DegPeriod; 
      UdpDemo->TestRateAxisCycleLim = (int)RateTest90DegPeriod;

      UdpDemo->TestRateAxisX = 0.0;
      UdpDemo->TestRateAxisY = 0.0;
      UdpDemo->TestRateAxisZ = 0.0;
      UdpDemo->TestLux       = 0;
      
      // First cycle will transition from Z to X axis
      UdpDemo->TestAxis = UDP_DEMO_AXIS_Z;
      UdpDemo->TestRateAxisCycleCnt = UdpDemo->TestRateAxisCycleLim; 
      
      strcpy(UdpDemo->TopicCsvCmd.Payload.Name,"UDP RPI");
      
      CFE_EVS_SendEvent(UDP_DEMO_CREATE_RPI_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                        "Started UDP RPI CSV Command use case with rate %6.2f and axis cycle limit %d",
                        UdpDemo->TestRate, UdpDemo->TestRateAxisCycleLim);

   }

   switch (UdpDemo->TestAxis)
   {
      case UDP_DEMO_AXIS_X:
         if (++UdpDemo->TestRateAxisCycleCnt > UdpDemo->TestRateAxisCycleLim)
         {
            UdpDemo->TestRateAxisCycleCnt = 0;
            UdpDemo->TestRateAxisX = 0.0;
            UdpDemo->TestRateAxisY = UdpDemo->TestRate;
            UdpDemo->TestAxis      = UDP_DEMO_AXIS_Y;
         }
         break;
      case UDP_DEMO_AXIS_Y:
         if (++UdpDemo->TestRateAxisCycleCnt > UdpDemo->TestRateAxisCycleLim)
         {
            UdpDemo->TestRateAxisCycleCnt = 0;
            UdpDemo->TestRateAxisY = 0.0;
            UdpDemo->TestRateAxisZ = UdpDemo->TestRate;
            UdpDemo->TestAxis      = UDP_DEMO_AXIS_Z;
         }
         break;
      case UDP_DEMO_AXIS_Z:
         if (++UdpDemo->TestRateAxisCycleCnt > UdpDemo->TestRateAxisCycleLim)
         {
            UdpDemo->TestRateAxisCycleCnt = 0;
            UdpDemo->TestRateAxisZ = 0.0;
            UdpDemo->TestRateAxisX = UdpDemo->TestRate;
            UdpDemo->TestAxis      = UDP_DEMO_AXIS_X;
         }
         break;
      default:
         UdpDemo->TestRateAxisCycleCnt = 0;
         UdpDemo->TestRateAxisX = UdpDemo->TestRate;
         UdpDemo->TestRateAxisY = 0.0;
         UdpDemo->TestRateAxisZ = 0.0;
         UdpDemo->TestAxis      = UDP_DEMO_AXIS_X;
         break;
      
   } /* End axis switch */
   UdpDemo->TestLux = UdpDemo->TestRateAxisCycleCnt;
   
   sprintf(ParamText, "rate-x,%0.5f,rate-y,%0.5f,rate-z,%0.5f,lux,%d",
           UdpDemo->TestRateAxisX,UdpDemo->TestRateAxisY,UdpDemo->TestRateAxisZ,UdpDemo->TestLux);
   
   strcpy(UdpDemo->TopicCsvCmd.Payload.ParamText,ParamText);
   
   CFE_EVS_SendEvent(UDP_DEMO_CREATE_RPI_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                     "Sending UDP RPI CSV Command %s", ParamText);
     
   UdpDemo->CreateRpiCsvCmdCnt++;
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader), true);

} /* End UDP_DEMO_CreateRpiCsvCmd() */


/******************************************************************************
** Function: UDP_DEMO_CreateScriptCmd
**
** Send a python script to be executed
**
** Notes:
**   None
**
*/
bool UDP_DEMO_CreateScriptCmd(bool Init)
{
   JMSG_LIB_TopicScriptCmd_Payload_t *Payload = &UdpDemo->TopicScriptCmd.Payload;
     
   Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_TEXT;

   memset(Payload->ScriptFile, 0, OS_MAX_PATH_LEN);
   memset(Payload->ScriptText, 0, JMSG_PLATFORM_TOPIC_STRING_MAX_LEN);

   strcpy(Payload->ScriptFile, "Unused");
   strcpy(Payload->ScriptText, TestPyScript);
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader), true);

   CFE_EVS_SendEvent(UDP_DEMO_CREATE_SCRIPT_CMD_EID, CFE_EVS_EventType_INFORMATION,
                     "Sucessfully sent test python script command");

   return true;   
   
} /* UDP_DEMO_CreateScriptCmd() */


/******************************************************************************
** Function: UDP_DEMO_CsvToRpiTlm
**
** Notes:
**   1. Loads Rate telemetry parameter fields from the JMSG and sends
**      the JMSG_DEMO Rate message. The parameter order must match the
**      JMSG_DEMO_RateParams_Enum_t definition.
**   2. The LocalPayload variable is needed because PktUtil_ParseCsvStr()
**      modifies the CSV parameter text.
**
*/
bool UDP_DEMO_CsvToRpiTlm(const CFE_MSG_Message_t *TopicCsvCmd)
{
   
   const JMSG_LIB_TopicCsvCmd_Payload_t *JMsgPayload = CMDMGR_PAYLOAD_PTR(TopicCsvCmd, JMSG_LIB_TopicCsvCmd_t);   

   bool   RetStatus = false;
   int    CsvEntries;
   JMSG_LIB_TopicCsvCmd_Payload_t LocalPayload;
   
   memcpy(&LocalPayload, JMsgPayload, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));

   CsvEntries = PktUtil_ParseCsvStr(LocalPayload.ParamText, JMsgRpiCsvEntry, JMSG_DEMO_RateParams_Enum_t_MAX);
   
   if (CsvEntries == JMSG_DEMO_RpiParams_Enum_t_MAX)
   {
      memcpy(&UdpDemo->RpiTlm.Payload, &RpiTlm,  sizeof(JMSG_DEMO_UdpRpiTlm_Payload_t));
      CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader));
      CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader), true);
      UdpDemo->CsvToRpiCnt++;
      RetStatus = true;
   } 
   else
   {
      CFE_EVS_SendEvent(UDP_DEMO_CSV_TO_RPI_TLM_EID, CFE_EVS_EventType_ERROR, 
                        "Incorrect number of rate parameters. Received %d expected %d", 
                        CsvEntries, JMSG_DEMO_RpiParams_Enum_t_MAX);         
   }
   
   return RetStatus;  
   
} /* End UDP_DEMO_CsvToRpiTlm() */


/******************************************************************************
** Function: UDP_DEMO_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void UDP_DEMO_ResetStatus(void)
{

   UdpDemo->CreateRpiCsvCmdCnt = 0;
   UdpDemo->CsvToRpiCnt        = 0;

} /* End UDP_DEMO_ResetStatus() */
