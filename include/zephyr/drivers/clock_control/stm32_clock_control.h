/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2016 BayLibre, SAS
 * Copyright (c) 2017-2022 Linaro Limited.
 * Copyright (c) 2017 RnDity Sp. z o.o.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_STM32_CLOCK_CONTROL_H_
#define ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_STM32_CLOCK_CONTROL_H_

#include <drivers/clock_control.h>

#if defined(CONFIG_SOC_SERIES_STM32F0X) || \
	defined(CONFIG_SOC_SERIES_STM32F1X) || \
	defined(CONFIG_SOC_SERIES_STM32F3X)
#include <dt-bindings/clock/stm32f1_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32F2X) || \
	defined(CONFIG_SOC_SERIES_STM32F4X) || \
	defined(CONFIG_SOC_SERIES_STM32F7X)
#include <dt-bindings/clock/stm32f4_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32G0X)
#include <dt-bindings/clock/stm32g0_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32L0X)
#include <dt-bindings/clock/stm32l0_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32L1X)
#include <dt-bindings/clock/stm32l1_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32G4X) || \
	defined(CONFIG_SOC_SERIES_STM32L4X) || \
	defined(CONFIG_SOC_SERIES_STM32L5X) || \
	defined(CONFIG_SOC_SERIES_STM32WBX)
#include <dt-bindings/clock/stm32l4_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32WLX)
#include <dt-bindings/clock/stm32wl_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32H7X)
#include <dt-bindings/clock/stm32h7_clock.h>
#elif defined(CONFIG_SOC_SERIES_STM32U5X)
#include <dt-bindings/clock/stm32u5_clock.h>
#else
#include <dt-bindings/clock/stm32_clock.h>
#endif

/** Common clock control device node for all STM32 chips */
#define STM32_CLOCK_CONTROL_NODE DT_NODELABEL(rcc)

/** RCC node related symbols */

#define STM32_AHB_PRESCALER	DT_PROP(DT_NODELABEL(rcc), ahb_prescaler)
#define STM32_APB1_PRESCALER	DT_PROP(DT_NODELABEL(rcc), apb1_prescaler)
#define STM32_APB2_PRESCALER	DT_PROP(DT_NODELABEL(rcc), apb2_prescaler)
#define STM32_APB3_PRESCALER	DT_PROP(DT_NODELABEL(rcc), apb3_prescaler)
#define STM32_AHB3_PRESCALER	DT_PROP(DT_NODELABEL(rcc), ahb3_prescaler)
#define STM32_AHB4_PRESCALER	DT_PROP(DT_NODELABEL(rcc), ahb4_prescaler)
#define STM32_CPU1_PRESCALER	DT_PROP(DT_NODELABEL(rcc), cpu1_prescaler)
#define STM32_CPU2_PRESCALER	DT_PROP(DT_NODELABEL(rcc), cpu2_prescaler)

#if DT_NODE_HAS_PROP(DT_NODELABEL(rcc), ahb_prescaler)
#define STM32_CORE_PRESCALER	STM32_AHB_PRESCALER
#elif DT_NODE_HAS_PROP(DT_NODELABEL(rcc), cpu1_prescaler)
#define STM32_CORE_PRESCALER	STM32_CPU1_PRESCALER
#endif

#if DT_NODE_HAS_PROP(DT_NODELABEL(rcc), ahb3_prescaler)
#define STM32_FLASH_PRESCALER	STM32_AHB3_PRESCALER
#elif DT_NODE_HAS_PROP(DT_NODELABEL(rcc), ahb4_prescaler)
#define STM32_FLASH_PRESCALER	STM32_AHB4_PRESCALER
#else
#define STM32_FLASH_PRESCALER	STM32_CORE_PRESCALER
#endif

#define STM32_D1CPRE	DT_PROP(DT_NODELABEL(rcc), d1cpre)
#define STM32_HPRE	DT_PROP(DT_NODELABEL(rcc), hpre)
#define STM32_D2PPRE1	DT_PROP(DT_NODELABEL(rcc), d2ppre1)
#define STM32_D2PPRE2	DT_PROP(DT_NODELABEL(rcc), d2ppre2)
#define STM32_D1PPRE	DT_PROP(DT_NODELABEL(rcc), d1ppre)
#define STM32_D3PPRE	DT_PROP(DT_NODELABEL(rcc), d3ppre)

