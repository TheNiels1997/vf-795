/*
** $Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/gl_proc.c#1 $
*/

/*! \file   "gl_proc.c"
    \brief  This file defines the interface which can interact with users in /proc fs.

    Detail description.
*/



/*
** $Log: gl_proc.c $
 *
 * 11 10 2011 cp.wu
 * [WCXRP00001098] [MT6620 Wi-Fi][Driver] Replace printk by DBG LOG macros in linux porting layer
 * 1. eliminaite direct calls to printk in porting layer.
 * 2. replaced by DBGLOG, which would be XLOG on ALPS platforms.
 *
 * 12 10 2010 kevin.huang
 * [WCXRP00000128] [MT6620 Wi-Fi][Driver] Add proc support to Android Driver for debug and driver status check
 * Add Linux Proc Support
**  \main\maintrunk.MT5921\19 2008-09-02 21:08:37 GMT mtk01461
**  Fix the compile error of SPRINTF()
**  \main\maintrunk.MT5921\18 2008-08-10 18:48:28 GMT mtk01461
**  Update for Driver Review
**  \main\maintrunk.MT5921\17 2008-08-04 16:52:01 GMT mtk01461
**  Add proc dbg print message of DOMAIN_INDEX level
**  \main\maintrunk.MT5921\16 2008-07-10 00:45:16 GMT mtk01461
**  Remove the check of MCR offset, we may use the MCR address which is not align to DW boundary or proprietary usage.
**  \main\maintrunk.MT5921\15 2008-06-03 20:49:44 GMT mtk01461
**  \main\maintrunk.MT5921\14 2008-06-02 22:56:00 GMT mtk01461
**  Rename some functions for linux proc
**  \main\maintrunk.MT5921\13 2008-06-02 20:23:18 GMT mtk01461
**  Revise PROC mcr read / write for supporting TELNET
**  \main\maintrunk.MT5921\12 2008-03-28 10:40:25 GMT mtk01461
**  Remove temporary set desired rate in linux proc
**  \main\maintrunk.MT5921\11 2008-01-07 15:07:29 GMT mtk01461
**  Add User Update Desired Rate Set for QA in Linux
**  \main\maintrunk.MT5921\10 2007-12-11 00:11:14 GMT mtk01461
**  Fix SPIN_LOCK protection
**  \main\maintrunk.MT5921\9 2007-12-04 18:07:57 GMT mtk01461
**  Add additional debug category to proc
**  \main\maintrunk.MT5921\8 2007-11-02 01:03:23 GMT mtk01461
**  Unify TX Path for Normal and IBSS Power Save + IBSS neighbor learning
**  \main\maintrunk.MT5921\7 2007-10-25 18:08:14 GMT mtk01461
**  Add VOIP SCAN Support  & Refine Roaming
** Revision 1.3  2007/07/05 07:25:33  MTK01461
** Add Linux initial code, modify doc, add 11BB, RF init code
**
** Revision 1.2  2007/06/27 02:18:51  MTK01461
** Update SCAN_FSM, Initial(Can Load Module), Proc(Can do Reg R/W), TX API
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_os.h"
#include "gl_kal.h"

#include "wlan_lib.h"
#include "debug.h"


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define PROC_MCR_ACCESS                         "mcr"
#define PROC_DRV_STATUS                         "status"
#define PROC_RX_STATISTICS                      "rx_statistics"
#define PROC_TX_STATISTICS                      "tx_statistics"
#define PROC_DBG_LEVEL_NAME             "dbgLevel"
#define PROC_ROOT_NAME			"wlan"


#define PROC_MCR_ACCESS_MAX_USER_INPUT_LEN      20
#define PROC_RX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_TX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_DBG_LEVEL_MAX_USER_INPUT_LEN       20
#define PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN      30
#define PROC_UID_SHELL							2000
#define PROC_GID_WIFI							1010

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static UINT_32 u4McrOffset = 0;
static P_GLUE_INFO_T gprGlueInfo = NULL;
static struct proc_dir_entry *gprProcRoot;
static UINT_8 aucDbModuleName[][PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN] = {
	"INIT", "HAL", "INTR", "REQ", "TX", "RX", "RFTEST", "EMU", "SW1", "SW2",
	"SW3", "SW4", "HEM", "AIS", "RLM", "MEM", "CNM", "RSN", "BSS", "SCN",
	"SAA", "AAA", "P2P", "QM", "SEC", "BOW"
};
static UINT_8 aucDbgLevelBuff[1024];
extern UINT_8 aucDebugModule[];

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reading MCR register to User Space, the offset of
*        the MCR is specified in u4McrOffset.
*
* \param[in] page       Buffer provided by kernel.
* \param[in out] start  Start Address to read(3 methods).
* \param[in] off        Offset.
* \param[in] count      Allowable number to read.
* \param[out] eof       End of File indication.
* \param[in] data       Pointer to the private data structure.
*
* \return number of characters print to the buffer from User Space.
*/
/*----------------------------------------------------------------------------*/
static ssize_t
procMCRRead (struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    PARAM_CUSTOM_MCR_RW_STRUC_T rMcrInfo;
    UINT_32 u4BufLen = count;
    char p[50] = {0,};
    char *temp = &p[0];
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    // Kevin: Apply PROC read method 1.
    if (*f_pos > 0 || gprGlueInfo == NULL) {
        return 0; // To indicate end of file.
    }
 
    rMcrInfo.u4McrOffset = u4McrOffset;

    rStatus = kalIoctl(gprGlueInfo,
                        wlanoidQueryMcrRead,
                        (PVOID)&rMcrInfo,
                        sizeof(rMcrInfo),
                        TRUE,
                        TRUE,
                        TRUE,
                        FALSE,
                        &u4BufLen);


    SPRINTF(temp, ("MCR (0x%08xh): 0x%08x\n",
        rMcrInfo.u4McrOffset, rMcrInfo.u4McrData));

     u4BufLen = kalStrLen(p);
     if (u4BufLen > count)
	 u4BufLen = count;
    if (copy_to_user(buf, p, u4BufLen))
	return -EFAULT;
	
    *f_pos += u4BufLen;

    return (int)u4BufLen;

} /* end of procMCRRead() */


