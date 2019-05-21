
#define PHY_ADDRESS							0x04

#define TX_ENA								0x00000001
#define RX_ENA								0x00000002
#define XON_GEN								0x00000004
#define ETH_SPEED							0x00000008
#define PROMIS_EN							0x00000010
#define PAD_EN								0x00000020
#define CRC_FWD								0x00000040
#define PAUSE_FWD							0x00000080
#define PAUSE_IGNORE						0x00000100
#define TX_ADR_INS							0x00000200
#define HD_ENA								0x00000400
#define EXCESS_COL							0x00000800		// RO
#define LATE_COL							0x00001000		// RO
#define SW_RESET							0x00002000
#define MHASH_SEL							0x00004000
#define LOOP_ENA							0x00008000
#define TX_ADR_SEL_0						0x00010000
#define TX_ADR_SEL_1						0x00020000
#define TX_ADR_SEL_2						0x00040000
#define MAGIC_ENA							0x00080000
#define SLEEP								0x00100000
#define WAKEUP								0x00200000
#define XOFF_GEN							0x00400000
#define CNTL_FRM_ENA						0x00800000
#define NO_LGTH_CHECK						0x01000000
#define ENA_10								0x02000000		// NOT USED
#define RX_ERR_DISC							0x04000000
#define DISABLE_READ_TIMEOUT				0x08000000
#define CNT_RESET							0x80000000

// TSE Register
#define TSE_REVISION						0x00
#define TSE_SCRATCH							0x01
#define TSE_COMMAND_CONFIG					0x02
#define TSE_MAC_0							0x03
#define TSE_MAC_1							0x04
#define TSE_FRM_LENGTH						0x05
#define TSE_PAUSE_QUANT						0x06
#define TSE_RX_SECTION_EMPTY				0x07
#define TSE_RX_SECTION_FULL					0x08
#define TSE_TX_SECTION_EMPTY				0x09
#define TSE_TX_SECTION_FULL					0x0A
#define TSE_RX_ALMOST_EMPTY					0x0B
#define TSE_RX_ALMOST_FULL					0x0C
#define TSE_TX_ALMOST_EMPTY					0x0D
#define TSE_TX_ALMOST_FULL					0x0E
#define TSE_MDIO_ADDR_0						0x0F
#define TSE_MDIO_ADDR_1						0x10
#define TSE_HOLDOFF_QUANT					0x11
#define TSE_TX_IPG_LENGTH					0x17
#define TSE_aMacID_0						0x18
#define TSE_aMacID_1						0x19
#define TSE_aFramesTransmittedOK			0x1A
#define TSE_aFramesReceivedOK				0x1B
#define TSE_aFrameCheckSequenceErrors		0x1C
#define TSE_aAlignmentErrors				0x1D
#define TSE_aOctetsTransmittedOK			0x1E
#define TSE_aOctetsReceivedOK				0x1F
#define TSE_aTxPAUSEMACCtrlFrames			0x20
#define TSE_aRxPAUSEMACCtrlFrames			0x21
#define TSE_ifInErrors						0x22
#define TSE_ifOutErrors						0x23
#define TSE_ifInUcastPkts					0x24
#define TSE_ifInMulticastPkts				0x25
#define TSE_ifInBroadcastPkts				0x26
#define TSE_ifOutDiscards					0x27
#define TSE_ifOutUcastPkts					0x28
#define TSE_ifOutMulticastPkts				0x29
#define TSE_ifOutBroadcastPkts				0x2A
#define TSE_etherStatsDropEvents			0x2B
#define TSE_etherStatsOctets				0x2C
#define TSE_etherStatsPkts					0x2D
#define TSE_etherStatsUndersizePkts			0x2E
#define TSE_etherStatsOversizePkts			0x2F
#define TSE_etherStatsPkts64Octets			0x30
#define TSE_etherStatsPkts65to127Octets		0x31
#define TSE_etherStatsPkts128to255Octets	0x32
#define TSE_etherStatsPkts256to511Octets	0x33
#define TSE_etherStatsPkts512to1023Octets	0x34
#define TSE_etherStatsPkts1024to1518Octets	0x35
#define TSE_etherStatsPkts1519toXOctets		0x36
#define TSE_etherStatsJabbers				0x37
#define TSE_etherStatsFragments				0x38
#define TSE_msb_aOctetsTransmittedOK		0x3C
#define TSE_msb_aOctetsReceivedOK			0x3D
#define TSE_msb_etherStatsOctets			0x3E
#define TSE_PHY_BASIC_CONTROL				0x80
#define TSE_PHY_BASIC_STATUS				0x81
#define TSE_PHY_IDENTIFIER_1				0x82
#define TSE_PHY_IDENTIFIER_2				0x83
#define TSE_PHY_AUTO_NEG_ADVERTISEMENT		0x84
#define TSE_PHY_AUTO_NEG_LINK_PARTNER_ABI	0x85
#define TSE_PHY_AUTO_NEG_EXPANSION			0x86
#define TSE_PHY_AUTO_NEG_NEXT_PAGE			0x87
#define TSE_PHY_AUTO_NEG_LINK_PARTNER_N_ABI	0x88
#define TSE_PHY_1000BASE_T_CONTR0L			0x89
#define TSE_PHY_1000BASE_T_STATUS			0x8A
#define TSE_PHY_MMD_ACCESS_CONTROL			0x8D
#define TSE_PHY_MMD_ACCESS_REGISTER_DATA	0x8E
#define TSE_PHY_EXTENDED_STATUS				0x8F
#define TSE_PHY_REMOTE_LOOPBACK				0x91
#define TSE_PHY_LINKMD_CABLE_DIAGNOSTIC		0x92
#define TSE_PHY_DIGITAL_PMA_PCS_STATUS		0x93
#define TSE_PHY_RX_ER_COUNTER				0x95
#define TSE_PHY_INTERRUPT_CONTROL_STATUS	0x9B
#define TSE_PHY_AUTO_MDI_MDIX				0x9C
#define TSE_PHY_CONTROL						0x9F

// PHY Basic Status
#define LINK_STATUS							0x004

// PHY Control
#define SPEED_STATUS_100					0x020
#define SPEED_STATUS_1000					0x040
