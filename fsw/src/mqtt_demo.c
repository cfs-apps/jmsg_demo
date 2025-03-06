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

#include "mqtt_demo.h"
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

static MQTT_DEMO_Class_t *MqttDemo;


static JMSG_DEMO_MqttDiscreteTlm_Payload_t DiscreteTlm; /* Working buffer for loads */
static PKTUTIL_CSV_Entry_t JMsgDiscreteCsvEntry[] = 
{
   
   { &DiscreteTlm.Item1,   PKTUTIL_CSV_INTEGER, PKTUTIL_CSV_INT_LEN},
   { &DiscreteTlm.Item2,   PKTUTIL_CSV_INTEGER, PKTUTIL_CSV_INT_LEN},
   { &DiscreteTlm.Item3,   PKTUTIL_CSV_INTEGER, PKTUTIL_CSV_INT_LEN}

};


static JMSG_DEMO_MqttRateTlm_Payload_t RateTlm; /* Working buffer for loads */
static PKTUTIL_CSV_Entry_t JMsgRateCsvEntry[] = 
{
   
   { &RateTlm.RateX,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RateTlm.RateY,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN},
   { &RateTlm.RateZ,   PKTUTIL_CSV_FLOAT,   PKTUTIL_CSV_FLT_LEN}

};

      
/******************************************************************************
** Function: MQTT_DEMO_Constructor
**
** Notes:
**   1. This must be called prior to any other member functions.
**
*/
void MQTT_DEMO_Constructor(MQTT_DEMO_Class_t *MqttDemoPtr, const INITBL_Class_t *IniTbl)
{

   MqttDemo = MqttDemoPtr;
   
   memset(MqttDemo, 0, sizeof(MQTT_DEMO_Class_t));

   MqttDemo->CreateRateCsvPeriod = INITBL_GetIntConfig(IniTbl, CFG_USE_CASE_TASK_DELAY);

   // See jmsg_lib.xml comments for why TopicCsvCmd is defined as a telemetry message
   CFE_MSG_Init(CFE_MSG_PTR(MqttDemo->TopicCsvCmd.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_LIB_TOPIC_CSV_CMD_TOPICID)),
                sizeof(JMSG_LIB_TopicCsvCmd_t));
                
   CFE_MSG_Init(CFE_MSG_PTR(MqttDemo->DiscreteTlm.TelemetryHeader), 
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_DEMO_MQTT_DISCRETE_TLM_TOPICID)),
                sizeof(JMSG_DEMO_MqttRateTlm_t));

   CFE_MSG_Init(CFE_MSG_PTR(MqttDemo->RateTlm.TelemetryHeader),
                CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_JMSG_DEMO_MQTT_RATE_TLM_TOPICID)),
                sizeof(JMSG_DEMO_MqttRateTlm_t));
   
   
} /* End MQTT_DEMO_Constructor() */


/******************************************************************************
** Function: MQTT_DEMO_CreateDiscreteCsvCmd
**
** Create a JMSG_LIB_TopicCsvCmd message with the ParamText containing
** JMSG_DEMO's Discrete telemetry.  
**
** Notes:
**   1. Saying it's a command is a little wonky because it contains rate
**      telemetry data. However, this is for demonstration purposes.
**   2. A walking bit pattern is used in the discrete data to help validation.
**
*/
void MQTT_DEMO_CreateDiscreteCsvCmd(bool Init)
{
   char ParamText[JMSG_DEMO_CSV_PARAM_TEXT_LEN];
   
   if (Init)
   {
      MqttDemo->DiscreteTestData = 0; 
      strcpy(MqttDemo->TopicCsvCmd.Payload.Name,"MQTT Discrete");
      
      CFE_EVS_SendEvent(MQTT_DEMO_CREATE_DISCRETE_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                        "Started MQTT Discrete CSV Command use case");

   }
   else
   {
   
      sprintf(ParamText, "item-1,%d,item-2,%d,item-3,%d,item-4,%d",
              (MqttDemo->DiscreteTestData & 0x01), ((MqttDemo->DiscreteTestData & 0x02) >> 1),
              ((MqttDemo->DiscreteTestData & 0x04) >> 2), ((MqttDemo->DiscreteTestData & 0x08) >> 3));
      
      strcpy(MqttDemo->TopicCsvCmd.Payload.ParamText,ParamText);
      
      CFE_EVS_SendEvent(MQTT_DEMO_CREATE_DISCRETE_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                        "Sending MQTT Discrete CSV Command %s", ParamText);
                        
      if (++MqttDemo->DiscreteTestData > 15)
      {
         MqttDemo->DiscreteTestData = 0;
      }
      
   }
   
   MqttDemo->CreateDiscreteCsvCmdCnt++;
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttDemo->TopicCsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttDemo->TopicCsvCmd.TelemetryHeader), true);
   
} /* End MQTT_DEMO_CreateDiscreteCsvCmd() */