/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for writing MCR register to HW or update u4McrOffset
*        for reading MCR later.
*
* \param[in] file   pointer to file.
* \param[in] buffer Buffer from user space.
* \param[in] count  Number of characters to write
* \param[in] data   Pointer to the private data structure.
*
* \return number of characters write from User Space.
*/
/*----------------------------------------------------------------------------*/
static ssize_t
procMCRWrite (struct file *file, const char *buffer, unsigned long count, void *data)
{
    char acBuf[PROC_MCR_ACCESS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    int i4CopySize;
    PARAM_CUSTOM_MCR_RW_STRUC_T rMcrInfo;
    UINT_32 u4BufLen;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    i4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if (copy_from_user(acBuf, buffer, i4CopySize) || gprGlueInfo == NULL) {
        return 0;
    }
    acBuf[i4CopySize] = '\0';

    switch (sscanf(acBuf, "0x%x 0x%x",
                   &rMcrInfo.u4McrOffset, &rMcrInfo.u4McrData)) {
    case 2:
        /* NOTE: Sometimes we want to test if bus will still be ok, after accessing
         * the MCR which is not align to DW boundary.
         */
        //if (IS_ALIGN_4(rMcrInfo.u4McrOffset))
        { 
            u4McrOffset = rMcrInfo.u4McrOffset;

            //printk("Write 0x%lx to MCR 0x%04lx\n",
                //rMcrInfo.u4McrOffset, rMcrInfo.u4McrData);

            rStatus = kalIoctl(gprGlueInfo,
                                wlanoidSetMcrWrite,
                                (PVOID)&rMcrInfo,
                                sizeof(rMcrInfo),
                                FALSE,
                                FALSE,
                                TRUE,
                                FALSE,
                                &u4BufLen);

        }
        break;

    case 1:
        //if (IS_ALIGN_4(rMcrInfo.u4McrOffset))
        {
            u4McrOffset = rMcrInfo.u4McrOffset;
        }
        break;

    default:
        break;
    }

    return count;

} /* end of procMCRWrite() */

static const struct file_operations mcr_ops = {
	.owner = THIS_MODULE,
	.read = procMCRRead,
	.write = procMCRWrite,
};

#if 0
/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reading Driver Status to User Space.
*
* \param[in] page       Buffer provided by kernel.
* \param[in out] start  Start Address to read(3 methods).
* \param[in] off        Offset.
* \param[in] count      Allowable number to read.
* \param[out] eof       End of File indication.
* \param[in] data       Pointer to the private data structure.
*
* \return number of characters print to the buffer from User Space.
*/
/*----------------------------------------------------------------------------*/
static int
procDrvStatusRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = ((struct net_device *)data)->priv;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(data);

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }


    SPRINTF(p, ("GLUE LAYER STATUS:"));
    SPRINTF(p, ("\n=================="));

    SPRINTF(p, ("\n* Number of Pending Frames: %ld\n",
        prGlueInfo->u4TxPendingFrameNum));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryDrvStatusForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procDrvStatusRead() */


