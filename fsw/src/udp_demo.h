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
**   1. This 
**
*/

#ifndef _udp_demo_
#define _udp_demo_

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

#define UDP_DEMO_CREATE_RPI_CSV_CMD_EID  (UDP_DEMO_BASE_EID + 0)
#define UDP_DEMO_CSV_TO_RPI_TLM_EID      (UDP_DEMO_BASE_EID + 1)
#define UDP_DEMO_CREATE_SCRIPT_CMD_EID   (UDP_DEMO_BASE_EID + 2)


/**********************/
/** Type Definitions **/
/**********************/


typedef enum 
{

  UDP_DEMO_AXIS_X = 1,
  UDP_DEMO_AXIS_Y = 2,
  UDP_DEMO_AXIS_Z = 3
  
} UDP_DEMO_TestAxis_t;

typedef struct
{

   JMSG_LIB_TopicCsvCmd_t     TopicCsvCmd;
   JMSG_LIB_TopicScriptCmd_t  TopicScriptCmd;
   JMSG_DEMO_UdpRpiTlm_t      RpiTlm;

   double              TestRate;
   double              TestRateAxisX;
   double              TestRateAxisY;
   double              TestRateAxisZ;
   UDP_DEMO_TestAxis_t TestAxis;
   uint16              TestRateAxisCycleLim;  /* Number of execution cycles to perform on each axis */
   uint16              TestRateAxisCycleCnt;
   uint16              TestLux;
   
   uint16  CsvToRpiCnt;
   uint16  CreateRpiCsvCmdCnt;
   uint16  CreateRpiCsvPeriod;
   
} UDP_DEMO_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: UDP_DEMO_Constructor
**
** Notes:
**   1. This must be called prior to any other member functions.
**
*/
void UDP_DEMO_Constructor(UDP_DEMO_Class_t *UdpDemoPtr, const INITBL_Class_t *IniTbl);


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
void UDP_DEMO_CreateRpiCsvCmd(bool Init);


/******************************************************************************
** Function: UDP_DEMO_CreateScriptCmd
**
** Send a python script to be executed
**
** Notes:
**   None
**
*/
bool UDP_DEMO_CreateScriptCmd(bool Init);


/******************************************************************************
** Function: UDP_DEMO_CsvToRpiTlm
**
**  Loads RPI telemetry parameter fields from the JMSG and sends the
**  JMSG_DEMO RPI message.
**
** Notes:
**   1. The CSV parameter order must match the JMSG_DEMO_RpiParams_Enum_t
**      definition.
**
*/
bool UDP_DEMO_CsvToRpiTlm(const CFE_MSG_Message_t *TopicCsvCmd);


/******************************************************************************
** Function: UDP_DEMO_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void UDP_DEMO_ResetStatus(void);


#endif /* _udp_demo_ */
