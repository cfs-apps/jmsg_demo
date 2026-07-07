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
**   telemetry CSV parameters
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
   { &RpiTlm.DeltaT,  PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.RateX,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.RateY,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.RateZ,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RpiTlm.Lux,     PKTUTIL_CSV_INTEGER, PKTUTIL_CSV_INT_LEN}
};

// Need to escape \n because encapsulated in JSON
static char TestPyScriptCmds[] = "print('Line 1: From udp_demo.c')\\nprint('Line 2: From udp_demo.c')"; 
static char TestPyScriptFile[] = "local_script_demo.py";


/******************************************************************************
** Function: UDP_DEMO_Constructor
**
** Notes:
**   1. This must be called prior to any other member functions.
**
*/
void UDP_DEMO_Constructor(UDP_DEMO_Class_t *UdpDemoPtr, const INITBL_Class_t *IniTbl)
{
   
   uint16 RpiTlmParamEntries = sizeof(JMsgRpiCsvEntry)/sizeof(PKTUTIL_CSV_Entry_t);
   
   UdpDemo = UdpDemoPtr;
   
   memset(UdpDemo, 0, sizeof(UDP_DEMO_Class_t));
   
   // See jmsg_lib.xml comments for why Topic Commands are defined as a telemetry message
   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_LIB_TOPIC_CSV_CMD_TOPICID)),
                sizeof(JMSG_LIB_TopicCsvCmd_t));

   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_LIB_TOPIC_SCRIPT_CMD_TOPICID)),
                sizeof(JMSG_LIB_TopicScriptCmd_t));
                
   CFE_MSG_Init(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_DEMO_UDP_RPI_TLM_TOPICID)),
                sizeof(JMSG_DEMO_UdpRpiTlm_t));
   
   UdpDemo->RpiTlmParamEntries = JMSG_DEMO_RpiTlmParams_Enum_t_MAX+1;
   if (UdpDemo->RpiTlmParamEntries != RpiTlmParamEntries)
   {
      CFE_EVS_SendEvent(UDP_DEMO_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR,
                        "EDS RPI telemetry parameter count %d does not match udp_demo.c count %d", 
                        UdpDemo->RpiTlmParamEntries, RpiTlmParamEntries);      
   }
    
} /* End UDP_DEMO_Constructor() */


/******************************************************************************
** Function: UDP_DEMO_CreateRpiCsvCmd
**
** Create a JMSG_LIB_TopicCsvCmd message
**
** Notes:
**   1. TopicRpiCsvCnt is sent as a command parameter to have a
**      changing value
**
*/
void UDP_DEMO_CreateRpiCsvCmd(bool Init)
{

   char ParamText[JMSG_DEMO_CSV_CMD_PARAM_TEXT_LEN];
   JMSG_LIB_TopicCsvCmd_Payload_t *Payload = &UdpDemo->TopicCsvCmd.Payload;  
   
   if (Init)
   {
      UdpDemo->CreateRpiCsvCmdCnt = 1;
      strcpy(Payload->Name,"UDP RPI");
   }

   memset(Payload->ParamText, 0, JMSG_PLATFORM_JMSG_PAYLOAD_STRING_MAX_LEN);
   sprintf(ParamText,"\"cmd_code\": 0, \"cmd_param_1\": %d",UdpDemo->CreateRpiCsvCmdCnt);
   strcpy(Payload->ParamText, ParamText);
   
   CFE_EVS_SendEvent(UDP_DEMO_CREATE_RPI_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                     "Sending UDP RPI CSV command %s", ParamText);
     
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->TopicCsvCmd.TelemetryHeader), true);

   UdpDemo->CreateRpiCsvCmdCnt++;
                        
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

   char ScriptText[256];
   JMSG_LIB_TopicScriptCmd_Payload_t *Payload = &UdpDemo->TopicScriptCmd.Payload;
   
   if (Init)
   {
      UdpDemo->CreateScriptCmdCnt = 1;
   }
 
   memset(Payload->ScriptFile, 0, OS_MAX_PATH_LEN);
   memset(Payload->ScriptText, 0, JMSG_PLATFORM_JMSG_PAYLOAD_STRING_MAX_LEN);
   
   if (UdpDemo->CreateScriptCmdCnt % 2)
   {
      Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_TEXT;
      sprintf(ScriptText,"print('Demo cycle %d')\\n%s",UdpDemo->CreateScriptCmdCnt,TestPyScriptCmds);
      strcpy(Payload->ScriptFile, "Unused");
      strcpy(Payload->ScriptText, ScriptText);
   }
   else
   {
      Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_FILE;
      strcpy(Payload->ScriptFile, TestPyScriptFile);
      strcpy(Payload->ScriptText, "Unused");
   }

   CFE_EVS_SendEvent(UDP_DEMO_CREATE_SCRIPT_CMD_EID, CFE_EVS_EventType_INFORMATION,
                     "Sending UDP python script command");
                     
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->TopicScriptCmd.TelemetryHeader), true);

   UdpDemo->CreateScriptCmdCnt++;

   return true;   
   
} /* UDP_DEMO_CreateScriptCmd() */


/******************************************************************************
** Function: UDP_DEMO_CsvToRpiTlm
**
** Notes:
**   1. Loads Rate telemetry parameter fields from the JMSG and sends
**      the JMSG_DEMO Rate message. The parameter order must match the
**      JMSG_DEMO_RpiTlmParams_Enum_t definition.
**   2. The LocalPayload variable is needed because PktUtil_ParseCsvStr()
**      modifies the CSV parameter text.
**
*/
bool UDP_DEMO_CsvToRpiTlm(const CFE_MSG_Message_t *TopicCsvTlm)
{
   
   const JMSG_LIB_TopicCsvTlm_Payload_t *JMsgPayload = CMDMGR_PAYLOAD_PTR(TopicCsvTlm, JMSG_LIB_TopicCsvTlm_t);   

   bool   RetStatus = false;
   int    CsvEntries;
   JMSG_LIB_TopicCsvTlm_Payload_t LocalPayload;


   CFE_EVS_SendEvent(UDP_DEMO_CSV_TO_RPI_TLM_EID, CFE_EVS_EventType_INFORMATION, 
                     "UDP_DEMO_CsvToRpiTlm() - Name: %s, Param: %s", 
                     JMsgPayload->Name, JMsgPayload->ParamText);         
   
   memcpy(&LocalPayload, JMsgPayload, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));

   CsvEntries = PktUtil_ParseCsvStr(LocalPayload.ParamText, JMsgRpiCsvEntry, UdpDemo->RpiTlmParamEntries);

   if (CsvEntries == UdpDemo->RpiTlmParamEntries)
   {
      memcpy(&UdpDemo->RpiTlm.Payload, &RpiTlm,  sizeof(JMSG_DEMO_UdpRpiTlm_Payload_t));
      CFE_SB_TimeStampMsg(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader));
      CFE_SB_TransmitMsg(CFE_MSG_PTR(UdpDemo->RpiTlm.TelemetryHeader), true);
      UdpDemo->CsvToRpiTlmCnt++;
      RetStatus = true;
   } 
   else
   {
      CFE_EVS_SendEvent(UDP_DEMO_CSV_TO_RPI_TLM_EID, CFE_EVS_EventType_ERROR, 
                        "Incorrect number of RPI CSV telemetry parameters. Received %d expected %d", 
                        CsvEntries, UdpDemo->RpiTlmParamEntries);         
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
   UdpDemo->CreateScriptCmdCnt = 0;
   UdpDemo->CsvToRpiTlmCnt     = 0;

} /* End UDP_DEMO_ResetStatus() */
