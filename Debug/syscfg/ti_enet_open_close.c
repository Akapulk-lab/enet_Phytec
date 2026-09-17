/*
 *  Copyright (c) Texas Instruments Incorporated 2020
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file ti_enet_open_close.c
 *
 * \brief This file contains enet driver memory allocation related functionality.
 */


/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <enet.h>
#include "enet_appmemutils.h"
#include "enet_appmemutils_cfg.h"
#include "enet_apputils.h"
#include <enet_cfg.h>
#include <include/core/enet_per.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_dma.h>
#include <include/common/enet_utils_dflt.h>
#include <include/per/icssg.h>
#include <priv/per/icssg_priv.h>
#include <drivers/udma/udma_priv.h>
#include <soc/k3/icssg_soc.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/EventP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/QueueP.h>

#include "ti_enet_config.h"
#include "ti_drivers_config.h"
#include "ti_enet_open_close.h"
#include <utils/include/enet_appsoc.h>

#define ENETAPP_PHY_STATEHANDLER_TASK_PRIORITY        (7U)
#define ENETAPP_PHY_STATEHANDLER_TASK_STACK     (3 * 1024)
#define AppEventId_ICSSG_PERIODIC_POLL          (1 << 3)


#include <drivers/udma/udma_priv.h>
#include <drivers/udma.h>

typedef struct EnetAppTxDmaSysCfg_Obj_s
{
    /* TX channel handle */
    EnetDma_TxChHandle hTxCh;
    /* TX channel number */
    uint32_t txChNum;
} EnetAppTxDmaSysCfg_Obj;

typedef struct txAppHandleInfo_s
{
    uint32_t useGlobalEvt;
    
    uint32_t packetsCount;
    
    Enet_Type enetType;
    
    uint32_t instId;
}txAppHandleInfo;

typedef struct EnetAppRxDmaSysCfg_Obj_s
{
    /* RX channel handle */
    EnetDma_RxChHandle hRxCh;
    /* RX flow start index */
    uint32_t rxFlowIdx;
    /* RX flow start index */
    uint32_t rxFlowStartIdx;
    /* num mac Address valid in macAddr[][] list below*/
    uint32_t numValidMacAddress;
    /* MAC address. It's port/ports MAC address in Dual-MAC or
     * host port MAC address in Switch */
    uint8_t macAddr[ENET_MAX_NUM_MAC_PER_PHER][ENET_MAC_ADDR_LEN];
} EnetAppRxDmaSysCfg_Obj;

typedef struct EnetAppRxDmaCfg_Info_s
{
    /* mac Address valid */
    uint32_t numValidMacAddress;
    /* max numTxPkts for the channel */
    uint32_t maxNumRxPkts;
    /*! Whether to use the shared global event or not. If set to false, a dedicated event
     *  will be used for this channel. */
    bool  useGlobalEvt;
    /*! Whether to use the shared global event or not. If set to false, a dedicated event
     *  will be used for this channel. */
    bool  useDefaultFlow;
    /*! [IN] UDMAP receive flow packet size based free buffer queue enable configuration
     * to be programmed into the rx_size_thresh_en field of the RFLOW_RFC register.
     * See the UDMAP section of the TRM for more information on this setting.
     * Configuration of the optional size thresholds when this configuration is
     * enabled is done by sending the @ref tisci_msg_rm_udmap_flow_size_thresh_cfg_req
     * message to System Firmware for the receive flow allocated by this request.
     * This parameter can be no greater than
     * @ref TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SIZE_THRESH_MAX */
    uint8_t                 sizeThreshEn;
    
    /* Channel id associated with the rx flow. Relevant only for ICSSG as for CPSW rx chIdx is
     * always 0
     */
    uint32_t chIdx;
        
    Enet_Type enetType;
    
    uint32_t instId;
#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    bool  useRingMon;
#endif
} EnetAppRxDmaCfg_Info;

typedef struct EnetAppTxDmaCfg_Info_s
{
    /*! Whether to use the shared global event or not. If set to false, a dedicated event
     *  will be used for this channel. */
    bool  useGlobalEvt;
    /*! UDMA driver handle*/
    Udma_DrvHandle hUdmaDrv;
    
    Enet_Type enetType;
    
    uint32_t instId;
    
    uint32_t txNumPkts;
} EnetAppTxDmaCfg_Info;

static int32_t EnetAppUtils_allocRxFlowForChIdx(Enet_Handle hEnet,
                                                uint32_t coreKey,
                                                uint32_t coreId,
                                                uint32_t chIdx,
                                                uint32_t *rxFlowStartIdx,
                                                uint32_t *flowIdx);
static int32_t EnetAppUtils_allocTxCh(Enet_Handle hEnet,
                                      uint32_t coreKey,
                                      uint32_t coreId,
                                      uint32_t *txPSILThreadId);
static int32_t EnetAppUtils_freeRxFlowForChIdx(Enet_Handle hEnet,
                                               uint32_t coreKey,
                                               uint32_t coreId,
                                               uint32_t chIdx,
                                               uint32_t rxFlowIdx);
static int32_t EnetAppUtils_freeTxCh(Enet_Handle hEnet,
                                     uint32_t coreKey,
                                     uint32_t coreId,
                                     uint32_t txChNum);
static uint32_t EnetAppUtils_getStartFlowIdx(Enet_Handle hEnet,
                                             uint32_t coreId);
static void EnetAppUtils_absFlowIdx2FlowIdxOffset(Enet_Handle hEnet,
                                                  uint32_t coreId,
                                                  uint32_t absRxFlowId,
                                                  uint32_t *pStartFlowIdx,
                                                  uint32_t *pFlowIdxOffset);
static void EnetAppUtils_openTxCh(Enet_Handle hEnet,
                                  uint32_t coreKey,
                                  uint32_t coreId,
                                  uint32_t *pTxChNum,
                                  EnetDma_TxChHandle *pTxChHandle,
                                  EnetUdma_OpenTxChPrms *pIcssTxChCfg);

static void EnetAppUtils_closeTxCh(Enet_Handle hEnet,
                                   uint32_t coreKey,
                                   uint32_t coreId,
                                   EnetDma_PktQ *pFqPktInfoQ,
                                   EnetDma_PktQ *pCqPktInfoQ,
                                   EnetDma_TxChHandle hTxChHandle,
                                   uint32_t txChNum);

static void EnetAppUtils_openRxFlowForChIdx(Enet_Type enetType,
                                            Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            bool useDfltFlow,
                                            uint32_t allocMacAddrCnt,
                                            uint32_t chIdx,
                                            uint32_t *pRxFlowStartIdx,
                                            uint32_t *pRxFlowIdx,
                                            uint8_t macAddr[ENET_MAX_NUM_MAC_PER_PHER][ENET_MAC_ADDR_LEN],
                                            EnetDma_RxChHandle *pRxFlowHandle,
                                            EnetUdma_OpenRxFlowPrms *pRxFlowPrms);

static void EnetAppUtils_closeRxFlowForChIdx(Enet_Type enetType,
                                            Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            bool useDfltFlow,
                                            EnetDma_PktQ *pFqPktInfoQ,
                                            EnetDma_PktQ *pCqPktInfoQ,
                                            uint32_t chIdx,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t rxFlowIdx,
                                            uint32_t numValidMacAddress,
                                            uint8_t macAddr[ENET_MAX_NUM_MAC_PER_PHER][ENET_MAC_ADDR_LEN],
                                            EnetDma_RxChHandle hRxFlow);

