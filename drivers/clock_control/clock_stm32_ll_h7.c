/*
 *
 * Copyright (c) 2019 Linaro Limited.
 * Copyright (c) 2020 Jeremy LOCHE
 * Copyright (c) 2021 Electrolance Solutions
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_utils.h>
#include <drivers/clock_control.h>
#include <sys/util.h>
#include <drivers/clock_control/stm32_clock_control.h>
#include "stm32_hsem.h"

/* Macros to fill up prescaler values */
#define z_hsi_divider(v) LL_RCC_HSI_DIV ## v
#define hsi_divider(v) z_hsi_divider(v)

#define z_sysclk_prescaler(v) LL_RCC_SYSCLK_DIV_ ## v
#define sysclk_prescaler(v) z_sysclk_prescaler(v)

#define z_ahb_prescaler(v) LL_RCC_AHB_DIV_ ## v
#define ahb_prescaler(v) z_ahb_prescaler(v)

#define z_apb1_prescaler(v) LL_RCC_APB1_DIV_ ## v
#define apb1_prescaler(v) z_apb1_prescaler(v)

#define z_apb2_prescaler(v) LL_RCC_APB2_DIV_ ## v
#define apb2_prescaler(v) z_apb2_prescaler(v)

#define z_apb3_prescaler(v) LL_RCC_APB3_DIV_ ## v
#define apb3_prescaler(v) z_apb3_prescaler(v)

#define z_apb4_prescaler(v) LL_RCC_APB4_DIV_ ## v
#define apb4_prescaler(v) z_apb4_prescaler(v)

/* Macro to check for clock feasibility */
/* It is Cortex M7's responsibility to setup clock tree */
/* This check should only be performed for the M7 core code */
#ifdef CONFIG_CPU_CORTEX_M7

/* Choose PLL SRC */
#if defined(STM32_PLL_SRC_HSI)
#define PLLSRC_FREQ  ((STM32_HSI_FREQ)/(STM32_HSI_DIVISOR))
#elif defined(STM32_PLL_SRC_CSI)
#define PLLSRC_FREQ  STM32_CSI_FREQ
#elif defined(STM32_PLL_SRC_HSE)
#define PLLSRC_FREQ  STM32_HSE_FREQ
#else
#define PLLSRC_FREQ 0
#endif

/* Given source clock and dividers, computed the output frequency of PLLP */
#define PLLP_FREQ(pllsrc_freq, divm, divn, divp)	(((pllsrc_freq)*\
							(divn))/((divm)*(divp)))

/* PLL P output frequency value */
#define PLLP_VALUE	PLLP_FREQ(\
				PLLSRC_FREQ,\
				STM32_PLL_M_DIVISOR,\
				STM32_PLL_N_MULTIPLIER,\
				STM32_PLL_P_DIVISOR)

/* SYSCLKSRC before the D1CPRE prescaler */
#if defined(STM32_SYSCLK_SRC_PLL)
#define SYSCLKSRC_FREQ	PLLP_VALUE
#elif defined(STM32_SYSCLK_SRC_HSI)
#define SYSCLKSRC_FREQ	((STM32_HSI_FREQ)/(STM32_HSI_DIVISOR))
#elif defined(STM32_SYSCLK_SRC_CSI)
#define SYSCLKSRC_FREQ	STM32_CSI_FREQ
#elif defined(STM32_SYSCLK_SRC_HSE)
#define SYSCLKSRC_FREQ	STM32_HSE_FREQ
#endif

/* ARM Sys CPU Clock before HPRE prescaler */
#define SYSCLK_FREQ	((SYSCLKSRC_FREQ)/(STM32_D1CPRE))
#define AHB_FREQ	((SYSCLK_FREQ)/(STM32_HPRE))
#define APB1_FREQ	((AHB_FREQ)/(STM32_D2PPRE1))
#define APB2_FREQ	((AHB_FREQ)/(STM32_D2PPRE2))
#define APB3_FREQ	((AHB_FREQ)/(STM32_D1PPRE))
#define APB4_FREQ	((AHB_FREQ)/(STM32_D3PPRE))

/* Datasheet maximum frequency definitions */
#if defined(CONFIG_SOC_STM32H743XX) ||\
	defined(CONFIG_SOC_STM32H745XX) ||\
	defined(CONFIG_SOC_STM32H747XX) ||\
	defined(CONFIG_SOC_STM32H750XX) ||\
	defined(CONFIG_SOC_STM32H753XX)
