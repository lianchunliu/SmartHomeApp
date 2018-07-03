/******************************************************************************
  Filename:       SmartHomeApp.c
  Revised:        $Date: 2012-03-07 01:04:58 -0800 (Wed, 07 Mar 2012) $
  Revision:       $Revision: 29656 $

  Description:    Generic Application (no Profile).


  Copyright 2004-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
******************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends "Hello World" to another "Generic"
  application every 5 seconds.  The application will also
  receives "Hello World" packets.

  The "Hello World" messages are sent/received as MSG type message.

  This applications doesn't have a profile, so it handles everything
  directly - itself.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "SmartHomeApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

#include "user_printf.h"
#include "Utils.h"

#include "DefaultProcessor.h"
#include "DoorLight.h"
#include "DoorSensor.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// This list should be filled with Application specific Cluster IDs.
const cId_t SmartHomeApp_ClusterList[SmartHomeApp_MAX_CLUSTERS] =
{
  SmartHomeApp_CLUSTERID
};

const SimpleDescriptionFormat_t SmartHomeApp_SimpleDesc =
{
  SmartHomeApp_ENDPOINT,              //  int Endpoint;
  SmartHomeApp_PROFID,                //  uint16 AppProfId[2];
  SmartHomeApp_DEVICEID,              //  uint16 AppDeviceId[2];
  SmartHomeApp_DEVICE_VERSION,        //  int   AppDevVer:4;
  SmartHomeApp_FLAGS,                 //  int   AppFlags:4;
  SmartHomeApp_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SmartHomeApp_ClusterList,  //  byte *pAppInClusterList;
  SmartHomeApp_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SmartHomeApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SmartHomeApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SmartHomeApp_epDesc;

#define SERIAL_APP_TX_MAX 64
uint8 RxBuf[SERIAL_APP_TX_MAX+1];
static uint8 SerialApp_TxLen;
static uint16 SmartHomeApp_LastSendAddr;
static uint16 SmartHomeApp_LastSendTransID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte SmartHomeApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SmartHomeApp_Init() is called.
devStates_t SmartHomeApp_NwkState;


byte SmartHomeApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SmartHomeApp_DstAddr;

void (*Processor_HandleKeys)(uint8 keys);
void (*Processor_HandleMsg)(uint8* msg);

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void SmartHomeApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
static void SmartHomeApp_HandleKeys( byte shift, byte keys );
static void SmartHomeApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
//static void SmartHomeApp_SendTheMessage( void );



/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SmartHomeApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SmartHomeApp_Init( uint8 task_id )
{
  SmartHomeApp_TaskID = task_id;
  SmartHomeApp_NwkState = DEV_INIT;
  SmartHomeApp_TransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().
  
  halUARTCfg_t uartConfig;
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = HAL_UART_BR_115200;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 48;
  uartConfig.rx.maxBufSize        = 128;
  uartConfig.tx.maxBufSize        = 128;
  uartConfig.idleTimeout          = 6;   
  uartConfig.intEnable            = TRUE;              
  uartConfig.callBackFunc         = SmartHomeApp_CallBack;
  HalUARTOpen (HAL_UART_PORT_0, &uartConfig);
  
 // uint8 msg[] = "hello,world!";
//  HalUARTWrite(HAL_UART_PORT_0, msg, sizeof(msg));
  
  printf("My name is %s\n", Util_ReadName());
  
  if (Util_StrEqual((uint8*)"DoorLight", Util_ReadName())) {    
    Processor_HandleKeys = &DoorLight_HandleKeys;
    Processor_HandleMsg = &DoorLight_HandleMsg;
  } else if (Util_StrEqual((uint8*)"DoorSensor", Util_ReadName())) {    
    Processor_HandleKeys = &DoorSensor_HandleKeys;
    Processor_HandleMsg = &DoorSensor_HandleMsg;
  } else {
    printf("ERROR: Unkown name: %s\n", Util_ReadName());
    Processor_HandleKeys = &DefaultProcessor_HandleKeys;
    Processor_HandleMsg = &DefaultProcessor_HandleMsg;
  }
  
  
  SmartHomeApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  SmartHomeApp_DstAddr.endPoint = SmartHomeApp_ENDPOINT;
  SmartHomeApp_DstAddr.addr.shortAddr = 0;

  
  
  // Fill out the endpoint description.
  SmartHomeApp_epDesc.endPoint = SmartHomeApp_ENDPOINT;
  SmartHomeApp_epDesc.task_id = &SmartHomeApp_TaskID;
  SmartHomeApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SmartHomeApp_SimpleDesc;
  SmartHomeApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SmartHomeApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SmartHomeApp_TaskID );

  // Update the display
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SmartHomeApp", HAL_LCD_LINE_1 );
#endif

  ZDO_RegisterForZDOMsg( SmartHomeApp_TaskID, End_Device_Bind_rsp );
  ZDO_RegisterForZDOMsg( SmartHomeApp_TaskID, Match_Desc_rsp );

}


void SmartHome_SendCmd(uint8* buf)
{
  
  uint16 destAddr;
  
  destAddr =  NameAddrCache_getAddrFromCmd(buf);
  
  if (destAddr == 0xFFFF) {
    SmartHomeApp_DstAddr.addrMode = afAddrBroadcast;
  } else {
    SmartHomeApp_DstAddr.addrMode = afAddr16Bit;
  }
 
  SmartHomeApp_DstAddr.addr.shortAddr=destAddr;
  
  SmartHomeApp_LastSendAddr = destAddr;
  SmartHomeApp_LastSendTransID = SmartHomeApp_TransID;
    
  SmartHomeApp_DstAddr.endPoint = SmartHomeApp_ENDPOINT;
  
  if ( AF_DataRequest( &SmartHomeApp_DstAddr, &SmartHomeApp_epDesc,
                       SmartHomeApp_CLUSTERID,
                       (byte)osal_strlen( (char*)buf ) + 1,
                       (byte *)buf,
                       &SmartHomeApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  //  printf("SendCmdOK:%s\n", buf);
  } else {
    printf("SendCmdFailed:%s\n", buf); 
  }
  
}

static void processUartCB(uint8* buf, uint8 len)
{
  // process UART input
  if (Util_StartWith(buf, "ReadName")) {
    printf("ReadName:%s\n", Util_ReadName());
  } else if (Util_StartWith(buf, "WriteName ")) {
    Util_WriteName(buf + 10, len - 9);
    printf("WriteName:%s\n", Util_ReadName());
  } else if (Util_StartWith(buf, "SendCmd ")) {
    SmartHome_SendCmd(buf + 8);
  } else {
    
    printf("Unkown Cmd:%s\n", buf); 
  }
  
  
}


static void SmartHomeApp_CallBack(uint8 port, uint8 event)
{
  (void)port;
  
  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
#if SERIAL_APP_LOOPBACK
      (SerialApp_TxLen < SERIAL_APP_TX_MAX))
#else
      !SerialApp_TxLen)
#endif
  {
    SerialApp_TxLen = HalUARTRead(0, RxBuf, SERIAL_APP_TX_MAX);
    if (SerialApp_TxLen)
    {
      RxBuf[SerialApp_TxLen] = 0;
      processUartCB(RxBuf, SerialApp_TxLen);
        
     
      SerialApp_TxLen=0;
    }
  }
  

//  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
//#if SERIAL_APP_LOOPBACK
//      (SerialApp_TxLen < SERIAL_APP_TX_MAX))
//#else
//      !SerialApp_TxLen)
//#endif
//  {
//    SerialApp_Send();
//  }
}

/*********************************************************************
 * @fn      SmartHomeApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SmartHomeApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SmartHomeApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          SmartHomeApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          SmartHomeApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
            printf("SendMsgFailed:%X\n", sentTransID);
            if (SmartHomeApp_LastSendTransID == sentTransID) {
              NameAddrCache_delete(SmartHomeApp_LastSendAddr);
            }
            
          } else {
            printf("SendMsgOK:%X\n", sentTransID);
            
          }
          break;

        case AF_INCOMING_MSG_CMD:
          SmartHomeApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          SmartHomeApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( (SmartHomeApp_NwkState == DEV_ZB_COORD)
              || (SmartHomeApp_NwkState == DEV_ROUTER)
              || (SmartHomeApp_NwkState == DEV_END_DEVICE) )
          {
            
            uint16 shortAddr = NLME_GetShortAddr();
            uint16 parentShortAddr = NLME_GetCoordShortAddr();
            
            printf("Addr=%04X, ParentAddr=%04X\n", shortAddr, parentShortAddr);
            
//            printf("BV(4) = %X, BV(5) = %X, BV(6) = %X\n", BV(4), BV(5), BV(6));
//            printf("P0_4 = %X, P0_5 = %X, P0_6 = %X\n", P0_4, P0_5, P0_6);
//            
            // Start sending "the" message in a regular interval.
//            osal_start_timerEx( SmartHomeApp_TaskID,
//                                SmartHomeApp_SEND_MSG_EVT,
//                                SmartHomeApp_SEND_MSG_TIMEOUT );
            
            osal_start_timerEx( SmartHomeApp_TaskID,
                        SmartHomeApp_TIMER_UPDATE_EVT,
                        1000 );
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SmartHomeApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SmartHomeApp_Init()).
//  if ( events & SmartHomeApp_SEND_MSG_EVT )
//  {
//    // Send "the" message
//    SmartHomeApp_SendTheMessage();
//
//    // Setup to send message again
//    osal_start_timerEx( SmartHomeApp_TaskID,
//                        SmartHomeApp_SEND_MSG_EVT,
//                        SmartHomeApp_SEND_MSG_TIMEOUT );
//
//    // return unprocessed events
//    return (events ^ SmartHomeApp_SEND_MSG_EVT);
//  }

  if ( events & SmartHomeApp_TIMER_UPDATE_EVT )
  {
    
    DoorLight_Update();
    // Setup to send message again
    osal_start_timerEx( SmartHomeApp_TaskID,
                        SmartHomeApp_TIMER_UPDATE_EVT,
                        1000 );

    // return unprocessed events
    return (events ^ SmartHomeApp_TIMER_UPDATE_EVT);
  }
  

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      SmartHomeApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
static void SmartHomeApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined( BLINK_LEDS )
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            SmartHomeApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            SmartHomeApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            SmartHomeApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      SmartHomeApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void SmartHomeApp_HandleKeys( uint8 shift, uint8 keys )
{
//  zAddrType_t dstAddr;
  
  Processor_HandleKeys(keys);

//  // Shift is used to make each button/switch dual purpose.
//  if ( shift )
//  {
//    if ( keys & HAL_KEY_SW_1 )
//    {
//    }
//    if ( keys & HAL_KEY_SW_2 )
//    {
//    }
//    if ( keys & HAL_KEY_SW_3 )
//    {
//    }
//    if ( keys & HAL_KEY_SW_4 )
//    {
//    }
//  }
//  else
//  {
//    if ( keys & HAL_KEY_SW_1 )
//    {
//      // Since SW1 isn't used for anything else in this application...
//#if defined( SWITCH1_BIND )
//      // we can use SW1 to simulate SW2 for devices that only have one switch,
//      keys |= HAL_KEY_SW_2;
//#elif defined( SWITCH1_MATCH )
//      // or use SW1 to simulate SW4 for devices that only have one switch
//      keys |= HAL_KEY_SW_4;
//#endif
//    }
//
//    if ( keys & HAL_KEY_SW_2 )
//    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//
//      // Initiate an End Device Bind Request for the mandatory endpoint
//      dstAddr.addrMode = Addr16Bit;
//      dstAddr.addr.shortAddr = 0x0000; // Coordinator
//      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
//                            SmartHomeApp_epDesc.endPoint,
//                            SmartHomeApp_PROFID,
//                            SmartHomeApp_MAX_CLUSTERS, (cId_t *)SmartHomeApp_ClusterList,
//                            SmartHomeApp_MAX_CLUSTERS, (cId_t *)SmartHomeApp_ClusterList,
//                            FALSE );
//    }
//
//    if ( keys & HAL_KEY_SW_3 )
//    {
//    }
//
//    if ( keys & HAL_KEY_SW_4 )
//    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//      // Initiate a Match Description Request (Service Discovery)
//      dstAddr.addrMode = AddrBroadcast;
//      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
//      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
//                        SmartHomeApp_PROFID,
//                        SmartHomeApp_MAX_CLUSTERS, (cId_t *)SmartHomeApp_ClusterList,
//                        SmartHomeApp_MAX_CLUSTERS, (cId_t *)SmartHomeApp_ClusterList,
//                        FALSE );
//    }
//  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SmartHomeApp_ProcessReportNames(afIncomingMSGPacket_t *inMsg)
{
  uint16 shortAddr = NLME_GetShortAddr();
  uint16 parentShortAddr = NLME_GetCoordShortAddr();
  char buf[48];
  int len = sprintf(buf, "ReportNamesResp %s %04X %04X", Util_ReadName(), shortAddr, parentShortAddr);
  
  SmartHomeApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  SmartHomeApp_DstAddr.endPoint = SmartHomeApp_ENDPOINT;
  SmartHomeApp_DstAddr.addr.shortAddr = inMsg->srcAddr.addr.shortAddr;
  
    
  if ( AF_DataRequest( &SmartHomeApp_DstAddr, &SmartHomeApp_epDesc,
                       SmartHomeApp_CLUSTERID,
                       osal_strlen(buf)+1,
                       (uint8*)buf,
                       &SmartHomeApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    printf("send ReportNamesResp error %s\n", buf);
  }
  
}

/**
 save name addr map 
*/
void SmartHomeApp_ProcessNameAddrMapResp(uint8* data);
void SmartHomeApp_ProcessNameAddrMapResp(uint8* data)
{
   //NameAddrMap 1234 name
  if (osal_strlen((char*)data) < 18|| osal_strlen((char*)data) > 38) {
    // invalid data
    printf("Invalid name addr map: %s\n", data);
    return;
  }
  
  
  uint8* addrPtr = data + 12;
  uint8* namePtr = data + 17;
  uint16 addr = Util_Str2Uint16(addrPtr);
  
 // printf("addrPtr: %s, name=%s, add=%04X\n", addrPtr, namePtr, addr);
  
  if (addr != 0xFFFF) {
    NameAddrCache_put(namePtr, addr); 
  }
}