static void EnetApp_openRxDma(EnetAppRxDmaSysCfg_Obj *rx,
                              Enet_Handle hEnet, 
                              uint32_t coreKey,
                              uint32_t coreId,
                              uint32_t chIdx,
                              Udma_DrvHandle hUdmaDrv,
                              const EnetAppRxDmaCfg_Info *rxCfg);

static void EnetApp_openTxDma(EnetAppTxDmaSysCfg_Obj *tx,
                              uint32_t numTxPkts,
                              Enet_Handle hEnet, 
                              uint32_t coreKey,
                              uint32_t coreId,
                              EnetAppTxDmaCfg_Info *txCfg);


static void EnetAppUtils_setCommonRxFlowPrms(EnetUdma_OpenRxFlowPrms *pRxChPrms);

static void EnetAppUtils_setCommonTxChPrms(EnetUdma_OpenTxChPrms *pTxChPrms);

Udma_DrvHandle EnetApp_getUdmaInstanceHandle(void);


typedef struct EnetAppDmaSysCfg_Obj_s
{
    EnetAppTxDmaSysCfg_Obj tx[ENET_SYSCFG_TX_CHANNELS_NUM];
    EnetAppRxDmaSysCfg_Obj rx[ENET_SYSCFG_RX_FLOWS_NUM];
} EnetAppDmaSysCfg_Obj;


typedef struct EnetAppSysCfg_Obj_s
{

    Enet_Handle hEnet[ENET_SYSCFG_NUM_PERIPHERAL];
    EnetAppDmaSysCfg_Obj dma[1];
    ClockP_Object timerObj;

    TaskP_Object task_phyStateHandlerObj;

    SemaphoreP_Object timerSemObj;

    volatile bool timerTaskShutDownFlag;

    volatile bool timerTaskShutDownDoneFlag;

    uint8_t appPhyStateHandlerTaskStack[ENETAPP_PHY_STATEHANDLER_TASK_STACK] __attribute__ ((aligned(32)));
}EnetAppSysCfg_Obj;

static EnetAppSysCfg_Obj gEnetAppSysCfgObj;

static int32_t EnetApp_enablePorts(Enet_Handle hEnet,
                                   Enet_Type enetType,
                                   uint32_t instId,
                                   uint32_t coreId,
                                   Enet_MacPort macPortList[ENET_MAC_PORT_NUM],
                                   uint8_t numMacPorts);


static void EnetApp_openDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                    Enet_Type enetType, 
                                    uint32_t instId,
                                    Enet_Handle hEnet, 
                                    uint32_t coreKey,
                                    uint32_t coreId);

static void EnetApp_openAllRxDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                         Enet_Handle hEnet, 
                                         uint32_t coreKey,
                                         uint32_t coreId);

static void EnetApp_openAllTxDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                         Enet_Handle hEnet, 
                                         uint32_t coreKey,
                                         uint32_t coreId);

static void EnetApp_getIcssgInitCfg(Enet_Type enetType,
                                    uint32_t instId,
                                    Icssg_Cfg *pIcssgCfg);


static void EnetApp_phyStateHandler(void * appHandle);

static uint32_t EnetApp_getEnetIdx(Enet_Type enetType, uint32_t instId)
{
    return 0;
}

static Enet_Handle EnetApp_doIcssgOpen(Enet_Type enetType, uint32_t instId, const Icssg_Cfg *icssgCfg)
{
    int32_t status = ENET_SOK;
    void *perCfg = NULL_PTR;
    uint32_t cfgSize;
    Enet_Handle hEnet;

    EnetAppUtils_assert(true == Enet_isIcssFamily(enetType));

    perCfg = (void *)icssgCfg;
    cfgSize = sizeof(*icssgCfg);

    hEnet = Enet_open(enetType, instId, perCfg, cfgSize);
    if(hEnet == NULL_PTR)
    {
        EnetAppUtils_print("Enet_open failed\r\n");
        EnetAppUtils_assert(hEnet != NULL_PTR);
    }

    status = EnetMem_init();
    EnetAppUtils_assert(ENET_SOK == status);

    return hEnet;
}

static void EnetApp_timerCb(ClockP_Object *clkInst, void * arg)
{
    SemaphoreP_Object *pTimerSem = (SemaphoreP_Object *)arg;

    /* Tick! */
    SemaphoreP_post(pTimerSem);
}
static void EnetApp_phyStateHandler(void * appHandle)
{
    SemaphoreP_Object *timerSem;
    EnetAppSysCfg_Obj *hEnetAppObj       = (EnetAppSysCfg_Obj *)appHandle;

    timerSem = &hEnetAppObj->timerSemObj;
    hEnetAppObj->timerTaskShutDownDoneFlag = false;
    while (hEnetAppObj->timerTaskShutDownFlag != true)
    {
        SemaphoreP_pend(timerSem, SystemP_WAIT_FOREVER);
        /* Enet_periodicTick should be called from only task context */
        for  (uint32_t idx = 0; idx < ENET_SYSCFG_NUM_PERIPHERAL; idx++)
        {
            if (hEnetAppObj->hEnet[idx] != NULL)
            {
                Enet_periodicTick(hEnetAppObj->hEnet[idx]);
            }
        }
    }
    hEnetAppObj->timerTaskShutDownDoneFlag = true;
    TaskP_destruct(&hEnetAppObj->task_phyStateHandlerObj);
    TaskP_exit();
}

static int32_t EnetApp_createPhyStateHandlerTask(EnetAppSysCfg_Obj *hEnetAppObj)
{
    TaskP_Params tskParams;
    int32_t status;

    status = SemaphoreP_constructCounting(&hEnetAppObj->timerSemObj, 0, 128);
    EnetAppUtils_assert(status == SystemP_SUCCESS);
    {
        ClockP_Params clkParams;
        const uint32_t timPeriodTicks = ClockP_usecToTicks((ENETPHY_FSM_TICK_PERIOD_MS)*1000U);  // Set timer expiry time in OS ticks

        ClockP_Params_init(&clkParams);
        clkParams.start     = TRUE;
        clkParams.timeout   = timPeriodTicks;
        clkParams.period    = timPeriodTicks;
        clkParams.args      = &hEnetAppObj->timerSemObj;
        clkParams.callback  = &EnetApp_timerCb;

        /* Creating timer and setting timer callback function*/
        status = ClockP_construct(&hEnetAppObj->timerObj ,
                                  &clkParams);
        if (status == SystemP_SUCCESS)
        {
            hEnetAppObj->timerTaskShutDownFlag = false;
        }
        else
        {
            EnetAppUtils_print("EnetApp_createClock() failed to create clock\r\n");
        }
    }
    /* Initialize the taskperiodicTick params. Set the task priority higher than the
     * default priority (1) */
    TaskP_Params_init(&tskParams);
    tskParams.priority       = ENETAPP_PHY_STATEHANDLER_TASK_PRIORITY;
    tskParams.stack          = &hEnetAppObj->appPhyStateHandlerTaskStack[0];
    tskParams.stackSize      = sizeof(hEnetAppObj->appPhyStateHandlerTaskStack);
    tskParams.args           = hEnetAppObj;
    tskParams.name           = "EnetApp_PhyStateHandlerTask";
    tskParams.taskMain       =  &EnetApp_phyStateHandler;

    status = TaskP_construct(&hEnetAppObj->task_phyStateHandlerObj, &tskParams);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    return status;

}