/* All h7 SoC with maximum 480MHz SYSCLK */
#define SYSCLK_FREQ_MAX		480000000UL
#define AHB_FREQ_MAX		240000000UL
#define APBx_FREQ_MAX		120000000UL
#elif defined(CONFIG_SOC_STM32H723XX) ||\
	  defined(CONFIG_SOC_STM32H725XX) ||\
	  defined(CONFIG_SOC_STM32H735XX)
/* All h7 SoC with maximum 550MHz SYSCLK */
#define SYSCLK_FREQ_MAX		550000000UL
#define AHB_FREQ_MAX		275000000UL
#define APBx_FREQ_MAX		137500000UL
#elif defined(CONFIG_SOC_STM32H7A3XX) || defined(CONFIG_SOC_STM32H7A3XXQ) ||\
	  defined(CONFIG_SOC_STM32H7B3XX) || defined(CONFIG_SOC_STM32H7B3XXQ)
#define SYSCLK_FREQ_MAX		280000000UL
#define AHB_FREQ_MAX		280000000UL
#define APBx_FREQ_MAX		140000000UL
#else
/* Default: All h7 SoC with maximum 280MHz SYSCLK */
#define SYSCLK_FREQ_MAX		280000000UL
#define AHB_FREQ_MAX		140000000UL
#define APBx_FREQ_MAX		70000000UL
#endif

#if SYSCLK_FREQ > SYSCLK_FREQ_MAX
#error "SYSCLK frequency is too high!"
#endif
#if AHB_FREQ > AHB_FREQ_MAX
#error "AHB frequency is too high!"
#endif
#if APB1_FREQ > APBx_FREQ_MAX
#error "APB1 frequency is too high!"
#endif
#if APB2_FREQ > APBx_FREQ_MAX
#error "APB2 frequency is too high!"
#endif
#if APB3_FREQ > APBx_FREQ_MAX
#error "APB3 frequency is too high!"
#endif
#if APB4_FREQ > APBx_FREQ_MAX
#error "APB4 frequency is too high!"
#endif

#if SYSCLK_FREQ != CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
#error "SYS clock frequency for M7 core doesn't match CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC"
#endif

/* end of clock feasability check */
#endif /* CONFIG_CPU_CORTEX_M7 */


#if defined(CONFIG_CPU_CORTEX_M7)
#if STM32_D1CPRE > 1
/*
 * D1CPRE prescaler allows to set a HCLK frequency lower than SYSCLK frequency.
 * Though, zephyr doesn't make a difference today between these two clocks.
 * So, changing this prescaler is not allowed until it is made possible to
 * use them independently in zephyr clock subsystem.
 */
#error "D1CPRE presacler can't be higher than 1"
#endif
#endif /* CONFIG_CPU_CORTEX_M7 */

#if defined(CONFIG_CPU_CORTEX_M7)
/* Offset to access bus clock registers from M7 (or only) core */
#define STM32H7_BUS_CLK_REG	DT_REG_ADDR(DT_NODELABEL(rcc))
#elif defined(CONFIG_CPU_CORTEX_M4)
/* Offset to access bus clock registers from M4 core */
#define STM32H7_BUS_CLK_REG	DT_REG_ADDR(DT_NODELABEL(rcc)) + 0x60
#endif

static uint32_t get_bus_clock(uint32_t clock, uint32_t prescaler)
{
	return clock / prescaler;
}

__unused
static uint32_t get_pllout_frequency(uint32_t pllsrc_freq,
					    int pllm_div,
					    int plln_mul,
					    int pllout_div)
{
	__ASSERT_NO_MSG(pllm_div && pllout_div);

	return (pllsrc_freq * plln_mul) /
		(pllm_div * pllout_div);
}

__unused
static uint32_t get_pllsrc_frequency(void)
{
	switch (LL_RCC_PLL_GetSource()) {
	case LL_RCC_PLLSOURCE_HSI:
		return STM32_HSI_FREQ;
	case LL_RCC_PLLSOURCE_CSI:
		return STM32_CSI_FREQ;
	case LL_RCC_PLLSOURCE_HSE:
		return STM32_HSE_FREQ;
	case LL_RCC_PLLSOURCE_NONE:
	default:
		return 0;
	}
}