#define DT_RCC_CLOCKS_CTRL	DT_CLOCKS_CTLR(DT_NODELABEL(rcc))

/* To enable use of IS_ENABLED utility macro, these symbols
 * should not be defined directly using DT_SAME_NODE.
 */
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(pll))
#define STM32_SYSCLK_SRC_PLL	1
#endif
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(clk_hsi))
#define STM32_SYSCLK_SRC_HSI	1
#endif
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(clk_hse))
#define STM32_SYSCLK_SRC_HSE	1
#endif
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(clk_msi))
#define STM32_SYSCLK_SRC_MSI	1
#endif
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(clk_msis))
#define STM32_SYSCLK_SRC_MSIS	1
#endif
#if DT_SAME_NODE(DT_RCC_CLOCKS_CTRL, DT_NODELABEL(clk_csi))
#define STM32_SYSCLK_SRC_CSI	1
#endif


/** PLL node related symbols */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f2_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f4_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f7_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32g0_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32g4_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32l4_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32u5_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32wb_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32h7_pll_clock, okay)
#define STM32_PLL_ENABLED	1
#define STM32_PLL_M_DIVISOR	DT_PROP(DT_NODELABEL(pll), div_m)
#define STM32_PLL_N_MULTIPLIER	DT_PROP(DT_NODELABEL(pll), mul_n)
#define STM32_PLL_P_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll), div_p)
#define STM32_PLL_P_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll), div_p, 1)
#define STM32_PLL_Q_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll), div_q)
#define STM32_PLL_Q_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll), div_q, 1)
#define STM32_PLL_R_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll), div_r)
#define STM32_PLL_R_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll), div_r, 1)
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll3), st_stm32h7_pll_clock, okay)
#define STM32_PLL3_ENABLED	1
#define STM32_PLL3_M_DIVISOR	DT_PROP(DT_NODELABEL(pll3), div_m)
#define STM32_PLL3_N_MULTIPLIER	DT_PROP(DT_NODELABEL(pll3), mul_n)
#define STM32_PLL3_P_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll3), div_p)
#define STM32_PLL3_P_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll3), div_p, 1)
#define STM32_PLL3_Q_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll3), div_q)
#define STM32_PLL3_Q_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll3), div_q, 1)
#define STM32_PLL3_R_ENABLED	DT_NODE_HAS_PROP(DT_NODELABEL(pll3), div_r)
#define STM32_PLL3_R_DIVISOR	DT_PROP_OR(DT_NODELABEL(pll3), div_r, 1)
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f1_pll_clock, okay)
#define STM32_PLL_ENABLED	1
#define STM32_PLL_XTPRE		DT_PROP(DT_NODELABEL(pll), xtpre)
#define STM32_PLL_MULTIPLIER	DT_PROP(DT_NODELABEL(pll), mul)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f0_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f100_pll_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32f105_pll_clock, okay)
#define STM32_PLL_ENABLED	1
#define STM32_PLL_MULTIPLIER	DT_PROP(DT_NODELABEL(pll), mul)
#define STM32_PLL_PREDIV	DT_PROP(DT_NODELABEL(pll), prediv)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(pll), st_stm32l0_pll_clock, okay)
#define STM32_PLL_ENABLED	1
#define STM32_PLL_DIVISOR	DT_PROP(DT_NODELABEL(pll), div)
#define STM32_PLL_MULTIPLIER	DT_PROP(DT_NODELABEL(pll), mul)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(pll), okay) && \
	DT_NODE_HAS_PROP(DT_NODELABEL(pll), clocks)
#define DT_PLL_CLOCKS_CTRL	DT_CLOCKS_CTLR(DT_NODELABEL(pll))
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(clk_msi))
#define STM32_PLL_SRC_MSI	1
#endif
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(clk_msis))
#define STM32_PLL_SRC_MSIS	1
#endif
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(clk_hsi))
#define STM32_PLL_SRC_HSI	1
#endif
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(clk_csi))
#define STM32_PLL_SRC_CSI	1
#endif
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(clk_hse))
#define STM32_PLL_SRC_HSE	1
#endif
#if DT_SAME_NODE(DT_PLL_CLOCKS_CTRL, DT_NODELABEL(pll2))
#define STM32_PLL_SRC_PLL2	1
#endif