static void EnetApp_ConfigureDscpMapping(Enet_Type enetType,
                                         uint32_t instId)
{
    Enet_IoctlPrms prms;
    Enet_Handle hEnet;
    EnetMacPort_SetIngressDscpPriorityMapInArgs inArgs;
    int32_t status;
    uint8_t numMacPorts;
    Enet_MacPort macPortList[ENET_SYSCFG_MAX_MAC_PORTS];
    uint32_t i;

    EnetApp_getEnetInstMacInfo(enetType,
                               instId,
                               macPortList,
                               &numMacPorts);

    hEnet = Enet_getHandle(enetType, instId);
    /*Configure FT3 filter and classifier and Enable DSCP*/
    memset(&inArgs, 0, sizeof(inArgs));
    /* mapped most used 8 dscp priority points to 8 qos levels remaining are routed to 0
       Write a non zero value to required dscp from 0 to 63 in increasing priority order
    */
    inArgs.dscpPriorityMap.tosMap[0]  = 0U;
    inArgs.dscpPriorityMap.tosMap[10] = 1U;
    inArgs.dscpPriorityMap.tosMap[18] = 2U;
    inArgs.dscpPriorityMap.tosMap[26] = 3U;
    inArgs.dscpPriorityMap.tosMap[34] = 4U;
    inArgs.dscpPriorityMap.tosMap[46] = 5U;
    inArgs.dscpPriorityMap.tosMap[48] = 6U;
    inArgs.dscpPriorityMap.tosMap[56] = 7U;

    inArgs.dscpPriorityMap.dscpIPv4En = 1;
    for (i = 0; i < numMacPorts; i++)
    {
        inArgs.macPort = macPortList[i];

        ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);
        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), ENET_MACPORT_IOCTL_SET_INGRESS_DSCP_PRI_MAP, &prms, status);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to set dscp Priority map for Port %d - %d \r\n", macPortList[i], status);
        }
    }
}

void EnetApp_driverInit()
{
/* keep this implementation that is generic across enetType and instId.
 * Initialization should be done only once.
 */
    EnetOsal_Cfg osalPrms;
    EnetUtils_Cfg utilsPrms;

    /* Initialize Enet driver with default OSAL, utils */
    Enet_initOsalCfg(&osalPrms);
    Enet_initUtilsCfg(&utilsPrms);
    Enet_init(&osalPrms, &utilsPrms);
}

int32_t EnetApp_driverOpen(Enet_Type enetType, uint32_t instId)
{
    int32_t status = ENET_SOK;
    Icssg_Cfg icssgCfg;
    Enet_MacPort macPortList[ENET_MAC_PORT_NUM];
    uint8_t numMacPorts;
    uint32_t selfCoreId;
    EnetRm_ResCfg *resCfg = &icssgCfg.resCfg;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetApp_HandleInfo handleInfo;
    EnetUdma_Cfg dmaCfg;
    EnetAppUtils_assert(Enet_isIcssFamily(enetType) == true);
    EnetApp_getEnetInstMacInfo(enetType, instId, macPortList, &numMacPorts);
    const uint32_t hEnetIndex = EnetApp_getEnetIdx(enetType, instId);

    /* Open UDMA */
    dmaCfg.rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
    dmaCfg.hUdmaDrv = EnetApp_getUdmaInstanceHandle();
    selfCoreId   = EnetSoc_getCoreId();
    icssgCfg.dmaCfg = (void *)&dmaCfg;

    Enet_initCfg(enetType, instId, &icssgCfg, sizeof(icssgCfg));
    icssgCfg.dmaCfg = (void *)&dmaCfg;

    EnetApp_getIcssgInitCfg(enetType, instId, &icssgCfg);
    
    resCfg = &icssgCfg.resCfg;
    EnetAppUtils_initResourceConfig(enetType, selfCoreId, resCfg);
    EnetApp_updateIcssgInitCfg(enetType, instId, &icssgCfg);

    icssgCfg.qosLevels = 3U;
    icssgCfg.isPremQueEnable = false;
    gEnetAppSysCfgObj.hEnet[hEnetIndex] = EnetApp_doIcssgOpen(enetType, instId, &icssgCfg);
    EnetAppUtils_assert(NULL != gEnetAppSysCfgObj.hEnet[hEnetIndex]);

    EnetApp_enablePorts(gEnetAppSysCfgObj.hEnet[hEnetIndex], enetType, instId, selfCoreId, macPortList, numMacPorts);
    EnetApp_ConfigureDscpMapping(enetType, instId);
    if (hEnetIndex == 0)
    {
        status = EnetApp_createPhyStateHandlerTask(&gEnetAppSysCfgObj);
    }
    EnetAppUtils_assert(status == SystemP_SUCCESS);
    /* Open all DMA channels */
    EnetApp_acquireHandleInfo(enetType, 
                              instId,
                              &handleInfo);

    (void)handleInfo; /* Handle info not used. Kill warning */
    EnetApp_coreAttach(enetType, 
                       instId,
                       selfCoreId,
                       &attachInfo);
    EnetApp_openDmaChannels(&gEnetAppSysCfgObj.dma[0],
                            enetType, 
                            instId,
                            gEnetAppSysCfgObj.hEnet[hEnetIndex], 
                            attachInfo.coreKey,
                            selfCoreId);
    return status;
}

static void EnetApp_asyncIoctlCb(Enet_Event evt,
                                uint32_t evtNum,
                                void *evtCbArgs,
                                void *arg1,
                                void *arg2)
{
    SemaphoreP_Object *pAsyncSem = (SemaphoreP_Object *)evtCbArgs;
    SemaphoreP_post(pAsyncSem);
}