__unused
static uint32_t get_hclk_frequency(void)
{
	uint32_t sysclk = 0;

	/* Get the current system clock source */
	switch (LL_RCC_GetSysClkSource()) {
	case LL_RCC_SYS_CLKSOURCE_STATUS_HSI:
		sysclk = STM32_HSI_FREQ/STM32_HSI_DIVISOR;
		break;
	case LL_RCC_SYS_CLKSOURCE_STATUS_CSI:
		sysclk = STM32_CSI_FREQ;
		break;
	case LL_RCC_SYS_CLKSOURCE_STATUS_HSE:
		sysclk = STM32_HSE_FREQ;
		break;
#if defined(STM32_PLL_ENABLED)
	case LL_RCC_SYS_CLKSOURCE_STATUS_PLL1:
		sysclk = get_pllout_frequency(get_pllsrc_frequency(),
					      STM32_PLL_M_DIVISOR,
					      STM32_PLL_N_MULTIPLIER,
					      STM32_PLL_P_DIVISOR);
		break;
#endif /* STM32_PLL_ENABLED */
	}

	return get_bus_clock(sysclk, STM32_HPRE);
}

#if !defined(CONFIG_CPU_CORTEX_M4)

static int32_t prepare_regulator_voltage_scale(void)
{
	/* Apply system power supply configuration */
#if defined(SMPS) && defined(CONFIG_POWER_SUPPLY_DIRECT_SMPS)
	LL_PWR_ConfigSupply(LL_PWR_DIRECT_SMPS_SUPPLY);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_EXT_AND_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_EXT_AND_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_EXT_AND_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_EXT_AND_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_EXT)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_EXT);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_EXT)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_EXT);
#elif defined(CONFIG_POWER_SUPPLY_EXTERNAL_SOURCE)
	LL_PWR_ConfigSupply(LL_PWR_EXTERNAL_SOURCE_SUPPLY);
#else
	LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
#endif

	/* Make sure to put the CPU in highest Voltage scale during clock configuration */
	/* Highest voltage is SCALE0 */
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
	return 0;
}

static int32_t optimize_regulator_voltage_scale(uint32_t sysclk_freq)
{

	/* After sysclock is configured, tweak the voltage scale down */
	/* to reduce power consumption */

	/* Needs some smart work to configure properly */
	/* LL_PWR_REGULATOR_SCALE3 is lowest power consumption */
	/* Must be done in accordance to the Maximum allowed frequency vs VOS*/
	/* See RM0433 page 352 for more details */
#if defined(SMPS) && defined(CONFIG_POWER_SUPPLY_DIRECT_SMPS)
	LL_PWR_ConfigSupply(LL_PWR_DIRECT_SMPS_SUPPLY);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_EXT_AND_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_EXT_AND_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_EXT_AND_LDO)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_EXT_AND_LDO);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_1V8_SUPPLIES_EXT)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_1V8_SUPPLIES_EXT);
#elif defined(SMPS) && defined(CONFIG_POWER_SUPPLY_SMPS_2V5_SUPPLIES_EXT)
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_EXT);
#elif defined(CONFIG_POWER_SUPPLY_EXTERNAL_SOURCE)
	LL_PWR_ConfigSupply(LL_PWR_EXTERNAL_SOURCE_SUPPLY);
#else
	LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
#endif
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
	return 0;
}

__unused
static int get_vco_input_range(uint32_t m_div, uint32_t *range)
{
	uint32_t vco_freq;

	vco_freq = PLLSRC_FREQ / m_div;

	if (MHZ(1) <= vco_freq && vco_freq <= MHZ(2)) {
		*range = LL_RCC_PLLINPUTRANGE_1_2;
	} else if (MHZ(2) < vco_freq && vco_freq <= MHZ(4)) {
		*range = LL_RCC_PLLINPUTRANGE_2_4;
	} else if (MHZ(4) < vco_freq && vco_freq <= MHZ(8)) {
		*range = LL_RCC_PLLINPUTRANGE_4_8;
	} else if (MHZ(8) < vco_freq && vco_freq <= MHZ(16)) {
		*range = LL_RCC_PLLINPUTRANGE_8_16;
	} else {
		return -ERANGE;
	}

	return 0;
}

__unused
static uint32_t get_vco_output_range(uint32_t vco_input_range)
{
	if (vco_input_range == LL_RCC_PLLINPUTRANGE_1_2) {
		return LL_RCC_PLLVCORANGE_MEDIUM;
	}

	return LL_RCC_PLLVCORANGE_WIDE;
}

#endif /* ! CONFIG_CPU_CORTEX_M4 */