/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reading Driver RX Statistic Counters to User Space.
*
* \param[in] page       Buffer provided by kernel.
* \param[in out] start  Start Address to read(3 methods).
* \param[in] off        Offset.
* \param[in] count      Allowable number to read.
* \param[out] eof       End of File indication.
* \param[in] data       Pointer to the private data structure.
*
* \return number of characters print to the buffer from User Space.
*/
/*----------------------------------------------------------------------------*/
static int
procRxStatisticsRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = ((struct net_device *)data)->priv;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(data);

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }


    SPRINTF(p, ("RX STATISTICS (Write 1 to clear):"));
    SPRINTF(p, ("\n=================================\n"));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryRxStatisticsForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procRxStatisticsRead() */


/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reset Driver RX Statistic Counters.
*
* \param[in] file   pointer to file.
* \param[in] buffer Buffer from user space.
* \param[in] count  Number of characters to write
* \param[in] data   Pointer to the private data structure.
*
* \return number of characters write from User Space.
*/
/*----------------------------------------------------------------------------*/
static int
procRxStatisticsWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = ((struct net_device *)data)->priv;
    char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    UINT_32 u4ClearCounter;

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(data);

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    copy_from_user(acBuf, buffer, u4CopySize);
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%ld", &u4ClearCounter) == 1) {
        if (u4ClearCounter == 1) {
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

            wlanoidSetRxStatisticsForLinuxProc(prGlueInfo->prAdapter);

            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
        }
    }

    return count;

} /* end of procRxStatisticsWrite() */


/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reading Driver TX Statistic Counters to User Space.
*
* \param[in] page       Buffer provided by kernel.
* \param[in out] start  Start Address to read(3 methods).
* \param[in] off        Offset.
* \param[in] count      Allowable number to read.
* \param[out] eof       End of File indication.
* \param[in] data       Pointer to the private data structure.
*
* \return number of characters print to the buffer from User Space.
*/
/*----------------------------------------------------------------------------*/
static int
procTxStatisticsRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = ((struct net_device *)data)->priv;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(data);

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }


    SPRINTF(p, ("TX STATISTICS (Write 1 to clear):"));
    SPRINTF(p, ("\n=================================\n"));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryTxStatisticsForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procTxStatisticsRead() */


/*----------------------------------------------------------------------------*/
/*!
* \brief The PROC function for reset Driver TX Statistic Counters.
*
* \param[in] file   pointer to file.
* \param[in] buffer Buffer from user space.
* \param[in] count  Number of characters to write
* \param[in] data   Pointer to the private data structure.
*
* \return number of characters write from User Space.
*/
/*----------------------------------------------------------------------------*/
static int
procTxStatisticsWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = ((struct net_device *)data)->priv;
    char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    UINT_32 u4ClearCounter;

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(data);

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    copy_from_user(acBuf, buffer, u4CopySize);
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%ld", &u4ClearCounter) == 1) {
        if (u4ClearCounter == 1) {
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

            wlanoidSetTxStatisticsForLinuxProc(prGlueInfo->prAdapter);

            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
        }
    }

    return count;

} /* end of procTxStatisticsWrite() */
#endif

static ssize_t procDbgLevelRead(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	UINT_8 *temp = &aucDbgLevelBuff[0];
	UINT_32 u4CopySize = 0;
	UINT_32 i;

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0) {
		return 0;
	}
	SPRINTF(temp, ("\nERROR |WARN |STATE |EVENT |TRACE |INFO |LOUD |TEMP\n"
					"bit0 | bit1 | bit2 | bit3 | bit4 | bit5 |bit6 |bit7\n\nDebug Module\tIndex\tLevel\n\n"));
	for (i = 0; i < (sizeof(aucDbModuleName) / PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN); i++) {
		SPRINTF(temp, ("DBG_%s_IDX\t(0x%02x):\t0x%02x\n", &aucDbModuleName[i][0], i, aucDebugModule[i]));
	}
	u4CopySize = kalStrLen(aucDbgLevelBuff);
	if (u4CopySize > count)
		u4CopySize = count;
	if (copy_to_user(buf, aucDbgLevelBuff, u4CopySize) ) {
		pr_err("copy to user failed\n");
		return -EFAULT;
	}
	
	*f_pos += u4CopySize;
	return (ssize_t)u4CopySize;
}