static int32_t EnetApp_enablePorts(Enet_Handle hEnet,
                                   Enet_Type enetType,
                                   uint32_t instId,
                                   uint32_t coreId,
                                   Enet_MacPort macPortList[ENET_MAC_PORT_NUM],
                                   uint8_t numMacPorts)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    uint8_t i;
    SemaphoreP_Object ayncIoctlSemObj;

    status = SemaphoreP_constructBinary(&ayncIoctlSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    Enet_registerEventCb(hEnet,
                        ENET_EVT_ASYNC_CMD_RESP,
                        0U,
                        EnetApp_asyncIoctlCb,
                        (void *)&ayncIoctlSemObj);

    for (i = 0U; i < numMacPorts; i++)
    {
        EnetPer_PortLinkCfg linkArgs;
        IcssgMacPort_Cfg icssgMacCfg;
        linkArgs.macCfg = &icssgMacCfg;
        EnetPhy_initCfg(&linkArgs.phyCfg);
        linkArgs.macPort = macPortList[i];
        EnetApp_initLinkArgs(enetType, instId, &linkArgs, macPortList[i]);
        {
            /* If gigabit support is disabled at instance level, update the phy userCaps to
             * not advertise Gigabit capability so that link is not established at gigabit speed
             */
            EnetPhy_Cfg *phyCfg = &linkArgs.phyCfg;
            EnetMacPort_LinkCfg *linkCfg = &linkArgs.linkCfg;

            if ((linkCfg->speed == ENETPHY_SPEED_AUTO) ||
                (linkCfg->duplexity == ENETPHY_DUPLEX_AUTO))
            {
                if (phyCfg->nwayCaps == 0u)
                {
                    phyCfg->nwayCaps = ENETPHY_LINK_CAP_ALL;
                }
                phyCfg->nwayCaps &= ~(ENETPHY_LINK_CAP_1000);
            }
            else
            {
                /* Application has disabled Gigabit support in syscfg but
                 * is configuring link speed expliticly to gigabit.
                 * This is wrong configuration. Fix sysconfig to enable
                 * gigabit support
                 */
                EnetAppUtils_assert(linkCfg->speed != ENETPHY_SPEED_1GBIT);
            }
        }
        ENET_IOCTL_SET_IN_ARGS(&prms, &linkArgs);
        ENET_IOCTL(hEnet,
                   coreId,
                   ENET_PER_IOCTL_OPEN_PORT_LINK,
                   &prms,
                   status);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("EnetApp_enablePorts() failed to open MAC port: %d\r\n", status);
        }

        if (status == ENET_SOK)
        {
            IcssgMacPort_SetPortStateInArgs setPortStateInArgs;

            setPortStateInArgs.macPort   = macPortList[i];
            setPortStateInArgs.portState = ICSSG_PORT_STATE_FORWARD;
            ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);
            prms.outArgs = NULL_PTR;

            ENET_IOCTL(hEnet, coreId, ICSSG_PER_IOCTL_SET_PORT_STATE, &prms, status);
            if (status == ENET_SINPROGRESS)
            {
                /* Wait for asyc ioctl to complete */
                do
                {
                    Enet_poll(hEnet, ENET_EVT_ASYNC_CMD_RESP, NULL, 0U);
                    status = SemaphoreP_pend(&ayncIoctlSemObj, SystemP_WAIT_FOREVER);
                    if (SystemP_SUCCESS == status)
                    {
                        break;
                    }
                } while (1);

                status = ENET_SOK;
            }
            else
            {
                EnetAppUtils_print("Failed to set port state: %d\n", status);
            }
        }
    }

    /* Show alive PHYs */
    if (status == ENET_SOK)
    {
        Enet_IoctlPrms prms;
        bool alive;
        int32_t i;

        for (i = 0U; i < ENET_MDIO_PHY_CNT_MAX; i++)
        {
            ENET_IOCTL_SET_INOUT_ARGS(&prms, &i, &alive);
            ENET_IOCTL(hEnet,
                       coreId,
                       ENET_MDIO_IOCTL_IS_ALIVE,
                       &prms,
                       status);
            if (status == ENET_SOK)
            {
                if (alive == true)
                {
                    EnetAppUtils_print("PHY %d is alive\r\n", i);
                }
            }
            else
            {
                EnetAppUtils_print("Failed to get PHY %d alive status: %d\r\n", i, status);
            }
        }
    }

    SemaphoreP_destruct(&ayncIoctlSemObj);
    Enet_unregisterEventCb(hEnet, ENET_EVT_ASYNC_CMD_RESP, 0U);

    return status;
}

static void EnetApp_deleteClock(EnetAppSysCfg_Obj *hEnetAppObj)
{
    ClockP_stop(&hEnetAppObj->timerObj);
    hEnetAppObj->timerTaskShutDownFlag = true;
    /* Post Timer Sem once to get the Periodic Tick task terminated */
    SemaphoreP_post(&hEnetAppObj->timerSemObj);

    do
    {
        ClockP_usleep(ClockP_ticksToUsec(1));
    } while (hEnetAppObj->timerTaskShutDownDoneFlag != true);

    SemaphoreP_destruct(&hEnetAppObj->timerSemObj);
    ClockP_destruct(&hEnetAppObj->timerObj);
}
void EnetApp_driverClose(Enet_Type enetType, uint32_t instId)
{
    Enet_IoctlPrms prms;
    int32_t status;
    uint32_t selfCoreId;
    uint32_t i;
    Enet_MacPort macPortList[ENET_MAC_PORT_NUM];
    uint8_t numMacPorts;
    Enet_Handle hEnet = Enet_getHandle(enetType, instId);

    EnetAppUtils_assert(Enet_isIcssFamily(enetType) == true);
    selfCoreId   = EnetSoc_getCoreId();
    EnetApp_getEnetInstMacInfo(enetType, instId, macPortList, &numMacPorts);
    EnetApp_deleteClock(&gEnetAppSysCfgObj);
    Enet_unregisterEventCb(hEnet,
                            ENET_EVT_ASYNC_CMD_RESP,
                            0U);

    EnetAppUtils_print("Unregister TX timestamp callback\r\n");
    Enet_unregisterEventCb(hEnet,
                            ENET_EVT_TIMESTAMP_TX,
                            0U);
    for (i = 0U; i < numMacPorts; i++)
    {
        Enet_MacPort macPort = macPortList[i];

        ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);
        ENET_IOCTL(hEnet,
                   selfCoreId,
                   ENET_PER_IOCTL_CLOSE_PORT_LINK,
                   &prms,
                   status);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("close() failed to close MAC port: %d\r\n", status);
        }
    }

    Enet_close(hEnet);

    EnetMem_deInit();

    Enet_deinit();
}




static uint32_t EnetApp_retrieveFreeTxPkts(EnetDma_TxChHandle hTxCh, EnetDma_PktQ *txPktInfoQ)
{
    EnetDma_PktQ txFreeQ;
    EnetDma_Pkt *pktInfo;
    uint32_t txFreeQCnt = 0U;
    int32_t status;

    EnetQueue_initQ(&txFreeQ);

    /* Retrieve any packets that may be free now */
    status = EnetDma_retrieveTxPktQ(hTxCh, &txFreeQ);
    if (status == ENET_SOK)
    {
        txFreeQCnt = EnetQueue_getQCount(&txFreeQ);

        pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        while (NULL != pktInfo)
        {
            EnetDma_checkPktState(&pktInfo->pktState,
                                    ENET_PKTSTATE_MODULE_APP,
                                    ENET_PKTSTATE_APP_WITH_DRIVER,
                                    ENET_PKTSTATE_APP_WITH_FREEQ);

            EnetQueue_enq(txPktInfoQ, &pktInfo->node);
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        }
    }
    else
    {
        EnetAppUtils_print("retrieveFreeTxPkts() failed to retrieve pkts: %d\r\n", status);
    }

    return txFreeQCnt;
}



static void EnetApp_openDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                    Enet_Type enetType,
                                    uint32_t instId,
                                    Enet_Handle hEnet, 
                                    uint32_t coreKey,
                                    uint32_t coreId)
{
    EnetApp_openAllTxDmaChannels(dma,
                                 hEnet, 
                                 coreKey,
                                 coreId);
    EnetApp_openAllRxDmaChannels(dma,
                                 hEnet,
                                 coreKey,
                                 coreId);
}






