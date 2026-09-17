/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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
/*
 * Auto generated file
 */

#include "ti_drivers_config.h"
#include <drivers/sciclient.h>

/*
 * I2C
 */
/* I2C atrributes */
static I2C_HwAttrs gI2cHwAttrs[CONFIG_I2C_NUM_INSTANCES] =
{
    {
        .baseAddr       = CSL_I2C0_CFG_BASE,
        .intNum         = 193,
        .eventId        = 0,
        .funcClk        = 96000000U,
        .enableIntr     = 1,
        .ownTargetAddr   =
        {
            0x1C,
            0x1C,
            0x1C,
            0x1C,
        },
    },
    {
        .baseAddr       = CSL_I2C1_CFG_BASE,
        .intNum         = 194,
        .eventId        = 0,
        .funcClk        = 96000000U,
        .enableIntr     = 1,
        .ownTargetAddr   =
        {
            0x1C,
            0x1C,
            0x1C,
            0x1C,
        },
    },
};
/* I2C objects - initialized by the driver */
static I2C_Object gI2cObjects[CONFIG_I2C_NUM_INSTANCES];
/* I2C driver configuration */
I2C_Config gI2cConfig[CONFIG_I2C_NUM_INSTANCES] =
{
    {
        .object = &gI2cObjects[CONFIG_I2C0],
        .hwAttrs = &gI2cHwAttrs[CONFIG_I2C0]
    },
    {
        .object = &gI2cObjects[CONFIG_I2C1],
        .hwAttrs = &gI2cHwAttrs[CONFIG_I2C1]
    },
};

uint32_t gI2cConfigNum = CONFIG_I2C_NUM_INSTANCES;

/*
 * GPIO
 */

/* ----------- GPIO Direction, Trigger, Interrupt initialization ----------- */

void GPIO_init()
{
    uint32_t    baseAddr;

    /* Instance 0 */
    /* Get address after translation translate */
    baseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_GPIO0_BASE_ADDR);
    GPIO_setDirMode(baseAddr, CONFIG_GPIO0_PIN, CONFIG_GPIO0_DIR);

    /* Instance 1 */
    /* Get address after translation translate */
    baseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_GPIO1_BASE_ADDR);
    GPIO_setDirMode(baseAddr, CONFIG_GPIO1_PIN, CONFIG_GPIO1_DIR);

    /* Instance 2 */
    /* Get address after translation translate */
    baseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_GPIO2_BASE_ADDR);
    GPIO_setDirMode(baseAddr, CONFIG_GPIO2_PIN, CONFIG_GPIO2_DIR);
}


/* ----------- GPIO Interrupt de-initialization ----------- */
void GPIO_deinit()
{

}

/*
 * PRUICSS
 */
/* PRUICSS HW attributes - provided by the driver */
extern PRUICSS_HwAttrs gPruIcssHwAttrs_ICSSG0;

/* PRUICSS objects - initialized by the driver */
static PRUICSS_Object gPruIcssObjects[CONFIG_PRUICSS_NUM_INSTANCES];
/* PRUICSS driver configuration */
PRUICSS_Config gPruIcssConfig[CONFIG_PRUICSS_NUM_INSTANCES] =
{
    {
        .object = &gPruIcssObjects[CONFIG_PRU_ICSS1],
        .hwAttrs = &gPruIcssHwAttrs_ICSSG0
    },
};

uint32_t gPruIcssConfigNum = CONFIG_PRUICSS_NUM_INSTANCES;

/*
 * UDMA
 */
/* UDMA driver instance object */
Udma_DrvObject          gUdmaDrvObj[CONFIG_UDMA_NUM_INSTANCES];
/* UDMA driver instance init params */
static Udma_InitPrms    gUdmaInitPrms[CONFIG_UDMA_NUM_INSTANCES] =
{
    {
        .instId             = UDMA_INST_ID_PKTDMA_0,
        .skipGlobalEventReg = FALSE,
        .virtToPhyFxn       = Udma_defaultVirtToPhyFxn,
        .phyToVirtFxn       = Udma_defaultPhyToVirtFxn,
    },
};

/*
 * UART
 */

/* UART atrributes */
static UART_Attrs gUartAttrs[CONFIG_UART_NUM_INSTANCES] =
{
    {
        .baseAddr           = CSL_UART0_BASE,
        .inputClkFreq       = 48000000U,
    },
};
/* UART objects - initialized by the driver */
static UART_Object gUartObjects[CONFIG_UART_NUM_INSTANCES];
/* UART driver configuration */
UART_Config gUartConfig[CONFIG_UART_NUM_INSTANCES] =
{
    {
        &gUartAttrs[CONFIG_UART0],
        &gUartObjects[CONFIG_UART0],
    },
};

uint32_t gUartConfigNum = CONFIG_UART_NUM_INSTANCES;

#include <drivers/uart/v0/dma/uart_dma.h>


UART_DmaConfig gUartDmaConfig[CONFIG_UART_NUM_DMA_INSTANCES] =
{
};

uint32_t gUartDmaConfigNum = CONFIG_UART_NUM_DMA_INSTANCES;


void Drivers_uartInit(void)
{
    UART_init();
}


void Pinmux_init(void);
void PowerClock_init(void);
void PowerClock_deinit(void);

/*
 * Common Functions
 */
void System_init(void)
{
    /* DPL init sets up address transalation unit, on some CPUs this is needed
     * to access SCICLIENT services, hence this needs to happen first
     */
    Dpl_init();
    /* We should do sciclient init before we enable power and clock to the peripherals */
    /* SCICLIENT init */
    {
        int32_t retVal = SystemP_SUCCESS;

        retVal = Sciclient_init(CSL_CORE_ID_R5FSS0_0);
        DebugP_assertNoLog(SystemP_SUCCESS == retVal);
    }

    
    /* initialize PMU */
    CycleCounterP_init(SOC_getSelfCpuClk());


    PowerClock_init();
    /* Now we can do pinmux */
    Pinmux_init();
    /* finally we initialize all peripheral drivers */
    I2C_init();
    GPIO_init();
    PRUICSS_init();

    /* UDMA */
    {
        uint32_t        instId;
        int32_t         retVal = UDMA_SOK;

        for(instId = 0U; instId < CONFIG_UDMA_NUM_INSTANCES; instId++)
        {
            retVal += Udma_init(&gUdmaDrvObj[instId], &gUdmaInitPrms[instId]);
            DebugP_assert(UDMA_SOK == retVal);
        }
    }
    Drivers_uartInit();
}

void System_deinit(void)
{
    I2C_deinit();
    GPIO_deinit();
    PRUICSS_deinit();
    /* UDMA */
    {
        uint32_t        instId;
        int32_t         retVal = UDMA_SOK;

        for(instId = 0U; instId < CONFIG_UDMA_NUM_INSTANCES; instId++)
        {
            retVal += Udma_deinit(&gUdmaDrvObj[instId]);
            DebugP_assert(UDMA_SOK == retVal);
        }
    }
    UART_deinit();
    PowerClock_deinit();
    /* SCICLIENT deinit */
    {
        int32_t         retVal = SystemP_SUCCESS;

        retVal = Sciclient_deinit();
        DebugP_assertNoLog(SystemP_SUCCESS == retVal);
    }
    Dpl_deinit();
}