static void SmartHomeApp_SendNameAddrResp(afIncomingMSGPacket_t *pkt)
{
  uint8 buf[64];
  
  
  SmartHomeApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  SmartHomeApp_DstAddr.endPoint = SmartHomeApp_ENDPOINT;
  SmartHomeApp_DstAddr.addr.shortAddr = pkt->srcAddr.addr.shortAddr;
  
  sprintf((char*)buf, "NameAddrMap %04X %s", NLME_GetShortAddr(), (char*)Util_ReadName());
    
  if ( AF_DataRequest( &SmartHomeApp_DstAddr, &SmartHomeApp_epDesc,
                       SmartHomeApp_CLUSTERID,
                       osal_strlen((char*)buf)+1,
                       (uint8*)buf,
                       &SmartHomeApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    printf("SmartHomeApp_SendNameAddrResp error %s\n", buf);
  }
  
}


static uint8 SmartHomeApp_NameAddrNotMatch(uint8* data)
{
  // find name and cmpare  
  uint8* name;
  uint8 ret;
  
  name = Util_GetNameFromCmd(data);
  
  if (name == NULL)
    return 1;
 
  
  ret = Util_StrEqual(name, Util_ReadName());
  osal_mem_free(name);
  return !ret;
  
}

/*********************************************************************
 * @fn      SmartHomeApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
static void SmartHomeApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case SmartHomeApp_CLUSTERID:
      pkt->cmd.Data[pkt->cmd.DataLength-1]=0;
      printf("ReceiveMsg:%s\n", pkt->cmd.Data);
      
      if (Util_StrEqual(pkt->cmd.Data, (uint8*)"ReportNames")) {
        SmartHomeApp_ProcessReportNames(pkt); 
      } else if (Util_StartWith(pkt->cmd.Data, (uint8*)"NameAddrMap ")) {
        SmartHomeApp_ProcessNameAddrMapResp(pkt->cmd.Data); 
      } else {
        
        if (SmartHomeApp_NameAddrNotMatch(pkt->cmd.Data)) { // maybe wrong cache data
          printf("NameAddrNotMatch %s\n", pkt->cmd.Data);
          if (!pkt->wasBroadcast) {
            SmartHomeApp_SendNameAddrResp(pkt);
          }
          return;
        }
        
        if (pkt->wasBroadcast) {
          SmartHomeApp_SendNameAddrResp(pkt); 
        }
        
        Processor_HandleMsg(pkt->cmd.Data);
      }
      
//#if defined( LCD_SUPPORTED )
//      HalLcdWriteScreen( (char*)pkt->cmd.Data, "rcvd" );
//#elif defined( WIN32 )
//      WPRINTSTR( pkt->cmd.Data );
//#endif
      break;
  }
}

/*********************************************************************
 * @fn      SmartHomeApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
//static void SmartHomeApp_SendTheMessage( void )
//{
//  char theMessageData[] = "Hello World";
//
//  if ( AF_DataRequest( &SmartHomeApp_DstAddr, &SmartHomeApp_epDesc,
//                       SmartHomeApp_CLUSTERID,
//                       (byte)osal_strlen( theMessageData ) + 1,
//                       (byte *)&theMessageData,
//                       &SmartHomeApp_TransID,
//                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
//  {
//    // Successfully requested to be sent.
//  }
//  else
//  {
//    // Error occurred in request to send.
//  }
//}


/*********************************************************************
 */