static int32_t EnetAppUtils_allocRxFlowForChIdx(Enet_Handle hEnet,
                                                uint32_t coreKey,
                                                uint32_t coreId,
                                                uint32_t chIdx,
                                                uint32_t *rxFlowStartIdx,
                                                uint32_t *flowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_AllocRxFlowInArgs inArgs;
    EnetRm_AllocRxFlow rxFlowPrms;

    inArgs.coreKey = coreKey;
    inArgs.chIdx   = chIdx;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &rxFlowPrms);
    ENET_IOCTL(hEnet,
               coreId,
               ENET_RM_IOCTL_ALLOC_RX_FLOW,
               &prms,
               status);

    if (status == ENET_SOK)
    {
        *rxFlowStartIdx = rxFlowPrms.startIdx;
        *flowIdx        = rxFlowPrms.flowIdx;
    }
    else
    {
        EnetAppUtils_print("EnetAppUtils_allocRxFlowForChIdx() failed : %d\n", status);
    }

    return status;
}

static int32_t EnetAppUtils_allocTxCh(Enet_Handle hEnet,
                                      uint32_t coreKey,
                                      uint32_t coreId,
                                      uint32_t *txPSILThreadId)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;

    /* Allocate Tx Ch */
    ENET_IOCTL_SET_INOUT_ARGS(&prms, &coreKey, txPSILThreadId);
    ENET_IOCTL(hEnet,
               coreId,
               ENET_RM_IOCTL_ALLOC_TX_CH_PEERID,
               &prms,
               status);
    if (status != ENET_SOK)
    {
        *txPSILThreadId = ENET_RM_TXCHNUM_INVALID;
        EnetAppUtils_print("EnetAppUtils_allocTxCh() failed: %d\n", status);
    }

    return status;
}

static int32_t EnetAppUtils_freeRxFlow(Enet_Handle hEnet,
                                       uint32_t coreKey,
                                       uint32_t coreId,
                                       uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeRxFlowInArgs freeRxFlowInArgs;

    /*Free Rx Flow*/
    freeRxFlowInArgs.coreKey = coreKey;
    freeRxFlowInArgs.flowIdx = rxFlowIdx;
    freeRxFlowInArgs.chIdx   = 0U;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeRxFlowInArgs);
    ENET_IOCTL(hEnet,
               coreId,
               ENET_RM_IOCTL_FREE_RX_FLOW,
               &prms,
               status);

    return status;
}

static int32_t EnetAppUtils_freeRxFlowForChIdx(Enet_Handle hEnet,
                                               uint32_t coreKey,
                                               uint32_t coreId,
                                               uint32_t chIdx,
                                               uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeRxFlowInArgs freeRxFlowInArgs;

    /*Free Rx Flow*/
    freeRxFlowInArgs.coreKey = coreKey;
    freeRxFlowInArgs.flowIdx = rxFlowIdx;
    freeRxFlowInArgs.chIdx   = chIdx;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeRxFlowInArgs);
    ENET_IOCTL(hEnet,
               coreId,
               ENET_RM_IOCTL_FREE_RX_FLOW,
               &prms,
               status);

    return status;
}

static int32_t EnetAppUtils_freeTxCh(Enet_Handle hEnet,
                                     uint32_t coreKey,
                                     uint32_t coreId,
                                     uint32_t txChNum)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeTxChInArgs freeTxChInArgs;

    /* Release Tx Ch */
    freeTxChInArgs.coreKey = coreKey;
    freeTxChInArgs.txChNum = txChNum;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeTxChInArgs);
    ENET_IOCTL(hEnet,
               coreId,
               ENET_RM_IOCTL_FREE_TX_CH_PEERID,
               &prms,
               status);

    return status;
}


static void EnetAppUtils_openTxCh(Enet_Handle hEnet,
                                  uint32_t coreKey,
                                  uint32_t coreId,
                                  uint32_t *pTxChNum,
                                  EnetDma_TxChHandle *pTxChHandle,
                                  EnetUdma_OpenTxChPrms *pIcssTxChCfg)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    int32_t status;

    EnetAppUtils_assert(hDma != NULL);

    status = EnetAppUtils_allocTxCh(hEnet,
                                    coreKey,
                                    coreId,
                                    pTxChNum);
    EnetAppUtils_assert(ENET_SOK == status);

    pIcssTxChCfg->chNum = *pTxChNum;

    *pTxChHandle = EnetDma_openTxCh(hDma, pIcssTxChCfg);
    EnetAppUtils_assert(NULL != *pTxChHandle);
}

static void EnetAppUtils_closeTxCh(Enet_Handle hEnet,
                                   uint32_t coreKey,
                                   uint32_t coreId,
                                   EnetDma_PktQ *pFqPktInfoQ,
                                   EnetDma_PktQ *pCqPktInfoQ,
                                   EnetDma_TxChHandle hTxChHandle,
                                   uint32_t txChNum)
{
    int32_t status;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    EnetDma_disableTxEvent(hTxChHandle);
    status = EnetDma_closeTxCh(hTxChHandle, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(ENET_SOK == status);

    status = EnetAppUtils_freeTxCh(hEnet,
                                   coreKey,
                                   coreId,
                                   txChNum);
    EnetAppUtils_assert(ENET_SOK == status);
}


static void EnetAppUtils_openRxFlowForChIdx(Enet_Type enetType,
                                            Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            bool useDefaultFlow,
                                            uint32_t allocMacAddrCnt,
                                            uint32_t chIdx,
                                            uint32_t *pRxFlowStartIdx,
                                            uint32_t *pRxFlowIdx,
                                            uint8_t macAddr[ENET_MAX_NUM_MAC_PER_PHER][ENET_MAC_ADDR_LEN],
                                            EnetDma_RxChHandle *pRxFlowHandle,
                                            EnetUdma_OpenRxFlowPrms *pRxFlowPrms)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    int32_t status = ENET_SOK;

    EnetAppUtils_assert(hDma != NULL);

    status = EnetAppUtils_allocRxFlowForChIdx(hEnet,
                                              coreKey,
                                              coreId,
                                              chIdx,
                                              pRxFlowStartIdx,
                                              pRxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);

    pRxFlowPrms->startIdx = *pRxFlowStartIdx;
    pRxFlowPrms->flowIdx  = *pRxFlowIdx;
    pRxFlowPrms->chIdx    = chIdx;

    *pRxFlowHandle = EnetDma_openRxCh(hDma, pRxFlowPrms);
    EnetAppUtils_assert(*pRxFlowHandle != NULL);

    if (useDefaultFlow)
    {
        if (chIdx == 0U)
        {
            for (uint32_t i = 0; i < allocMacAddrCnt; i++)
            {
                status = EnetAppUtils_allocMac(hEnet, coreKey, coreId, macAddr[i]);
                EnetAppUtils_assert(status == ENET_SOK);

                {
                    // Should we add this entry to ICSSG FDB?
                }
            }
        }

        status = EnetAppUtils_regDfltRxFlowForChIdx(hEnet,
                                                    coreKey,
                                                    coreId,
                                                    chIdx,
                                                    *pRxFlowStartIdx,
                                                    *pRxFlowIdx);
    }
    else
    {
        if (allocMacAddrCnt > 0)
        {

        }
    }

    EnetAppUtils_assert(status == ENET_SOK);
}