/******************************************************************************
** Function: MQTT_DEMO_CreateRateCsvCmd
**
** Create a JMSG_LIB_TopicCsvCmd message with the ParamText containing
** JMSG_DEMO's Rate telemetry.  
**
** Notes:
**   1. Saying it's a command is a little wonky because it contains rate
**      telemetry data. However, this is for demonstration purposes.
**
*/
void MQTT_DEMO_CreateRateCsvCmd(bool Init)
{

   char ParamText[JMSG_DEMO_CSV_PARAM_TEXT_LEN];
   double RateTest90DegPeriod = RATE_TEST_90_DEG_TIME/((double)MqttDemo->CreateRateCsvPeriod/1000.0);
   
   if (Init)
   {
      
      /*
      ** Goal is to rotate 90 degree per axis in a short enough time to keep the 
      ** user interested if they're watching a graphic
      ** - 9s for 90 degree rotation seems reasonable
      ** - Delta time = 9s / (child task delay / 1000)
      ** - Constant rate = 90 deg / Delta Time
      */

      MqttDemo->TestRate = (90.0*0.0174533)/RateTest90DegPeriod; 
      MqttDemo->TestRateAxisCycleLim = (int)RateTest90DegPeriod;

      MqttDemo->TestRateAxisX = 0.0;
      MqttDemo->TestRateAxisY = 0.0;
      MqttDemo->TestRateAxisZ = 0.0;
      // First cycle will transition from Z to X axis
      MqttDemo->TestAxis = MQTT_DEMO_AXIS_Z;
      MqttDemo->TestRateAxisCycleCnt = MqttDemo->TestRateAxisCycleLim; 
      
      strcpy(MqttDemo->TopicCsvCmd.Payload.Name,"MQTT Rate");
      
      CFE_EVS_SendEvent(MQTT_DEMO_CREATE_RATE_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                        "Started MQTT Rate CSV Command use case with rate %f and axis cycle limit %d",
                        MqttDemo->TestRate, MqttDemo->TestRateAxisCycleLim);

   }

   switch (MqttDemo->TestAxis)
   {
      case MQTT_DEMO_AXIS_X:
         if (++MqttDemo->TestRateAxisCycleCnt > MqttDemo->TestRateAxisCycleLim)
         {
            MqttDemo->TestRateAxisCycleCnt = 0;
            MqttDemo->TestRateAxisX = 0.0;
            MqttDemo->TestRateAxisY = MqttDemo->TestRate;
            MqttDemo->TestAxis      = MQTT_DEMO_AXIS_Y;
         }
         break;
      case MQTT_DEMO_AXIS_Y:
         if (++MqttDemo->TestRateAxisCycleCnt > MqttDemo->TestRateAxisCycleLim)
         {
            MqttDemo->TestRateAxisCycleCnt = 0;
            MqttDemo->TestRateAxisY = 0.0;
            MqttDemo->TestRateAxisZ = MqttDemo->TestRate;
            MqttDemo->TestAxis      = MQTT_DEMO_AXIS_Z;
         }
         break;
      case MQTT_DEMO_AXIS_Z:
         if (++MqttDemo->TestRateAxisCycleCnt > MqttDemo->TestRateAxisCycleLim)
         {
            MqttDemo->TestRateAxisCycleCnt = 0;
            MqttDemo->TestRateAxisZ = 0.0;
            MqttDemo->TestRateAxisX = MqttDemo->TestRate;
            MqttDemo->TestAxis      = MQTT_DEMO_AXIS_X;
         }
         break;
      default:
         MqttDemo->TestRateAxisCycleCnt = 0;
         MqttDemo->TestRateAxisX = MqttDemo->TestRate;
         MqttDemo->TestRateAxisY = 0.0;
         MqttDemo->TestRateAxisZ = 0.0;
         MqttDemo->TestAxis      = MQTT_DEMO_AXIS_X;
         break;
      
   } /* End axis switch */
     
   sprintf(ParamText, "rate-x,%0.5f,rate-y,%0.5f,rate-z,%0.5f",
           MqttDemo->TestRateAxisX,MqttDemo->TestRateAxisY,MqttDemo->TestRateAxisZ);
   
   strcpy(MqttDemo->TopicCsvCmd.Payload.ParamText,ParamText);
   
   CFE_EVS_SendEvent(MQTT_DEMO_CREATE_RATE_CSV_CMD_EID, CFE_EVS_EventType_INFORMATION,
                     "Sending MQTT Rate CSV Command %s", ParamText);
     
   MqttDemo->CreateRateCsvCmdCnt++;
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttDemo->TopicCsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttDemo->TopicCsvCmd.TelemetryHeader), true);

} /* End MQTT_DEMO_CreateRateCsvCmd() */