static inline int stm32_clock_control_on(const struct device *dev,
					 clock_control_subsys_t sub_system)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *)(sub_system);
	volatile uint32_t *reg;
	uint32_t reg_val;

	ARG_UNUSED(dev);

	if (IN_RANGE(pclken->bus, STM32_PERIPH_BUS_MIN, STM32_PERIPH_BUS_MAX) == 0) {
		/* Attemp to toggle a wrong periph clock bit */
		return -ENOTSUP;
	}

	z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

	reg = (uint32_t *)(STM32H7_BUS_CLK_REG + pclken->bus);
	reg_val = *reg;
	reg_val |= pclken->enr;
	*reg = reg_val;

	z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

	return 0;
}

static inline int stm32_clock_control_off(const struct device *dev,
					  clock_control_subsys_t sub_system)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *)(sub_system);
	volatile uint32_t *reg;
	uint32_t reg_val;

	ARG_UNUSED(dev);

	if (IN_RANGE(pclken->bus, STM32_PERIPH_BUS_MIN, STM32_PERIPH_BUS_MAX) == 0) {
		/* Attemp to toggle a wrong periph clock bit */
		return -ENOTSUP;
	}

	z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

	reg = (uint32_t *)(STM32H7_BUS_CLK_REG + pclken->bus);
	reg_val = *reg;
	reg_val &= ~pclken->enr;
	*reg = reg_val;

	z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

	return 0;
}

static int stm32_clock_control_get_subsys_rate(const struct device *clock,
					       clock_control_subsys_t sub_system,
					       uint32_t *rate)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *)(sub_system);
	/*
	 * Get AHB Clock (= SystemCoreClock = SYSCLK/prescaler)
	 * SystemCoreClock is preferred to CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
	 * since it will be updated after clock configuration and hence
	 * more likely to contain actual clock speed
	 */
#if defined(CONFIG_CPU_CORTEX_M4)
	uint32_t ahb_clock = SystemCoreClock;
#else
	uint32_t ahb_clock = get_bus_clock(SystemCoreClock, STM32_HPRE);
#endif
	uint32_t apb1_clock = get_bus_clock(ahb_clock, STM32_D2PPRE1);
	uint32_t apb2_clock = get_bus_clock(ahb_clock, STM32_D2PPRE2);
	uint32_t apb3_clock = get_bus_clock(ahb_clock, STM32_D1PPRE);
	uint32_t apb4_clock = get_bus_clock(ahb_clock, STM32_D3PPRE);

	ARG_UNUSED(clock);

	switch (pclken->bus) {
	case STM32_CLOCK_BUS_AHB1:
	case STM32_CLOCK_BUS_AHB2:
	case STM32_CLOCK_BUS_AHB3:
	case STM32_CLOCK_BUS_AHB4:
		*rate = ahb_clock;
		break;
	case STM32_CLOCK_BUS_APB1:
	case STM32_CLOCK_BUS_APB1_2:
		*rate = apb1_clock;
		break;
	case STM32_CLOCK_BUS_APB2:
		*rate = apb2_clock;
		break;
	case STM32_CLOCK_BUS_APB3:
		*rate = apb3_clock;
		break;
	case STM32_CLOCK_BUS_APB4:
		*rate = apb4_clock;
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

static struct clock_control_driver_api stm32_clock_control_api = {
	.on = stm32_clock_control_on,
	.off = stm32_clock_control_off,
	.get_rate = stm32_clock_control_get_subsys_rate,
};

__unused
static void set_up_fixed_clock_sources(void)
{

	if (IS_ENABLED(STM32_HSE_ENABLED)) {
		/* Enable HSE oscillator */
		if (IS_ENABLED(STM32_HSE_BYPASS)) {
			LL_RCC_HSE_EnableBypass();
		} else {
			LL_RCC_HSE_DisableBypass();
		}

		LL_RCC_HSE_Enable();
		while (LL_RCC_HSE_IsReady() != 1) {
		}
	}

	if (IS_ENABLED(STM32_HSI_ENABLED)) {
		/* Enable HSI oscillator */
		LL_RCC_HSI_Enable();
		while (LL_RCC_HSI_IsReady() != 1) {
		}
		/* HSI divider configuration */
		LL_RCC_HSI_SetDivider(hsi_divider(STM32_HSI_DIVISOR));
	}

	if (IS_ENABLED(STM32_CSI_ENABLED)) {
		/* Enable CSI oscillator */
		LL_RCC_CSI_Enable();
		while (LL_RCC_CSI_IsReady() != 1) {
		}
	}

	if (IS_ENABLED(STM32_LSI_ENABLED)) {
		/* Enable LSI oscillator */
		LL_RCC_LSI_Enable();
		while (LL_RCC_LSI_IsReady() != 1) {
		}
	}

	if (IS_ENABLED(STM32_LSE_ENABLED)) {
		/* Enable backup domain */
		LL_PWR_EnableBkUpAccess();

		/* Enable LSE oscillator */
		LL_RCC_LSE_Enable();
		while (LL_RCC_LSE_IsReady() != 1) {
		}
	}
}

__unused
static int set_up_plls(void)
{
#if defined(STM32_PLL_ENABLED) || defined(STM32_PLL3_ENABLED)
	int r;
	uint32_t vco_input_range;
	uint32_t vco_output_range;

	/* Configure PLL source */

	/* Can be HSE , HSI 64Mhz/HSIDIV, CSI 4MHz*/
	if (IS_ENABLED(STM32_PLL_SRC_HSE)) {
		/* Main PLL configuration and activation */
		LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
	} else if (IS_ENABLED(STM32_PLL_SRC_CSI)) {
		/* Main PLL configuration and activation */
		LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_CSI);
	} else if (IS_ENABLED(STM32_PLL_SRC_HSI)) {
		/* Main PLL configuration and activation */
		LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSI);
	} else {
		return -ENOTSUP;
	}

