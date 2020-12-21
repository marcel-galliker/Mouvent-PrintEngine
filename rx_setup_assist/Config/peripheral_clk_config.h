/* Auto-generated config file peripheral_clk_config.h */
#ifndef PERIPHERAL_CLK_CONFIG_H
#define PERIPHERAL_CLK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

/**
 * \def CONF_HCLK_FREQUENCY
 * \brief HCLK's Clock frequency
 */
#ifndef CONF_HCLK_FREQUENCY
#define CONF_HCLK_FREQUENCY 300000000
#endif

/**
 * \def CONF_FCLK_FREQUENCY
 * \brief FCLK's Clock frequency
 */
#ifndef CONF_FCLK_FREQUENCY
#define CONF_FCLK_FREQUENCY 300000000
#endif

/**
 * \def CONF_CPU_FREQUENCY
 * \brief CPU's Clock frequency
 */
#ifndef CONF_CPU_FREQUENCY
#define CONF_CPU_FREQUENCY 300000000
#endif

/**
 * \def CONF_SLCK_FREQUENCY
 * \brief Slow Clock frequency
 */
#define CONF_SLCK_FREQUENCY 32000

/**
 * \def CONF_MCK_FREQUENCY
 * \brief Master Clock frequency
 */
#define CONF_MCK_FREQUENCY 150000000

/**
 * \def CONF_PCK6_FREQUENCY
 * \brief Programmable Clock Controller 6 frequency
 */
#define CONF_PCK6_FREQUENCY 12000000

// <h> SPI0 Clock Settings
// <y> SPI0 Clock source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <i> This defines the clock source for the SPI0
// <id> spi_clock_source
#ifndef CONF_SPI0_SRC
#define CONF_SPI0_SRC CONF_SRC_MCK
#endif
// </h>

/**
 * \def SPI0 FREQUENCY
 * \brief SPI0's Clock frequency
 */
#ifndef CONF_SPI0_FREQUENCY
#define CONF_SPI0_FREQUENCY 150000000
#endif

// <y> TC Clock Source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <id> tc_clock_source
// <i> Select the clock source for TC.
#ifndef CONF_TC0_SRC
#define CONF_TC0_SRC CONF_SRC_MCK
#endif

/**
 * \def CONF_TC0_FREQUENCY
 * \brief TC0's Clock frequency
 */
#ifndef CONF_TC0_FREQUENCY
#define CONF_TC0_FREQUENCY 150000000
#endif

// <y> TC Clock Source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <id> tc_clock_source
// <i> Select the clock source for TC.
#ifndef CONF_TC1_SRC
#define CONF_TC1_SRC CONF_SRC_MCK
#endif

/**
 * \def CONF_TC1_FREQUENCY
 * \brief TC1's Clock frequency
 */
#ifndef CONF_TC1_FREQUENCY
#define CONF_TC1_FREQUENCY 150000000
#endif

// <y> TC Clock Source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <id> tc_clock_source
// <i> Select the clock source for TC.
#ifndef CONF_TC2_SRC
#define CONF_TC2_SRC CONF_SRC_MCK
#endif

/**
 * \def CONF_TC2_FREQUENCY
 * \brief TC2's Clock frequency
 */
#ifndef CONF_TC2_FREQUENCY
#define CONF_TC2_FREQUENCY 150000000
#endif

// <y> TC Clock Source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <id> tc_clock_source
// <i> Select the clock source for TC.
#ifndef CONF_TC3_SRC
#define CONF_TC3_SRC CONF_SRC_MCK
#endif

/**
 * \def CONF_TC3_FREQUENCY
 * \brief TC3's Clock frequency
 */
#ifndef CONF_TC3_FREQUENCY
#define CONF_TC3_FREQUENCY 150000000
#endif

// <h> TWIHS Clock Settings
// <y> TWIHS Clock source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <i> This defines the clock source for the TWIHS
// <id> twihs_clock_source
#ifndef CONF_TWIHS0_SRC
#define CONF_TWIHS0_SRC CONF_SRC_MCK
#endif
// </h>

/**
 * \def TWIHS FREQUENCY
 * \brief TWIHS's Clock frequency
 */
#ifndef CONF_TWIHS0_FREQUENCY
#define CONF_TWIHS0_FREQUENCY 150000000
#endif

// <h> GMAC Clock Settings
// <y> GMAC Clock source
// <CONF_SRC_MCK"> Master Clock (MCK)
// <i> Select the clock source for GMAC
// <id> gmac_clock_source
#ifndef CONF_GMAC_SRC
#define CONF_GMAC_SRC CONF_SRC_MCK
#endif
// </h>

/**
 * \def GMAC FREQUENCY
 * \brief GMAC Clock frequency
 */
#ifndef CONF_GMAC_FREQUENCY
#define CONF_GMAC_FREQUENCY 150000000
#endif

// <<< end of configuration section >>>

#endif // PERIPHERAL_CLK_CONFIG_H