static void EnetAppUtils_closeRxFlowForChIdx(Enet_Type enetType,
                                            Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            bool useDefaultFlow,
                                            EnetDma_PktQ *pFqPktInfoQ,
                                            EnetDma_PktQ *pCqPktInfoQ,
                                            uint32_t chIdx,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t rxFlowIdx,
                                            uint32_t allocMacAddrCnt,
                                            uint8_t macAddr[ENET_MAX_NUM_MAC_PER_PHER][ENET_MAC_ADDR_LEN],
                                            EnetDma_RxChHandle hRxFlow)
{
    int32_t status = ENET_SOK;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    EnetDma_disableRxEvent(hRxFlow);

    if (useDefaultFlow)
    {
        status = EnetAppUtils_unregDfltRxFlowForChIdx(hEnet,
                                                      coreKey,
                                                      coreId,
                                                      chIdx,
                                                      rxFlowStartIdx,
                                                      rxFlowIdx);
        EnetAppUtils_assert(status == ENET_SOK);

        for(uint32_t i = 0; i < allocMacAddrCnt; i++)
        {
            status = EnetAppUtils_freeMac(hEnet,
                                          coreKey,
                                          coreId,
                                          macAddr[i]);
            EnetAppUtils_assert(status == ENET_SOK);
        }
    }
    else
    {

    }

    status = EnetDma_closeRxCh(hRxFlow, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);


    status = EnetAppUtils_freeRxFlowForChIdx(hEnet,
                                             coreKey,
                                             coreId,
                                             chIdx,
                                             rxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);
}


static void EnetApp_txPktNotifyCb(void *cbArg)
{


}

static void EnetApp_rxPktNotifyCb(void *cbArg)
{


}

static void EnetApp_openTxDma(EnetAppTxDmaSysCfg_Obj *tx,
                              uint32_t numTxPkts,
                              Enet_Handle hEnet, 
                              uint32_t coreKey,
                              uint32_t coreId,
                              EnetAppTxDmaCfg_Info *txCfg)
{
    EnetUdma_OpenTxChPrms enetTxChCfg;

    EnetDma_initTxChParams(&enetTxChCfg);
    EnetAppUtils_setCommonTxChPrms(&enetTxChCfg);
    enetTxChCfg.hUdmaDrv  = txCfg->hUdmaDrv;
    /* This param is dont care for UDMA LCDMA variant */
    enetTxChCfg.useProxy  = false;
    enetTxChCfg.numTxPkts = numTxPkts;
    enetTxChCfg.cbArg     = tx;
    enetTxChCfg.notifyCb  = EnetApp_txPktNotifyCb;
    enetTxChCfg.useGlobalEvt = txCfg->useGlobalEvt;


    EnetAppUtils_openTxCh(hEnet,
                          coreKey,
                          coreId,
                          &tx->txChNum,
                          &tx->hTxCh,
                          &enetTxChCfg);
}

static void EnetApp_openRxDma(EnetAppRxDmaSysCfg_Obj *rx,
                              Enet_Handle hEnet, 
                              uint32_t coreKey,
                              uint32_t coreId,
                              uint32_t chIdx,
                              Udma_DrvHandle hUdmaDrv,
                              const EnetAppRxDmaCfg_Info *rxCfg)
{
    EnetUdma_OpenRxFlowPrms enetRxFlowCfg;
    int32_t status;
    Enet_Type enetType;
    uint32_t instId;

    status = Enet_getHandleInfo(hEnet,
                                &enetType,
                                &instId);
    EnetAppUtils_assert(status == ENET_SOK);
    (void)instId; /* Instd id not used */

    EnetDma_initRxChParams(&enetRxFlowCfg);
    EnetAppUtils_setCommonRxFlowPrms(&enetRxFlowCfg);
    /* Plugin dummy callback. This will be overridden by 
     * application provided callback when getHandle is done */
    enetRxFlowCfg.notifyCb  = EnetApp_rxPktNotifyCb;
    enetRxFlowCfg.cbArg     = rx;
    enetRxFlowCfg.numRxPkts = rxCfg->maxNumRxPkts;
    enetRxFlowCfg.hUdmaDrv  = hUdmaDrv;
    /* This param is dont care for UDMA LCDMA variant */
    enetRxFlowCfg.useProxy  = false;
    enetRxFlowCfg.useGlobalEvt = rxCfg->useGlobalEvt;
    enetRxFlowCfg.flowPrms.sizeThreshEn = rxCfg->sizeThreshEn;

#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    {
        /* Use ring monitor for the CQ ring of RX flow */
        EnetUdma_UdmaRingPrms *pFqRingPrms = &enetRxFlowCfg.udmaChPrms.fqRingPrms;
        pFqRingPrms->useRingMon = rxCfg->useRingMon;
        pFqRingPrms->ringMonCfg.mode = TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD;
        /* Ring mon low threshold */

#if defined _DEBUG_
        /* In debug mode as CPU is processing lesser packets per event, keep threshold more */
        pFqRingPrms->ringMonCfg.data0 = (numRxPkts - 10U);
#else
        pFqRingPrms->ringMonCfg.data0 = (numRxPkts - 20U);
#endif
        /* Ring mon high threshold - to get only low  threshold event, setting high threshold as more than ring depth*/
        pFqRingPrms->ringMonCfg.data1 = numRxPkts;
    }
#endif


    EnetAppUtils_openRxFlowForChIdx(enetType,
                                hEnet,
                                coreKey,
                                coreId,
                                rxCfg->useDefaultFlow,
                                rxCfg->numValidMacAddress,
                                chIdx,
                                &rx->rxFlowStartIdx,
                                &rx->rxFlowIdx,
                                rx->macAddr,
                                &rx->hRxCh,
                                &enetRxFlowCfg);

    rx->numValidMacAddress = rxCfg->numValidMacAddress;

}

void EnetApp_closeTxDma(uint32_t enetTxDmaChId,
                        Enet_Handle hEnet, 
                        uint32_t coreKey,
                        uint32_t coreId,
                        EnetDma_PktQ *fqPktInfoQ,
                        EnetDma_PktQ *cqPktInfoQ)
{
    EnetAppTxDmaSysCfg_Obj *tx;
    uint32_t i;
    for (i = 0; i< ENET_SYSCFG_NUM_PERIPHERAL; i++)
    {
        EnetAppUtils_assert(enetTxDmaChId < ENET_ARRAYSIZE(gEnetAppSysCfgObj.dma[i].tx));
        tx = &gEnetAppSysCfgObj.dma[i].tx[enetTxDmaChId];
    }
    EnetQueue_initQ(fqPktInfoQ);
    EnetQueue_initQ(cqPktInfoQ);
    EnetApp_retrieveFreeTxPkts(tx->hTxCh, cqPktInfoQ);
    EnetAppUtils_closeTxCh(hEnet,
                           coreKey,
                           coreId,
                           fqPktInfoQ,
                           cqPktInfoQ,
                           tx->hTxCh,
                           tx->txChNum);
    memset(tx, 0, sizeof(*tx));
}