#if defined(STM32_PLL_ENABLED)
	r = get_vco_input_range(STM32_PLL_M_DIVISOR, &vco_input_range);
	if (r < 0) {
		return r;
	}

	vco_output_range = get_vco_output_range(vco_input_range);

	LL_RCC_PLL1_SetM(STM32_PLL_M_DIVISOR);

	LL_RCC_PLL1_SetVCOInputRange(vco_input_range);
	LL_RCC_PLL1_SetVCOOutputRange(vco_output_range);

	LL_RCC_PLL1_SetN(STM32_PLL_N_MULTIPLIER);

	/* FRACN disable DIVP,DIVQ,DIVR enable*/
	LL_RCC_PLL1FRACN_Disable();

	if (IS_ENABLED(STM32_PLL_P_ENABLED)) {
		LL_RCC_PLL1_SetP(STM32_PLL_P_DIVISOR);
		LL_RCC_PLL1P_Enable();
	}

	if (IS_ENABLED(STM32_PLL_Q_ENABLED)) {
		LL_RCC_PLL1_SetQ(STM32_PLL_Q_DIVISOR);
		LL_RCC_PLL1Q_Enable();
	}

	if (IS_ENABLED(STM32_PLL_R_ENABLED)) {
		LL_RCC_PLL1_SetR(STM32_PLL_R_DIVISOR);
		LL_RCC_PLL1R_Enable();
	}

	LL_RCC_PLL1_Enable();
	while (LL_RCC_PLL1_IsReady() != 1U) {
	}

#endif /* STM32_PLL_ENABLED */

#if defined(STM32_PLL3_ENABLED)
	r = get_vco_input_range(STM32_PLL3_M_DIVISOR, &vco_input_range);
	if (r < 0) {
		return r;
	}

	vco_output_range = get_vco_output_range(vco_input_range);

	LL_RCC_PLL3_SetM(STM32_PLL3_M_DIVISOR);

	LL_RCC_PLL3_SetVCOInputRange(vco_input_range);
	LL_RCC_PLL3_SetVCOOutputRange(vco_output_range);

	LL_RCC_PLL3_SetN(STM32_PLL3_N_MULTIPLIER);

	LL_RCC_PLL3FRACN_Disable();

	if (IS_ENABLED(STM32_PLL3_P_ENABLED)) {
		LL_RCC_PLL3_SetP(STM32_PLL3_P_DIVISOR);
		LL_RCC_PLL3P_Enable();
	}

	if (IS_ENABLED(STM32_PLL3_Q_ENABLED)) {
		LL_RCC_PLL3_SetQ(STM32_PLL3_Q_DIVISOR);
		LL_RCC_PLL3Q_Enable();
	}

	if (IS_ENABLED(STM32_PLL3_R_ENABLED)) {
		LL_RCC_PLL3_SetR(STM32_PLL3_R_DIVISOR);
		LL_RCC_PLL3R_Enable();
	}

	LL_RCC_PLL3_Enable();
	while (LL_RCC_PLL3_IsReady() != 1U) {
	}

#endif /* STM32_PLL3_ENABLED */

