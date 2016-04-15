//
// Register Definition Header File, automatically generated by reg_tran.pl
//
#ifndef __MT_REG_BASE
#define __MT_REG_BASE

/* Please get register base from device tree. DO NOT define here */

#define BOOTROM_BASE (0x00000000)
#define BOOTSRAM_BASE (0x00100000)

// CCI400
#define CCI400_BASE (0xF0390000)

// APB Module cksys
#define CKSYS_BASE (0xF0000000)

// APB Module infracfg_ao
#define INFRACFG_AO_BASE (0xF0001000)

// APB Module pericfg
#define PERICFG_BASE (0xF0003000)

// APB Module dramc
#define DRAMC0_BASE (0xF0004000)

// APB Module gpio
//#define GPIO_BASE (0xF0005000)

// APB Module sleep
#define SLEEP_BASE (0xF0006000)

// APB Module toprgu
//#define TOPRGU_BASE (0xF0007000)

// APB Module apxgpt
//#define APXGPT_BASE (0xF0008000)

// APB Module rsvd
#define RSVD_BASE (0xF0009000)

// APB Module sej
#define SEJ_BASE (0xF000A000)

// APB Module ap_cirq_eint
#define APIRQ_BASE (0xF000B000)

// APB Module smi
#define SMI_COMMON_AO_BASE (0xF000C000)

// APB Module pmic_wrap
#define PWRAP_BASE (0xF000D000)

// APB Module device_apc_ao
#define DEVAPC_AO_BASE (0xF000E000)

// APB Module ddrphy
#define DDRPHY_BASE (0xF000F000)

// APB Module kp
#define KP_BASE (0xF0010000)

// APB Module DRAMC1_BASE
#define DRAMC1_BASE (0xF0011000)

// APB Module DDRPHY1_BASE
#define DDRPHY1_BASE (0xF0012000)

// APB Module md32
//#define MD32_BASE (0xF0050000)

// APB Module dbgapb
#define DBGAPB_BASE (0xF0100000)

// APB Module mcucfg
#define MCUCFG_BASE (0xF0200000)

// APB Module ca7mcucfg
#define CA7MCUCFG_BASE (0xF0200100)

// APB Module ca15l_config
#define CA15L_CONFIG_BASE (0xF0200200)

// APB Module infracfg
#define INFRACFG_BASE (0xF0201000)

// APB Module sramrom
#define SRAMROM_BASE (0xF0202000)

// APB Module emi
#define EMI_BASE (0xF0203000)

// APB Module sys_cirq
//#define SYS_CIRQ_BASE (0xF0204000)

// APB Module mm_iommu
#define M4U_BASE (0xF0205000)

// APB Module efusec
#define EFUSEC_BASE (0xF0206000)

// APB Module device_apc
#define DEVAPC_BASE (0xF0207000)

// APB Module bus_dbg_tracker_cfg
//#define BUS_DBG_BASE (0xF0208000)

// APB Module apmixed
#define APMIXED_BASE (0xF0209000)

// APB Module fhctl
#define FHCTL_BASE (0xF0209F00)

// APB Module ccif
#define AP_CCIF0_BASE (0xF020A000)

// APB Module ccif
#define MD_CCIF0_BASE (0xA020B000)

// APB Module gpio1
#define GPIO1_BASE (0xF020C000)

// APB Module infra_mbist
#define INFRA_MBIST_BASE (0xF020D000)

// APB Module dramc_conf_nao
#define DRAMC_NAO_BASE (0xF020E000)

// APB Module trng
#define TRNG_BASE (0xF020F000)

// APB Module gcpu
#define GCPU_BASE (0xF0210000)

// APB Module gcpu_ns
#define GCPU_NS_BASE (0xF0211000)

// APB Module gcpu_ns
#define GCE_BASE (0xF0212000)

// APB Module dramc_conf_nao
#define DRAMC1_NAO_BASE (0xF0213000)

// APB Module perisys_iommu
#define PERISYS_IOMMU_BASE (0xF0214000)

// APB Module mipi_tx_config
#define MIPI_TX0_BASE (0xF0215000)

// APB Module mipi_tx_config
#define MIPI_TX1_BASE (0xF0216000)

// MIPI TX Config
#define MIPI_TX_CONFIG_BASE	(0xF0012000)

// APB Module mipi_rx_ana_csi0
#define MIPI_RX_ANA_CSI0_BASE (0xF0217000)

// APB Module mipi_rx_ana_csi1
#define MIPI_RX_ANA_CSI1_BASE (0xF0218000)

// APB Module mipi_rx_ana_csi2
#define MIPI_RX_ANA_CSI2_BASE (0xF0219000)

// APB Module ca9
#define CA9_BASE (0xF0220000)

