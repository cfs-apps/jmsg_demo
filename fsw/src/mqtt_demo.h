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

#ifndef _mqtt_demo_
#define _mqtt_demo_

/*
** Includes
*/

#include "app_cfg.h"
#include "jmsg_platform_eds_defines.h"
#include "jmsg_lib_eds_interface.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_DEMO_CREATE_DISCRETE_CSV_CMD_EID  (MQTT_DEMO_BASE_EID + 0)
#define MQTT_DEMO_CSV_TO_DISCRETE_TLM_EID      (MQTT_DEMO_BASE_EID + 1)
#define MQTT_DEMO_CREATE_RATE_CSV_CMD_EID      (MQTT_DEMO_BASE_EID + 2)
#define MQTT_DEMO_CSV_TO_RATE_TLM_EID          (MQTT_DEMO_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


typedef enum 
{

  MQTT_DEMO_AXIS_X = 1,
  MQTT_DEMO_AXIS_Y = 2,
  MQTT_DEMO_AXIS_Z = 3
  
} MQTT_DEMO_TestAxis_t;


typedef struct
{

   JMSG_LIB_TopicCsvCmd_t        TopicCsvCmd;
   JMSG_DEMO_MqttDiscreteTlm_t   DiscreteTlm;
   JMSG_DEMO_MqttRateTlm_t       RateTlm;
      
   uint32  DiscreteTestData;
   uint32  CsvToDiscreteCnt;
   uint32  CreateDiscreteCsvCmdCnt;

   double               TestRate;
   double               TestRateAxisX;
   double               TestRateAxisY;
   double               TestRateAxisZ;
   MQTT_DEMO_TestAxis_t TestAxis;
   uint16               TestRateAxisCycleLim;  /* Number of execution cycles to perform on each axis */
   uint16               TestRateAxisCycleCnt;
   
   uint16  CsvToRateCnt;
   uint16  CreateRateCsvCmdCnt;
   uint16  CreateRateCsvPeriod;
   
} MQTT_DEMO_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_DEMO_Constructor
**
** Notes:
**   1. This must be called prior to any other member functions.
**
*/
void MQTT_DEMO_Constructor(MQTT_DEMO_Class_t *MqttDemoPtr, const INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: MQTT_DEMO_CreateDiscreteCsvCmd
**
** Notes:
**   1. Loads Discrete telemetry parameter fields from the JMSG and sends
**      the JMSG_DEMO Discrete message. The parameter order must match the
**      JMSG_DEMO_DiscreteParams_Enum_t definition.
**
*/
void MQTT_DEMO_CreateDiscreteCsvCmd(bool Init);


/******************************************************************************
** Function: MQTT_DEMO_CreateRateCsvCmd
**
** Create a JMSG_LIB_TopicCsvCmd message with the ParamText containing
** JMSG_DEMO's Rate telemetry.    
**
*/
void MQTT_DEMO_CreateRateCsvCmd(bool Init);


/******************************************************************************
** Function: MQTT_DEMO_CreateDiscreteCsvCmd
**
**  Loads Discrete telemetry parameter fields from the JMSG and sends the
**  JMSG_DEMO Discrete message.
**
** Notes:
**   1. The CSV parameter order must match the JMSG_DEMO_DiscreteParams_Enum_t
**      definition.
**
*/
bool MQTT_DEMO_CsvToDiscreteTlm(const CFE_MSG_Message_t *TopicCsvCmd);


/******************************************************************************
** Function: MQTT_DEMO_CsvToRateTlm
**
**  Loads Rate telemetry parameter fields from the JMSG and sends the
**  JMSG_DEMO Rate message.
**
** Notes:
**   1. The CSV parameter order must match the JMSG_DEMO_RateParams_Enum_t
**      definition.
**
*/
bool MQTT_DEMO_CsvToRateTlm(const CFE_MSG_Message_t *TopicCsvCmd);


/******************************************************************************
** Function: MQTT_DEMO_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_DEMO_ResetStatus(void);


#endif /* _mqtt_demo_ */