static ssize_t procDbgLevelWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	UINT_32 u4NewDbgModule, u4NewDbgLevel;
	UINT_8 i = 0;
	UINT_32 u4CopySize = sizeof(aucDbgLevelBuff);
	UINT_8 *temp = &aucDbgLevelBuff[0];

	kalMemSet(aucDbgLevelBuff, 0, u4CopySize);
	if (u4CopySize >= count+1)
		u4CopySize = count;

	if (copy_from_user(aucDbgLevelBuff, buffer, u4CopySize)) {
		pr_err("error of copy from user\n");
		return -EFAULT;
	}
	aucDbgLevelBuff[u4CopySize] = '\0';

	while (temp) {
		if (sscanf(temp, "0x%x:0x%x", &u4NewDbgModule, &u4NewDbgLevel) != 2)  {
			pr_info("debug module and debug level should be one byte in length\n");
			break;
		}
		if (u4NewDbgModule >= DBG_MODULE_NUM) {
			pr_info("debug module index should less than %d\n", DBG_MODULE_NUM);
			break;
		}
		aucDebugModule[u4NewDbgModule] =  u4NewDbgLevel & DBG_CLASS_MASK;
		temp = kalStrChr(temp, ',');
		if (!temp)
			break;
		temp++; /* skip ',' */
	}
	return count;
}

static const struct file_operations dbglevel_ops = {
	.owner = THIS_MODULE,
	.read = procDbgLevelRead,
	.write = procDbgLevelWrite,
};

INT_32 procInitFs()
{
	struct proc_dir_entry *prEntry;

	if (init_net.proc_net == (struct proc_dir_entry *)NULL) {
		pr_err("init proc fs fail: proc_net == NULL\n");
		return -ENOENT;
	}

	/*
	 * Directory: Root (/proc/net/wlan0)
	 */

	gprProcRoot = proc_mkdir(PROC_ROOT_NAME, init_net.proc_net);
	if (!gprProcRoot) {
		pr_err("gprProcRoot == NULL\n");
		return -ENOENT;
	}
	proc_set_user(gprProcRoot, KUIDT_INIT(PROC_UID_SHELL), KGIDT_INIT(PROC_GID_WIFI));
	
	prEntry = proc_create(PROC_DBG_LEVEL_NAME, 0666, gprProcRoot, &dbglevel_ops);
	if (prEntry == NULL) {
		pr_err("Unable to create /proc entry dbgLevel\n\r");
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL), KGIDT_INIT(PROC_GID_WIFI));
	return 0;
}				/* end of procInitProcfs() */

INT_32 procUninitProcFs()
{
	remove_proc_entry(PROC_DBG_LEVEL_NAME, gprProcRoot);
	remove_proc_subtree(PROC_ROOT_NAME, init_net.proc_net);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function clean up a PROC fs created by procInitProcfs().
*
* \param[in] prDev      Pointer to the struct net_device.
* \param[in] pucDevName Pointer to the name of net_device.
*
* \return N/A
*/
/*----------------------------------------------------------------------------*/
INT_32 procRemoveProcfs()
{
	/* remove root directory (proc/net/wlan0) */
	/* remove_proc_entry(pucDevName, init_net.proc_net); */
	remove_proc_entry(PROC_MCR_ACCESS, gprProcRoot);
	gprGlueInfo = NULL;
	return 0;
}				/* end of procRemoveProcfs() */

INT_32 procCreateFsEntry(P_GLUE_INFO_T prGlueInfo)
{
	struct proc_dir_entry *prEntry;

	prGlueInfo->pProcRoot = gprProcRoot;
	gprGlueInfo = prGlueInfo;
	prEntry = proc_create(PROC_MCR_ACCESS, 0, gprProcRoot, &mcr_ops);
	if (prEntry == NULL) {
		printk("Unable to create /proc entry\n\r");
		return -1;
	}

	return 0;
}