// APB Module gce
#define GCE_BASE (0xF0212000)

// APB Module cq_dma
//#define CQ_DMA_BASE (0xF0212c00)

// APB Module ap_dma
#define AP_DMA_BASE (0xF1000000)

// APB Module auxadc
#define AUXADC_BASE (0xF1001000)

// APB Module uart
#define AP_UART0_BASE (0xF1002000)

// APB Module uart
#define AP_UART1_BASE (0xF1003000)

// APB Module uart
#define AP_UART2_BASE (0xF1004000)

// APB Module uart
#define AP_UART3_BASE (0xF1005000)

// APB Module pwm
#define PWM_BASE (0xF1006000)

// APB Module i2c
#define I2C0_BASE (0xF1007000)

// APB Module i2c
#define I2C1_BASE (0xF1008000)

// APB Module i2c
#define I2C2_BASE (0xF1009000)

// APB Module spi
#define SPI1_BASE (0xF100A000)

// APB Module therm_ctrl
#define THERM_CTRL_BASE (0xF100B000)

// APB Module irda
#define IRDA_FRAMER_BASE (0xF100C000)

// APB Module nfi
#define NFI_BASE (0xF100D000)

// APB Module nfiecc
#define NFIECC_BASE (0xF100E000)

// APB Module nli_arb
#define NLI_ARB_BASE (0xF100F000)

// APB Module i2c
#define I2C3_BASE (0xF1010000)

// APB Module i2c
#define I2C4_BASE (0xF1011000)

// APB Module usb2
#define USB0_BASE (0xF1200000)

// APB Module usb_sif
#define USB_SIF_BASE (0xF1210000)

// APB Module audio
#define AUDIO_BASE (0xF1220000)

// APB Module msdc
#define MSDC0_BASE (0xF1230000)

// APB Module msdc
#define MSDC1_BASE (0xF1240000)

// APB Module msdc
#define MSDC2_BASE (0xF1250000)

// APB Module msdc
#define MSDC3_BASE (0xF1260000)

// APB Module ssusb_top
#define USB3_BASE (0xF1270000)

// APB Module ssusb_top_sif
#define USB3_SIF_BASE (0xF1280000)

// APB Module ssusb_top_sif2
#define USB3_SIF2_BASE (0xF1290000)

// APB Module mfg_top
#define MFGCFG_BASE (0xF3FFF000)

// APB Module han
#define HAN_BASE (0xF3000000)

// APB Module mmsys_config
//#define MMSYS_CONFIG_BASE (0xF4000000)

// APB Module mdp_rdma
//#define MDP_RDMA0_BASE (0xF4001000)

// APB Module mdp_rdma
//#define MDP_RDMA1_BASE (0xF4002000)

// APB Module mdp_rsz
//#define MDP_RSZ0_BASE (0xF4003000)

// APB Module mdp_rsz
//#define MDP_RSZ1_BASE (0xF4004000)

// APB Module mdp_rsz
//#define MDP_RSZ2_BASE (0xF4005000)

// APB Module disp_wdma
//#define MDP_WDMA_BASE (0xF4006000)

// APB Module mdp_wrot
//#define MDP_WROT0_BASE (0xF4007000)

// APB Module mdp_wrot
//#define MDP_WROT1_BASE (0xF4008000)

// APB Module mdp_tdshp
//#define MDP_TDSHP0_BASE (0xF4009000)

// APB Module mdp_tdshp
//#define MDP_TDSHP1_BASE (0xF400a000)

// APB Module mdp_tdshp
#define MDP_CROP_BASE (0xF400b000)

// APB Module ovl
#define OVL0_BASE (0xF400c000)

// APB Module ovl
#define OVL1_BASE (0xF400d000)

// APB Module disp_rdma
#define DISP_RDMA0_BASE (0xF400e000)

// APB Module disp_rdma
#define DISP_RDMA1_BASE (0xF400f000)

// APB Module disp_rdma
#define DISP_RDMA2_BASE (0xF4010000)

// APB Module disp_wdma
#define DISP_WDMA0_BASE (0xF4011000)

// APB Module disp_wdma
#define DISP_WDMA1_BASE (0xF4012000)

// APB Module disp_color_config
#define COLOR0_BASE (0xF4013000)

// APB Module disp_color_config
#define COLOR1_BASE (0xF4014000)

// APB Module disp_aal
#define DISP_AAL_BASE (0xF4015000)

// APB Module disp_gamma
#define DISP_GAMMA_BASE (0xF4016000)

// APB Module merge
#define DISP_MERGE_BASE (0xF4017000)

// APB Module split
#define DISP_SPLIT0_BASE (0xF4018000)

// APB Module split
#define DISP_SPLIT1_BASE (0xF4019000)