void EnetApp_closeRxDma(uint32_t enetRxDmaChId,
                        Enet_Handle hEnet, 
                        uint32_t coreKey,
                        uint32_t coreId,
                        EnetDma_PktQ *fqPktInfoQ,
                        EnetDma_PktQ *cqPktInfoQ)
{
    Enet_Type enetType;
    uint32_t instId;
    int32_t status;
    const EnetAppRxDmaCfg_Info rxDmaInfo[ENET_SYSCFG_RX_FLOWS_NUM] = 
    {
        
        [0] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 1,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 0,
        },
        [1] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 0,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 1,
        }
    
    };

    status = Enet_getHandleInfo(hEnet,
                                &enetType,
                                &instId);
    EnetAppUtils_assert(status == ENET_SOK);
    EnetAppUtils_assert(enetRxDmaChId < ENET_ARRAYSIZE(gEnetAppSysCfgObj.dma[0].rx));
    EnetAppUtils_assert(enetRxDmaChId < ENET_ARRAYSIZE(rxDmaInfo));

    /* Close RX channel */
    EnetQueue_initQ(fqPktInfoQ);
    EnetQueue_initQ(cqPktInfoQ);

    EnetAppRxDmaSysCfg_Obj *pRx= &gEnetAppSysCfgObj.dma[0].rx[enetRxDmaChId];
    EnetAppUtils_closeRxFlowForChIdx(enetType,
                                         hEnet,
                                         coreKey,
                                         coreId,
                                         rxDmaInfo[enetRxDmaChId].useDefaultFlow,
                                         fqPktInfoQ,
                                         cqPktInfoQ,
                                         enetRxDmaChId,
                                         pRx->rxFlowStartIdx,
                                         pRx->rxFlowIdx,
                                         pRx->numValidMacAddress,
                                         pRx->macAddr,
                                         pRx->hRxCh);
    EnetAppSoc_releaseMacAddrList(pRx->macAddr, pRx->numValidMacAddress);
    memset(pRx, 0, sizeof(*pRx));
}

void EnetApp_txChInPeripheral(Enet_Type enetType, uint32_t instId, uint32_t *startIdx, uint32_t *chCount)
{
    uint32_t i, count = 0;
    int32_t startIdOffset = -1;
    const txAppHandleInfo txChInfo[ENET_SYSCFG_TX_CHANNELS_NUM] =
                           {
                               
        [0] = 
        {
                .useGlobalEvt    = true,
                .packetsCount    = 8,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
        }
                           };
    
    for (i = 0; i < ENET_SYSCFG_TX_CHANNELS_NUM; i++)
    {
        if (enetType == txChInfo[i].enetType && instId == txChInfo[i].instId)
        {
            if (startIdOffset == -1)
            {
                startIdOffset = i;
            }
            count++;
        }
    }
    *startIdx = startIdOffset;
    *chCount  = count;
}

void EnetApp_rxChInPeripheral(Enet_Type enetType, uint32_t instId, uint32_t *startIdx, uint32_t *chCount)
{
    uint32_t i, count = 0;
    int32_t startIdOffset = -1;
    const EnetApp_GetRxDmaHandleOutArgs rxChInfo[ENET_SYSCFG_RX_FLOWS_NUM] =
                           {
                               
        [0] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 1,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 0,
        },
        [1] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 0,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 1,
        }
                           };
    
    for (i = 0; i < ENET_SYSCFG_RX_FLOWS_NUM; i++)
    {
        if (enetType == rxChInfo[i].enetType && instId == rxChInfo[i].instId)
        {
            if (startIdOffset == -1)
            {
                startIdOffset = i;
            }
            count++;
        }
    }
    *startIdx = startIdOffset;
    *chCount  = count;
}

void EnetApp_getTxDmaHandle(uint32_t enetTxDmaChId,
                            const EnetApp_GetDmaHandleInArgs *inArgs,
                            EnetApp_GetTxDmaHandleOutArgs *outArgs)
{
    int32_t status;
    EnetAppTxDmaSysCfg_Obj *tx;
    const uint32_t module = 0;
    const uint32_t txNumPkts[ENET_SYSCFG_TX_CHANNELS_NUM] = 
                           {
                            8
                           };

    const txAppHandleInfo txChInfo[ENET_SYSCFG_TX_CHANNELS_NUM] =
                           {
                               
        [0] = 
        {
                .useGlobalEvt    = true,
                .packetsCount    = 8,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
        }
                           };

    EnetAppUtils_assert(enetTxDmaChId < ENET_ARRAYSIZE(txNumPkts));
    EnetAppUtils_assert(enetTxDmaChId < ENET_ARRAYSIZE(gEnetAppSysCfgObj.dma[module].tx));
    tx = &gEnetAppSysCfgObj.dma[module].tx[enetTxDmaChId];

    EnetAppUtils_assert(tx->hTxCh != NULL);
    status = EnetDma_registerTxEventCb(tx->hTxCh, inArgs->notifyCb, inArgs->cbArg);
    EnetAppUtils_assert(status == ENET_SOK);
    outArgs->hTxCh = tx->hTxCh;
    outArgs->txChNum = tx->txChNum;
    outArgs->maxNumTxPkts = txChInfo[enetTxDmaChId].packetsCount;
    outArgs->useGlobalEvt = txChInfo[enetTxDmaChId].useGlobalEvt;
    return;
}

void EnetApp_getMacAddress(uint32_t enetRxDmaChId,
                            EnetApp_GetMacAddrOutArgs *outArgs)
{

    EnetAppUtils_assert(enetRxDmaChId < ENET_ARRAYSIZE(gEnetAppSysCfgObj.dma[0].rx));
    EnetAppRxDmaSysCfg_Obj* rx = &gEnetAppSysCfgObj.dma[0].rx[enetRxDmaChId]; // 

    outArgs->macAddressCnt = rx->numValidMacAddress;
    EnetAppUtils_assert(outArgs->macAddressCnt <= ENET_MAX_NUM_MAC_PER_PHER);
    for (uint32_t i = 0; i < outArgs->macAddressCnt; i++)
    {
        EnetUtils_copyMacAddr(outArgs->macAddr[i], rx->macAddr[i]);
    }

}

void EnetApp_getRxDmaHandle(uint32_t enetRxDmaChId,
                            const EnetApp_GetDmaHandleInArgs *inArgs,
                            EnetApp_GetRxDmaHandleOutArgs *outArgs)
{
    int32_t status;
    uint32_t startIdOffset;
    EnetAppRxDmaSysCfg_Obj *rx;

    const uint32_t module = 0;
    const EnetApp_GetRxDmaHandleOutArgs rxDmaInfo[ENET_SYSCFG_RX_FLOWS_NUM] = 
    {
        
        [0] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 1,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 0,
        },
        [1] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 0,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 1,
        }
    };

    for (startIdOffset = 0; startIdOffset < ENET_SYSCFG_RX_FLOWS_NUM; startIdOffset++)
    {
        if (inArgs->enetType == rxDmaInfo[startIdOffset].enetType && inArgs->instId == rxDmaInfo[startIdOffset].instId)
        {
            break;
        }
    }
    EnetAppUtils_assert(enetRxDmaChId < ENET_ARRAYSIZE(gEnetAppSysCfgObj.dma[module].rx));
    rx = &gEnetAppSysCfgObj.dma[module].rx[enetRxDmaChId];

    EnetAppUtils_assert(rx->hRxCh != NULL);
    status = EnetDma_registerRxEventCb(rx->hRxCh, inArgs->notifyCb, inArgs->cbArg);
    EnetAppUtils_assert(status == ENET_SOK);
    
    outArgs->hRxCh = rx->hRxCh;
    outArgs->rxFlowIdx = rx->rxFlowIdx;
    outArgs->rxFlowStartIdx = rx->rxFlowStartIdx;
    EnetAppUtils_assert(enetRxDmaChId < ENET_ARRAYSIZE(rxDmaInfo));
    outArgs->numValidMacAddress = rx->numValidMacAddress;
    for (uint32_t i = 0; i < rx->numValidMacAddress; i++)
    {
        EnetUtils_copyMacAddr(outArgs->macAddr[i], rx->macAddr[i]);
    }
    outArgs->maxNumRxPkts   = rxDmaInfo[enetRxDmaChId].maxNumRxPkts;
    outArgs->sizeThreshEn   = rxDmaInfo[enetRxDmaChId].sizeThreshEn;
    outArgs->useDefaultFlow = rxDmaInfo[enetRxDmaChId].useDefaultFlow;
    outArgs->useGlobalEvt   = rxDmaInfo[enetRxDmaChId].useGlobalEvt;
    outArgs->chIdx          = rxDmaInfo[enetRxDmaChId].chIdx;
    return;
}