#endif


/** Fixed clocks related symbols */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_lse), fixed_clock, okay)
#define STM32_LSE_ENABLED	1
#define STM32_LSE_FREQ		DT_PROP(DT_NODELABEL(clk_lse), clock_frequency)
#else
#define STM32_LSE_ENABLED	0
#define STM32_LSE_FREQ		0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_msi), st_stm32_msi_clock, okay) || \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_msi), st_stm32l0_msi_clock, okay)
#define STM32_MSI_ENABLED	1
#define STM32_MSI_RANGE		DT_PROP(DT_NODELABEL(clk_msi), msi_range)
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_msi), st_stm32_msi_clock, okay)
#define STM32_MSI_ENABLED	1
#define STM32_MSI_PLL_MODE	DT_PROP(DT_NODELABEL(clk_msi), msi_pll_mode)
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_msis), st_stm32u5_msi_clock, okay)
#define STM32_MSIS_ENABLED	1
#define STM32_MSIS_RANGE	DT_PROP(DT_NODELABEL(clk_msis), msi_range)
#define STM32_MSIS_PLL_MODE	DT_PROP(DT_NODELABEL(clk_msis), msi_pll_mode)
#else
#define STM32_MSIS_ENABLED	0
#define STM32_MSIS_RANGE	0
#define STM32_MSIS_PLL_MODE	0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_msik), st_stm32u5_msi_clock, okay)
#define STM32_MSIK_ENABLED	1
#define STM32_MSIK_RANGE	DT_PROP(DT_NODELABEL(clk_msik), msi_range)
#define STM32_MSIK_PLL_MODE	DT_PROP(DT_NODELABEL(clk_msik), msi_pll_mode)
#else
#define STM32_MSIK_ENABLED	0
#define STM32_MSIK_RANGE	0
#define STM32_MSIK_PLL_MODE	0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_csi), fixed_clock, okay)
#define STM32_CSI_ENABLED	1
#define STM32_CSI_FREQ		DT_PROP(DT_NODELABEL(clk_csi), clock_frequency)
#else
#define STM32_CSI_FREQ		0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_lsi), fixed_clock, okay)
#define STM32_LSI_ENABLED	1
#define STM32_LSI_FREQ		DT_PROP(DT_NODELABEL(clk_lsi), clock_frequency)
#else
#define STM32_LSI_FREQ		0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_hsi), fixed_clock, okay)
#define STM32_HSI_ENABLED	1
#define STM32_HSI_FREQ		DT_PROP(DT_NODELABEL(clk_hsi), clock_frequency)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_hsi), st_stm32h7_hsi_clock, okay)
#define STM32_HSI_ENABLED	1
#define STM32_HSI_DIVISOR	DT_PROP(DT_NODELABEL(clk_hsi), hsi_div)
#define STM32_HSI_FREQ		DT_PROP(DT_NODELABEL(clk_hsi), clock_frequency)
#else
#define STM32_HSI_DIVISOR	1
#define STM32_HSI_FREQ		0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_hse), fixed_clock, okay)
#define STM32_HSE_ENABLED	1
#define STM32_HSE_FREQ		DT_PROP(DT_NODELABEL(clk_hse), clock_frequency)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_hse), st_stm32_hse_clock, okay)
#define STM32_HSE_ENABLED	1
#define STM32_HSE_BYPASS	DT_PROP(DT_NODELABEL(clk_hse), hse_bypass)
#define STM32_HSE_FREQ		DT_PROP(DT_NODELABEL(clk_hse), clock_frequency)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(clk_hse), st_stm32wl_hse_clock, okay)
#define STM32_HSE_ENABLED	1
#define STM32_HSE_TCXO		DT_PROP(DT_NODELABEL(clk_hse), hse_tcxo)
#define STM32_HSE_DIV2		DT_PROP(DT_NODELABEL(clk_hse), hse_div2)
#define STM32_HSE_FREQ		DT_PROP(DT_NODELABEL(clk_hse), clock_frequency)
#else
#define STM32_HSE_FREQ		0
#endif

/** Driver structure definition */

struct stm32_pclken {
	uint32_t bus;
	uint32_t enr;
};

#endif /* ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_STM32_CLOCK_CONTROL_H_ */