#else
	/* Init PLL source to None */
	LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_NONE);

#endif /* STM32_PLL_ENABLED || STM32_PLL3_ENABLED */

	return 0;
}

#if defined(CONFIG_CPU_CORTEX_M7)
static int stm32_clock_control_init(const struct device *dev)
{
	uint32_t old_hclk_freq = 0;
	uint32_t new_hclk_freq = 0;
	int r;

	ARG_UNUSED(dev);

	/* HW semaphore Clock enable */
#if defined(CONFIG_SOC_STM32H7A3XX) || defined(CONFIG_SOC_STM32H7A3XXQ) || \
	defined(CONFIG_SOC_STM32H7B3XX) || defined(CONFIG_SOC_STM32H7B3XXQ)
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_HSEM);
#else
	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_HSEM);
#endif

	z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

	/* Set up indiviual enabled clocks */
	set_up_fixed_clock_sources();

	/* Set up PLLs */
	r = set_up_plls();
	if (r < 0) {
		return r;
	}

	/* Configure Voltage scale to comply with the desired system frequency */
	prepare_regulator_voltage_scale();

	/* Current hclk value */
	old_hclk_freq = get_hclk_frequency();
	/* AHB is HCLK clock to configure */
	new_hclk_freq = get_bus_clock(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC,
				      STM32_HPRE);

	/* Set flash latency */
	/* AHB/AXI/HCLK clock is SYSCLK / HPRE */
	/* If freq increases, set flash latency before any clock setting */
	if (new_hclk_freq > old_hclk_freq) {
		LL_SetFlashLatency(new_hclk_freq);
	}

	/* Preset the prescalers prior to chosing SYSCLK */
	/* Prevents APB clock to go over limits */
	/* Set buses (Sys,AHB, APB1, APB2 & APB4) prescalers */
	LL_RCC_SetSysPrescaler(sysclk_prescaler(STM32_D1CPRE));
	LL_RCC_SetAHBPrescaler(ahb_prescaler(STM32_HPRE));
	LL_RCC_SetAPB1Prescaler(apb1_prescaler(STM32_D2PPRE1));
	LL_RCC_SetAPB2Prescaler(apb2_prescaler(STM32_D2PPRE2));
	LL_RCC_SetAPB3Prescaler(apb3_prescaler(STM32_D1PPRE));
	LL_RCC_SetAPB4Prescaler(apb4_prescaler(STM32_D3PPRE));

	/* Set up sys clock */
	if (IS_ENABLED(STM32_SYSCLK_SRC_PLL)) {
		/* Set PLL1 as System Clock Source */
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
		while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
		}
	} else if (IS_ENABLED(STM32_SYSCLK_SRC_HSE)) {
		/* Set sysclk source to HSE */
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
		while (LL_RCC_GetSysClkSource() !=
					LL_RCC_SYS_CLKSOURCE_STATUS_HSE) {
		}
	} else if (IS_ENABLED(STM32_SYSCLK_SRC_HSI)) {
		/* Set sysclk source to HSI */
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
		while (LL_RCC_GetSysClkSource() !=
					LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {
		}
	} else if (IS_ENABLED(STM32_SYSCLK_SRC_CSI)) {
		/* Set sysclk source to CSI */
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_CSI);
		while (LL_RCC_GetSysClkSource() !=
					LL_RCC_SYS_CLKSOURCE_STATUS_CSI) {
		}
	} else {
		return -ENOTSUP;
	}

	/* Set FLASH latency */
	/* AHB/AXI/HCLK clock is SYSCLK / HPRE */
	/* If freq not increased, set flash latency after all clock setting */
	if (new_hclk_freq <= old_hclk_freq) {
		LL_SetFlashLatency(new_hclk_freq);
	}

	optimize_regulator_voltage_scale(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);

	z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

	/* Update CMSIS variable */
	SystemCoreClock = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

	return r;
}
#else
static int stm32_clock_control_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* Update CMSIS variable */
	SystemCoreClock = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

	return 0;
}
#endif /* CONFIG_CPU_CORTEX_M7 */

/**
 * @brief RCC device, note that priority is intentionally set to 1 so
 * that the device init runs just after SOC init
 */
DEVICE_DT_DEFINE(DT_NODELABEL(rcc),
		    &stm32_clock_control_init,
		    NULL,
		    NULL, NULL,
		    PRE_KERNEL_1,
		    CONFIG_CLOCK_CONTROL_INIT_PRIORITY,
		    &stm32_clock_control_api);