static void EnetApp_openAllTxDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                         Enet_Handle hEnet, 
                                         uint32_t coreKey,
                                         uint32_t coreId)
{
    const uint32_t txNumPkts[ENET_SYSCFG_TX_CHANNELS_NUM] = 
                           {
                            8
                           };
                           
    EnetAppTxDmaCfg_Info txDmaCfg[ENET_SYSCFG_TX_CHANNELS_NUM] =
                           {
                            
        [0] = 
        {
                .useGlobalEvt    = true,
                .hUdmaDrv        = NULL,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
        }
                           };

    for (uint32_t chIdx = 0; chIdx < ENET_SYSCFG_TX_CHANNELS_NUM; chIdx++)
    {
        if (hEnet->enetPer->enetType == txDmaCfg[chIdx].enetType && hEnet->enetPer->instId == txDmaCfg[chIdx].instId)
        {
            txDmaCfg[chIdx].hUdmaDrv     = EnetApp_getUdmaInstanceHandle();
            EnetApp_openTxDma(&dma->tx[chIdx], txNumPkts[chIdx], hEnet, coreKey, coreId, &txDmaCfg[chIdx]);
        }
    }
}

static void EnetApp_openAllRxDmaChannels(EnetAppDmaSysCfg_Obj *dma,
                                         Enet_Handle hEnet, 
                                         uint32_t coreKey,
                                         uint32_t coreId)
{
    uint32_t flowIdx;
    const EnetAppRxDmaCfg_Info rxDmaCfg[ENET_SYSCFG_RX_FLOWS_NUM] = 
    {
        
        [0] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 1,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 0,
        },
        [1] = 
        {
                .maxNumRxPkts    = 8,
                .numValidMacAddress = 0,
                .useGlobalEvt    = true,
                .useDefaultFlow  = true,
                .sizeThreshEn    = 7,
                .enetType        = ENET_ICSSG_SWITCH,
                .instId          = 0,
                .chIdx           = 1,
        }
    };
    Udma_DrvHandle hUdmaDrv;

    hUdmaDrv = EnetApp_getUdmaInstanceHandle();

    for (flowIdx = 0; flowIdx<ENET_SYSCFG_RX_FLOWS_NUM; flowIdx++)
    {
        if(hEnet->enetPer->enetType == rxDmaCfg[flowIdx].enetType && hEnet->enetPer->instId == rxDmaCfg[flowIdx].instId)
        {
            EnetApp_openRxDma(&dma->rx[flowIdx], hEnet, coreKey, coreId,  rxDmaCfg[flowIdx].chIdx, hUdmaDrv, &rxDmaCfg[flowIdx]);
        }
    }
}

#define ENET_SYSCFG_DEFAULT_NUM_TX_PKT                                     (16U)
#define ENET_SYSCFG_DEFAULT_NUM_RX_PKT                                     (32U)


void EnetAppUtils_setCommonRxFlowPrms(EnetUdma_OpenRxFlowPrms *pRxFlowPrms)
{
    pRxFlowPrms->numRxPkts           = ENET_SYSCFG_DEFAULT_NUM_RX_PKT;
    pRxFlowPrms->disableCacheOpsFlag = false;
    pRxFlowPrms->rxFlowMtu           = ENET_MEM_LARGE_POOL_PKT_SIZE;

    pRxFlowPrms->ringMemAllocFxn = &EnetMem_allocRingMem;
    pRxFlowPrms->ringMemFreeFxn  = &EnetMem_freeRingMem;

    pRxFlowPrms->dmaDescAllocFxn = &EnetMem_allocDmaDesc;
    pRxFlowPrms->dmaDescFreeFxn  = &EnetMem_freeDmaDesc;
}

void EnetAppUtils_setCommonTxChPrms(EnetUdma_OpenTxChPrms *pTxChPrms)
{
    pTxChPrms->numTxPkts           = ENET_SYSCFG_DEFAULT_NUM_TX_PKT;
    pTxChPrms->disableCacheOpsFlag = false;

    pTxChPrms->ringMemAllocFxn = &EnetMem_allocRingMem;
    pTxChPrms->ringMemFreeFxn  = &EnetMem_freeRingMem;

    pTxChPrms->dmaDescAllocFxn = &EnetMem_allocDmaDesc;
    pTxChPrms->dmaDescFreeFxn  = &EnetMem_freeDmaDesc;
}


Udma_DrvHandle EnetApp_getUdmaInstanceHandle(void)
{
    Udma_DrvHandle hUdmaDrv;

    hUdmaDrv = &gUdmaDrvObj[CONFIG_UDMA_PKTDMA_0];
    return hUdmaDrv;
}






static const Mdio_Cfg enetAppIcssgMdioCfg =
{
    .mode               = MDIO_MODE_NORMAL,
    .mdioBusFreqHz      = 2200000,
    .phyStatePollFreqHz = 22000,
    .pollEnMask         = 3,
    .c45EnMask          = 0,
    .isMaster           = true,
    .disableStateMachineOnInit = false,
};

static const IcssgTimeSync_Cfg enetAppIcssgTimesyncCfg =
{
    .enable            = false,
    .clkType           = ICSSG_TIMESYNC_CLKTYPE_WORKING_CLOCK,
    .syncOut_start_WC  = 10000,
    .syncOut_pwidth_WC = 25000
};

static void EnetApp_initMdioConfig(Mdio_Cfg *pMdioCfg)
{
    *pMdioCfg = enetAppIcssgMdioCfg;
}

static void EnetApp_initTimesyncConfig(IcssgTimeSync_Cfg *pTimesyncCfg)
{
    *pTimesyncCfg = enetAppIcssgTimesyncCfg;
}

static void EnetApp_getIcssgInitCfg(Enet_Type enetType,
                                    uint32_t instId,
                                    Icssg_Cfg *pIcssgCfg)
{
    EnetApp_initTimesyncConfig(&pIcssgCfg->timeSyncCfg);
    EnetApp_initMdioConfig(&pIcssgCfg->mdioCfg);
}