// APB Module disp_ufoe_config
#define DISP_UFOE_BASE (0xF401a000)

// APB Module dsi
#define DSI0_BASE (0xF401b000)

// APB Module dsi
#define DSI1_BASE (0xF401c000)

// APB Module disp_dpi
#define DPI_BASE (0xF401d000)

// APB Module disp_pwm
#define DISP_PWM0_BASE (0xF401e000)

// APB Module disp_pwm
#define DISP_PWM1_BASE (0xF401f000)

// APB Module disp_mutex
#define MM_MUTEX_BASE (0xF4020000)

// APB Module smi_larb
#define SMI_LARB0_BASE (0xF4021000)

// APB Module smi
#define SMI_COMMON_BASE (0xF4022000)

// APB Module disp_od
#define DISP_OD_BASE (0xF4023000)

// APB Module smi_larb
#define SMI_LARB2_BASE (0xF5001000)

// APB Module fake_eng
#define FAKE_ENG_BASE (0xF5002000)

// APB Module imgsys
#define IMGSYS_BASE (0xF5000000)

// APB Module cam1
#define CAM1_BASE (0xF5004000)

// APB Module cam2
#define CAM2_BASE (0xF5005000)

// APB Module cam3
#define CAM3_BASE (0xF5006000)

// APB Module cam4
#define CAM4_BASE (0xF5007000)

// APB Module camsv
#define CAMSV_BASE (0xF5009000)

// APB Module camsv_top
#define CAMSV_TOP_BASE (0xF5009000)

// APB Module csi2
#define CSI2_BASE (0xF5008000)

// APB Module seninf
#define SENINF_BASE (0xF5008000)

// APB Module seninf_tg
#define SENINF_TG_BASE (0xF5008000)

// APB Module seninf_top
#define SENINF_TOP_BASE (0xF5008000)

// APB Module seninf_mux
#define SENINF_MUX_BASE (0xF5008000)

// APB Module mipi_rx_config
#define MIPI_RX_CONFIG_BASE (0xF5008000)

// APB Module scam
#define SCAM_BASE (0xF5008C00)

// APB Module ncsi2
#define NCSI2_BASE (0xF5008000)

// APB Module ccir656
#define CCIR656_BASE (0xF5008000)

// APB Module n3d_ctl
#define N3D_CTL_BASE (0xF5008000)

// APB Module fdvt
#define FDVT_BASE (0xF500B000)

// APB Module vdecsys_config
#define VDEC_GCON_BASE (0xF6000000)

// APB Module smi_larb
#define SMI_LARB1_BASE (0xF6010000)

// APB Module vdtop
#define VDEC_BASE (0xF6020000)

// APB Module vdtop
#define VDTOP_BASE (0xF6020000)

// APB Module vld
#define VLD_BASE (0xF6021000)

// APB Module vld_top
#define VLD_TOP_BASE (0xF6021800)

// APB Module mc
#define MC_BASE (0xF6022000)

// APB Module avc_vld
#define AVC_VLD_BASE (0xF6023000)

// APB Module avc_mv
#define AVC_MV_BASE (0xF6024000)

// APB Module vdec_pp
#define VDEC_PP_BASE (0xF6025000)

// APB Module hevc_vld
#define HEVC_VLD_BASE (0xF6028000)

// APB Module vp8_vld
#define VP8_VLD_BASE (0xF6026800)

// APB Module vp6
#define VP6_BASE (0xF6027000)

// APB Module vld2
#define VLD2_BASE (0xF6027800)

// APB Module mc_vmmu
#define MC_VMMU_BASE (0xF6028000)

// APB Module pp_vmmu
#define PP_VMMU_BASE (0xF6029000)

// APB Module mjc_config
#define MJC_CONFIG_BASE (0xF7000000)

// APB Module mjc_top
#define MJC_TOP_BASE (0xF7001000)

// APB Module smi_larb
#define SMI_LARB4_BASE (0xF7002000)

// APB Module venc_config
//#define VENC_GCON_BASE (0xF8000000)

// APB Module smi_larb
#define SMI_LARB3_BASE (0xF8001000)

// APB Module venc
//#define VENC_BASE (0xF8002000)

// APB Module jpgenc
#define JPGENC_BASE (0xF8003000)

// APB Module jpgdec
#define JPGDEC_BASE (0xF8004000)

// APB Module audiosys
#define AUDIOSYS_BASE (0xF1220000)


//Marcos add for early porting
#define SYSRAM_BASE (0xF9000000)
//#define GIC_DIST_BASE (CA9_BASE + 0x1000)
//#define GIC_CPU_BASE  (CA9_BASE + 0x2000)
//#if defined(CONFIG_MTK_FPGA)
// FPGA only
#define DEVINFO_BASE (0xF0001000)
//#endif

#endif