/******************************************************************************
** Function: MQTT_DEMO_CsvToDiscreteTlm
**
** Notes:
**   1. Loads Discrete telemetry parameter fields from the JMSG and sends
**      the JMSG_DEMO Discrete message. The parameter order must match the
**      JMSG_DEMO_DiscreteParams_Enum_t definition.
**   2. The LocalPayload variable is needed because PktUtil_ParseCsvStr()
**      modifies the CSV parameter text.
**
*/
bool MQTT_DEMO_CsvToDiscreteTlm(const CFE_MSG_Message_t *TopicCsvCmd)
{
   
   const JMSG_LIB_TopicCsvCmd_Payload_t *JMsgPayload = CMDMGR_PAYLOAD_PTR(TopicCsvCmd, JMSG_LIB_TopicCsvCmd_t);   

   bool   RetStatus = false;
   int    CsvEntries;
   JMSG_LIB_TopicCsvCmd_Payload_t LocalPayload;
   
   memcpy(&LocalPayload, JMsgPayload, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));

   CsvEntries = PktUtil_ParseCsvStr(LocalPayload.ParamText, JMsgDiscreteCsvEntry, JMSG_DEMO_DiscreteParams_Enum_t_MAX);
   
   if (CsvEntries == JMSG_DEMO_DiscreteParams_Enum_t_MAX)
   {
      memcpy(&MqttDemo->DiscreteTlm.Payload, &DiscreteTlm,  sizeof(JMSG_DEMO_MqttDiscreteTlm_Payload_t));
      CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttDemo->DiscreteTlm.TelemetryHeader));
      CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttDemo->DiscreteTlm.TelemetryHeader), true);
      MqttDemo->CsvToDiscreteCnt++;
      RetStatus = true;
   } 
   else
   {
      CFE_EVS_SendEvent(MQTT_DEMO_CSV_TO_DISCRETE_TLM_EID, CFE_EVS_EventType_ERROR, 
                        "Incorrect number of discrete parameters. Received %d expected %d", 
                        CsvEntries, JMSG_DEMO_DiscreteParams_Enum_t_MAX);         
   }
   
   return RetStatus;  
   
} /* End MQTT_DEMO_CsvToDiscreteTlm() */


/******************************************************************************
** Function: MQTT_DEMO_CsvToRateTlm
**
** Notes:
**   1. Loads Rate telemetry parameter fields from the JMSG and sends
**      the JMSG_DEMO Rate message. The parameter order must match the
**      JMSG_DEMO_RateParams_Enum_t definition.
**   2. The LocalPayload variable is needed because PktUtil_ParseCsvStr()
**      modifies the CSV parameter text.
**
*/
bool MQTT_DEMO_CsvToRateTlm(const CFE_MSG_Message_t *TopicCsvCmd)
{
   
   const JMSG_LIB_TopicCsvCmd_Payload_t *JMsgPayload = CMDMGR_PAYLOAD_PTR(TopicCsvCmd, JMSG_LIB_TopicCsvCmd_t);   

   bool   RetStatus = false;
   int    CsvEntries;
   JMSG_LIB_TopicCsvCmd_Payload_t LocalPayload;
   
   memcpy(&LocalPayload, JMsgPayload, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));

   CsvEntries = PktUtil_ParseCsvStr(LocalPayload.ParamText, JMsgRateCsvEntry, JMSG_DEMO_RateParams_Enum_t_MAX);
   
   if (CsvEntries == JMSG_DEMO_RateParams_Enum_t_MAX)
   {
      memcpy(&MqttDemo->RateTlm.Payload, &RateTlm,  sizeof(JMSG_DEMO_MqttRateTlm_Payload_t));
      CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttDemo->RateTlm.TelemetryHeader));
      CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttDemo->RateTlm.TelemetryHeader), true);
      MqttDemo->CsvToRateCnt++;
      RetStatus = true;
   } 
   else
   {
      CFE_EVS_SendEvent(MQTT_DEMO_CSV_TO_RATE_TLM_EID, CFE_EVS_EventType_ERROR, 
                        "Incorrect number of rate parameters. Received %d expected %d", 
                        CsvEntries, JMSG_DEMO_RateParams_Enum_t_MAX);         
   }
   
   return RetStatus;  
   
} /* End MQTT_DEMO_CsvToRateTlm() */


/******************************************************************************
** Function: MQTT_DEMO_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_DEMO_ResetStatus(void)
{

   MqttDemo->CsvToDiscreteCnt        = 0;
   MqttDemo->CreateDiscreteCsvCmdCnt = 0;

   MqttDemo->CsvToRateCnt        = 0;
   MqttDemo->CreateRateCsvCmdCnt = 0;
   
} /* End MQTT_DEMO_ResetStatus() */