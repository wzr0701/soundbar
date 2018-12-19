/* config.h -- Autogenerated! Do not edit. */

#ifndef __INCLUDE_NUTTX_CONFIG_H
#define __INCLUDE_NUTTX_CONFIG_H

/* Architecture-specific options *************************/

#define CONFIG_SYSTEM_VERSION "zhuque_20180615_v"
#define CONFIG_PRODUCT_TYPE "XJ-1"
#define CONFIG_SDK_LEVEL "1"
#define CONFIG_SYSTEM_TYPE "R"
#define CONFIG_TIMR_STAMP "20181031.0352"
#define CONFIG_EXPERIMENTAL 1
#define CONFIG_HOST_LINUX 1
#define CONFIG_APPS_DIR "../cloudcard"
#define CONFIG_BUILD_FLAT 1
#define CONFIG_INTELHEX_BINARY 1
#define CONFIG_RAW_BINARY 1
#define CONFIG_ARCH_HAVE_HEAPCHECK 1
#define CONFIG_ARCH_HAVE_STACKCHECK 1
#define CONFIG_DEBUG_SYMBOLS 1
#define CONFIG_ARCH_HAVE_CUSTOMOPT 1
#define CONFIG_DEBUG_FULLOPT 1
#define CONFIG_UKERNEL_TYPE_BANANA 1
#define CONFIG_UKERNEL_TYPE banana
#define CONFIG_BUILTIN 1
#define CONFIG_DECOMPRESS 1
#define CONFIG_DECOMPRESS_LZ77 1
#define CONFIG_USE_LZ77 1
#define CONFIG_FS_READABLE 1
#define CONFIG_FS_WRITABLE 1
#define CONFIG_FS_MQUEUE_MPATH "/var/mqueue"
#define CONFIG_FS_FAT 1
#define CONFIG_FAT_LCNAMES 1
#define CONFIG_FAT_LFN 1
#define CONFIG_FAT_MAXFNAME 255
#define CONFIG_FAT_SURPPORT_UNICODE 1
#define CONFIG_FS_ROMFS 1
#define CONFIG_FS_BINFS 1
#define CONFIG_FS_UNIONFS 1
#define CONFIG_SYSLOG_TIMESTAMP 1
#define CONFIG_SYSLOG_DEFAULT_MASK 0x0
#define CONFIG_MM_REGIONS 1
#define CONFIG_MM_DETECT_ERROR 1
#define CONFIG_ARCH_HAVE_NET 1
#define CONFIG_ARCH_HAVE_PHY 1
#define CONFIG_NET_NULL 1
#define CONFIG_DISABLE_OS_API 1
#define CONFIG_USEC_PER_TICK 3000
#define CONFIG_CLOCK_MONOTONIC 1
#define CONFIG_START_YEAR 2014
#define CONFIG_START_MONTH 1
#define CONFIG_START_DAY 1
#define CONFIG_MAX_WDOGPARMS 2
#define CONFIG_PREALLOC_WDOGS 32
#define CONFIG_WDOG_INTRESERVE 4
#define CONFIG_PREALLOC_TIMERS 4
#define CONFIG_TIMER_PRECISON_CORRECT 1
#define CONFIG_INIT_ENTRYPOINT 1
#define CONFIG_USER_ENTRYPOINT nsh_main
#define CONFIG_INIT_MODULE_ENTRY 1
#define CONFIG_RR_INTERVAL 50
#define CONFIG_TASK_NAME_SIZE 31
#define CONFIG_MAX_TASKS 32
#define CONFIG_SCHED_WAITPID 1
#define CONFIG_NPTHREAD_KEYS 4
#define CONFIG_DEV_CONSOLE 1
#define CONFIG_NFILE_DESCRIPTORS 16
#define CONFIG_NFILE_STREAMS 8
#define CONFIG_NAME_MAX 128
#define CONFIG_KERNEL_INITTHREAD 1
#define CONFIG_KERNEL_INITTHREAD_STACKSIZE 4096
#define CONFIG_KERNEL_INITTHREAD_PRIORITY 192
#define CONFIG_BOARD_INITIALIZE 1
#define CONFIG_BOARD_INITTHREAD 1
#define CONFIG_BOARD_INITTHREAD_STACKSIZE 4096
#define CONFIG_BOARD_INITTHREAD_PRIORITY 192
#define CONFIG_SCHED_ATEXIT 1
#define CONFIG_SCHED_ATEXIT_MAX 1
#define CONFIG_SIG_SIGUSR1 1
#define CONFIG_SIG_SIGUSR2 2
#define CONFIG_SIG_SIGALARM 3
#define CONFIG_SIG_SIGCONDTIMEDOUT 16
#define CONFIG_SIG_SIGWORK 17
#define CONFIG_PREALLOC_MQ_MSGS 4
#define CONFIG_MQ_MAXMSGSIZE 32
#define CONFIG_SCHED_WORKQUEUE 1
#define CONFIG_SCHED_HPWORK 1
#define CONFIG_SCHED_HPWORKPRIORITY 192
#define CONFIG_SCHED_HPWORKPERIOD 50000
#define CONFIG_SCHED_HPWORKSTACKSIZE 2048
#define CONFIG_IDLETHREAD_STACKSIZE 4096
#define CONFIG_USERMAIN_STACKSIZE 8192
#define CONFIG_PTHREAD_STACK_MIN 256
#define CONFIG_PTHREAD_STACK_DEFAULT 8192
#define CONFIG_ARCH_CSKY 1
#define CONFIG_ARCH "csky"
#define CONFIG_ARCH_CSKYV1 1
#define CONFIG_ARCH_CK610 1
#define CONFIG_ARCH_FAMILY "cskyv1"
#define CONFIG_CKCPU_MMU 1
#define CONFIG_TLB_PAGESIZE_4MB 1
#define CONFIG_SECURITY_IMA 1
#define CONFIG_ARCH_HAVE_VFORK 1
#define CONFIG_BOARD_LOOPSPERMSEC 16717
#define CONFIG_BOARDCTL_POWEROFF 1
#define CONFIG_BOARDCTL_RESET 1
#define CONFIG_ARCH_HAVE_INTERRUPTSTACK 1
#define CONFIG_ARCH_BACKTRACE 1
#define CONFIG_ARCH_HAVE_HIPRI_INTERRUPT 1
#define CONFIG_RAM_START 0x0
#define CONFIG_RAM_SIZE 0x2000000
#define CONFIG_ARCH_CHIP_SC6138 1
#define CONFIG_ARCH_CHIP "sl_common"
#define CONFIG_CHIP_NAME "sc6138"
#define CONFIG_SILAN_BOARD 1
#define CONFIG_CHIP_SC6138 1
#define CONFIG_SILAN_ADC 1
#define CONFIG_SILAN_TIMER 1
#define CONFIG_SILAN_SPI_CTRL 1
#define CONFIG_SILAN_SD 1
#define CONFIG_SILAN_SD_ENABLE 1
#define CONFIG_SILAN_SDIO_CH2_ENABLE 1
#define CONFIG_SC6138_SPDIFIN 1
#define CONFIG_SILAN_I2C1 1
#define CONFIG_SILAN_I2C2 1
#define CONFIG_SC6138_WATCHDOG 1
#define CONFIG_SC6138_WDG_DEVPATH "/dev/watchdog0"
#define CONFIG_SILAN_RTC 1
#define CONFIG_SILAN_IR 1
#define CONFIG_SILAN_IR_GPIO 39
#define CONFIG_SILAN_IR_TIMER 3
#define CONFIG_SC6138_SPIFLASH 1
#define CONFIG_FLASH_WRITE_CHECK 1
#define CONFIG_KEY_TRANS 1
#define CONFIG_OS_TYPE_NORMAL 1
#define CONFIG_SYS_CONF 1
#define CONFIG_SE_GEMALTO_MTF008 1
#define CONFIG_NSECURE_STACKSIZE 0x10000
#define CONFIG_KERNEL_HEAPSIZE 0x200000
#define CONFIG_LD_TEXT_START 0x80000000
#define CONFIG_LD_TEXT_LENGTH 0x90000
#define CONFIG_LD_MEM_LENGTH 0x400000
#define CONFIG_LD_DATA_START 0xc0800000
#define CONFIG_LD_DATA_LENGTH 0x400000
#define CONFIG_ARCH_INTERRUPTSTACK 0
#define CONFIG_ARCH_EXCEPSTACK 8192
#define CONFIG_ARCH_BOARD_SC6138A_XJ_1 1
#define CONFIG_ARCH_BOARD "sc6138a_soundbox-8M"
#define CONFIG_ARCH_HAVE_BUTTONS 1
#define CONFIG_ARCH_BUTTONS 1
#define CONFIG_NSH_MMCSDMINOR 0
#define CONFIG_NSH_MMCSDSLOTNO 0
#define CONFIG_PM_BUTTON_ACTIVITY 10
#define CONFIG_PM_ALARM_SEC 15
#define CONFIG_PM_ALARM_NSEC 0
#define CONFIG_PM_SLEEP_WAKEUP_SEC 10
#define CONFIG_PM_SLEEP_WAKEUP_NSEC 0
#define CONFIG_LIB_BOARDCTL 1
#define CONFIG_DRIVER_ABSTRACT 1
#define CONFIG_DEV_NULL 1
#define CONFIG_PWM 1
#define CONFIG_POSIX_PWM 1
#define CONFIG_I2C 1
#define CONFIG_SPI 1
#define CONFIG_SPI_EXCHANGE 1
#define CONFIG_TIMER 1
#define CONFIG_POSIX_TIMER 1
#define CONFIG_MULTI_TIMER 1
#define CONFIG_RTC_DATETIME 1
#define CONFIG_RTC 1
#define CONFIG_POSIX_RTC 1
#define CONFIG_RTC_DRIVER 1
#define CONFIG_RTC_IOCTL 1
#define CONFIG_WATCHDOG 1
#define CONFIG_WATCHDOG_DEVPATH "/dev/watchdog0"
#define CONFIG_INPUT 1
#define CONFIG_INPUT_EVENT 1
#define CONFIG_MMCSD 1
#define CONFIG_MMCSD_NSLOTS 1
#define CONFIG_ARCH_HAVE_SDIO 1
#define CONFIG_MMCSD_SDIO 1
#define CONFIG_SDIO_DMA 1
#define CONFIG_SDIO_WIDTH_D1_ONLY 1
#define CONFIG_SDIO_BLOCKSETUP 1
#define CONFIG_SYSTEM_WIFI_SDIO 1
#define CONFIG_MTD 1
#define CONFIG_MTD_PARTITION 1
#define CONFIG_MTD_PARTITIONS 15
#define CONFIG_BTUPDATE1_IDX 1
#define CONFIG_BTUPDATE1_DEVNAME "/dev/mtd1"
#define CONFIG_BTUPDATE2_IDX 2
#define CONFIG_BTUPDATE2_DEVNAME "/dev/mtd2"
#define CONFIG_BOOTCONF_IDX 3
#define CONFIG_BOOTCONF_DEVNAME "/dev/mtd3"
#define CONFIG_KERNEL_IMG_IDX 4
#define CONFIG_KERNEL_IMG_DEVNAME "/dev/mtd4"
#define CONFIG_SYSCONF1_IDX 5
#define CONFIG_SYSCONF1_DEVNAME "/dev/mtd5"
#define CONFIG_SYSCONF2_IDX 6
#define CONFIG_SYSCONF2_DEVNAME "/dev/mtd6"
#define CONFIG_SYSDATA_IDX 7
#define CONFIG_SYSDATA_DEVNAME "/dev/mtd7"
#define CONFIG_WIFIBIN_IDX 8
#define CONFIG_WIFIBIN_DEVNAME "/dev/mtd8"
#define CONFIG_CRASH_LOG_IDX 9
#define CONFIG_CRASH_LOG_PATH "/dev/mtd9"
#define CONFIG_USER_IDX 10
#define CONFIG_USER_DEVNAME "/dev/mtd10"
#define CONFIG_USER_DEVNAME_BLK "/dev/mtdblock10"
#define CONFIG_TONE_IDX 11
#define CONFIG_TONE_DEVNAME "/dev/mtd11"
#define CONFIG_TBL_IDX 12
#define CONFIG_TBL_DEVNAME "/dev/mtd12"
#define CONFIG_CH_FNT_IDX 13
#define CONFIG_CH_FNT_DEVNAME "/dev/mtd13"
#define CONFIG_BACKUP_IDX 14
#define CONFIG_BACKUP_DEVNAME "/dev/mtd14"
#define CONFIG_PM 1
#define CONFIG_POWER 1
#define CONFIG_PM_WAIT 1
#define CONFIG_PM_TIME_THRESHOLD 1800
#define CONFIG_PM_TIME_THRESHOLD_AFTER_WAKEUP 10
#define CONFIG_SERIAL 1
#define CONFIG_CONSOLE_UART 2
#define CONFIG_MCU_SERIAL 1
#define CONFIG_SYSTEM_UARTS 4
#define CONFIG_USART_BAUD 115200
#define CONFIG_USART_BITS 8
#define CONFIG_USART_PARITY 0
#define CONFIG_USART_2STOP 0
#define CONFIG_USART1_RXBUFSIZE 128
#define CONFIG_USART1_TXBUFSIZE 128
#define CONFIG_USART2_RXBUFSIZE 128
#define CONFIG_USART2_TXBUFSIZE 128
#define CONFIG_USART3_RXBUFSIZE 128
#define CONFIG_USART3_TXBUFSIZE 128
#define CONFIG_USART4_RXBUFSIZE 128
#define CONFIG_USART4_TXBUFSIZE 128
#define CONFIG_STANDARD_SERIAL 1
#define CONFIG_SERIAL_NPOLLWAITERS 2
#define CONFIG_ARCH_HAVE_SERIAL_TERMIOS 1
#define CONFIG_SERIAL_TERMIOS 1
#define CONFIG_NO_SERIAL_CONSOLE 1
#define CONFIG_SYSTEM_UART1 1
#define CONFIG_SYSTEM_UART2 1
#define CONFIG_SYSTEM_UART3 1
#define CONFIG_SYSTEM_UART4 1
#define CONFIG_USBHOST 1
#define CONFIG_USBHOST_NPREALLOC 4
#define CONFIG_USBHOST_MSC 1
#define CONFIG_SILAN_USBHS 1
#define CONFIG_DRIVER_HAL 1
#define CONFIG_IDJS_AUTO_START 1
#define CONFIG_YUNIO_TYPE cmns
#define CONFIG_AUDIO 1
#define CONFIG_AUDIO_NUM_BUFFERS 2
#define CONFIG_AUDIO_BUFFER_NUMBYTES 8192
#define CONFIG_AUDIO_FORMAT_PCM 1
#define CONFIG_AUDIO_FORMAT_MP3 1
#define CONFIG_AUDIO_EXCLUDE_EQUALIZER 1
#define CONFIG_AUDIO_EXCLUDE_REWIND 1
#define CONFIG_CRYPTO 1
#define CONFIG_CRYPTO_SW_AES 1
#define CONFIG_CRYPTO_SW_DES 1
#define CONFIG_CRYPTO_HW_AES 1
#define CONFIG_CRYPTO_HW_DES 1
#define CONFIG_CRYPTO_HW_AES_CBC 1
#define CONFIG_STDIO_BUFFER_SIZE 0
#define CONFIG_STDIO_LINEBUFFER 1
#define CONFIG_NUNGET_CHARS 2
#define CONFIG_LIB_HOMEDIR "/"
#define CONFIG_LIBM 1
#define CONFIG_LIBC_LONG_LONG 1
#define CONFIG_LIBC_TZ_MAX_TIMES 370
#define CONFIG_LIBC_TZ_MAX_TYPES 20
#define CONFIG_LIBC_TZDIR "/etc/zoneinfo"
#define CONFIG_LIB_RAND_ORDER 1
#define CONFIG_EOL_IS_EITHER_CRLF 1
#define CONFIG_POSIX_SPAWN_PROXY_STACKSIZE 1024
#define CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE 2048
#define CONFIG_LIBC_TMPDIR "/tmp"
#define CONFIG_LIBC_MAX_TMPFILE 32
#define CONFIG_ARCH_LOWPUTC 1
#define CONFIG_TIME_EXTENDED 1
#define CONFIG_LIB_SENDFILE_BUFSIZE 512
#define CONFIG_ARCH_OPTIMIZED_FUNCTIONS 1
#define CONFIG_ARCH_MEMCPY 1
#define CONFIG_ARCH_MEMCMP 1
#define CONFIG_ARCH_MEMMOVE 1
#define CONFIG_MEMSET_OPTSPEED 1
#define CONFIG_ARCH_STRCAT 1
#define CONFIG_ARCH_STRNCAT 1
#define CONFIG_ARCH_STRCHR 1
#define CONFIG_ARCH_STRCMP 1
#define CONFIG_ARCH_STRCPY 1
#define CONFIG_ARCH_STRNCPY 1
#define CONFIG_ARCH_STRLEN 1
#define CONFIG_HAVE_CXX 1
#define CONFIG_HAVE_CXXINITIALIZE 1
#define CONFIG_LIB_INETUTILS 1
#define CONFIG_CJSON_WITHOUT_DOUBLE 1
#define CONFIG_NETUTILS_JSON 1
#define CONFIG_NETUTILS_CODECS 1
#define CONFIG_CODECS_BASE64 1
#define CONFIG_CODECS_HASH_MD5 1
#define CONFIG_CODECS_URLCODE_NEWMEMORY 1
#define CONFIG_LIB_IUV 1
#define CONFIG_LIBS_LIBIUV_MODULE_TIMER 1
#define CONFIG_LIBS_LIBIUV_MODULE_ASYNC 1
#define CONFIG_LIBS_LIBIUV_MODULE_THREADPOOL 1
#define CONFIG_LIBS_LIBIUV_MODULE_FS 1
#define CONFIG_LIBS_LIBIUV_MODULE_ERROR 1
#define CONFIG_UV_QUEUE_CONNECT 1
#define CONFIG_BUILTIN_PROXY_STACKSIZE 1024
#define CONFIG_EXAMPLES_NSH 1
#define CONFIG_NSH_LIBRARY 1
#define CONFIG_NSH_READLINE 1
#define CONFIG_NSH_LINELEN 256
#define CONFIG_NSH_MAXARGUMENTS 6
#define CONFIG_NSH_NESTDEPTH 3
#define CONFIG_NSH_BUILTIN_APPS 1
#define CONFIG_NSH_CODECS_BUFSIZE 128
#define CONFIG_NSH_FILEIOSIZE 4096
#define CONFIG_NSH_CONSOLE 1
#define CONFIG_SYSTEM_EVMANAGER 1
#define CONFIG_SYSTEM_FSMGR 1
#define CONFIG_SYSTEM_NXPLAYER 1
#define CONFIG_SPDIFIN 1
#define CONFIG_NXPLAYER_FILE_READ_LIBUV 1
#define CONFIG_LIB_MEDIA 1
#define CONFIG_NXPLAYER_PLAYTHREAD_STACKSIZE 1500
#define CONFIG_NXPLAYER_COMMAND_LINE 1
#define CONFIG_PLAYER_BUFFER_LEN 1024
#define CONFIG_SYSTEM_READLINE 1
#define CONFIG_READLINE_ECHO 1
#define CONFIG_READLINE_TABCOMPLETION 1
#define CONFIG_READLINE_MAX_BUILTINS 64
#define CONFIG_READLINE_CMD_HISTORY 1
#define CONFIG_READLINE_CMD_HISTORY_LINELEN 80
#define CONFIG_READLINE_CMD_HISTORY_LEN 16
#define CONFIG_SYSTEM_SYSRESET 1
#define CONFIG_SYSTEM_POWER_MANAGER 1
#define CONFIG_UI 1
#define CONFIG_FLASH_4M 1
#define CONFIG_FAT32_FILELIST 1
#define CONFIG_CEC 1
#define CONFIG_CEC_CMD_GPIO 0
#define CONFIG_CEC_DET_GPIO 46
#define CONFIG_CEC_TIMER_ID 2
#define CONFIG_SYSTEM_DEBUG 1
#define CONFIG_EXAMPLES_SLADSP 1

/* Sanity Checks *****************************************/

/* If this is an NXFLAT, external build, then make sure that
 * NXFLAT support is enabled in the base code.
 */

#if defined(__NXFLAT__) && !defined(CONFIG_NXFLAT)
# error "NXFLAT support not enabled in this configuration"
#endif

/* NXFLAT requires PIC support in the TCBs. */

#if defined(CONFIG_NXFLAT)
# undef CONFIG_PIC
# define CONFIG_PIC 1
#endif

/* Binary format support is disabled if no binary formats are
 * configured (at present, NXFLAT is the only supported binary.
 * format).
 */

#if !defined(CONFIG_NXFLAT) && !defined(CONFIG_ELF) && !defined(CONFIG_BUILTIN)
# undef CONFIG_BINFMT_DISABLE
# define CONFIG_BINFMT_DISABLE 1
#endif

/* The correct way to disable RR scheduling is to set the
 * timeslice to zero.
 */

#ifndef CONFIG_RR_INTERVAL
# define CONFIG_RR_INTERVAL 0
#endif

/* The correct way to disable filesystem supuport is to set the number of
 * file descriptors to zero.
 */

#ifndef CONFIG_NFILE_DESCRIPTORS
# define CONFIG_NFILE_DESCRIPTORS 0
#endif

/* If a console is selected, then make sure that there are resources for
 * three file descriptors and, if any streams are selected, also for three
 * file streams.
 *
 * CONFIG_DEV_CONSOLE means that a builtin console device exists at /dev/console
 * and can be opened during boot-up.  Other consoles, such as USB consoles, may
 * not exist at boot-upand have to be handled in a different way.  Three file
 * descriptors and three file streams are still needed.
 */

#if defined(CONFIG_DEV_CONSOLE) || defined(CONFIG_CDCACM_CONSOLE) || \
    defined(CONFIG_PL2303_CONSOLE)
# if CONFIG_NFILE_DESCRIPTORS < 3
#   undef CONFIG_NFILE_DESCRIPTORS
#   define CONFIG_NFILE_DESCRIPTORS 3
# endif

# if CONFIG_NFILE_STREAMS > 0 && CONFIG_NFILE_STREAMS < 3
#  undef CONFIG_NFILE_STREAMS
#  define CONFIG_NFILE_STREAMS 3
# endif

/* If no console is selected, then disable all builtin console devices */

#else
#  undef CONFIG_DEV_LOWCONSOLE
#  undef CONFIG_RAMLOG_CONSOLE
#endif

/* If priority inheritance is disabled, then do not allocate any
 * associated resources.
 */

#if !defined(CONFIG_PRIORITY_INHERITANCE) || !defined(CONFIG_SEM_PREALLOCHOLDERS)
# undef CONFIG_SEM_PREALLOCHOLDERS
# define CONFIG_SEM_PREALLOCHOLDERS 0
#endif

#if !defined(CONFIG_PRIORITY_INHERITANCE) || !defined(CONFIG_SEM_NNESTPRIO)
# undef CONFIG_SEM_NNESTPRIO
# define CONFIG_SEM_NNESTPRIO 0
#endif

/* If no file descriptors are configured, then make certain no
 * streams are configured either.
 */

#if CONFIG_NFILE_DESCRIPTORS == 0
# undef CONFIG_NFILE_STREAMS
# define CONFIG_NFILE_STREAMS 0
#endif

/* There must be at least one memory region. */

#ifndef CONFIG_MM_REGIONS
# define CONFIG_MM_REGIONS 1
#endif

/* If the end of RAM is not specified then it is assumed to be the beginning
 * of RAM plus the RAM size.
 */

#ifndef CONFIG_RAM_END
# define CONFIG_RAM_END (CONFIG_RAM_START+CONFIG_RAM_SIZE)
#endif

#ifndef CONFIG_RAM_VEND
# define CONFIG_RAM_VEND (CONFIG_RAM_VSTART+CONFIG_RAM_SIZE)
#endif

/* If the end of FLASH is not specified then it is assumed to be the beginning
 * of FLASH plus the FLASH size.
 */

#ifndef CONFIG_FLASH_END
# define CONFIG_FLASH_END (CONFIG_FLASH_START+CONFIG_FLASH_SIZE)
#endif

/* If no file streams are configured, then make certain that buffered I/O
 * support is disabled
 */

#if CONFIG_NFILE_STREAMS == 0
# undef CONFIG_STDIO_BUFFER_SIZE
# define CONFIG_STDIO_BUFFER_SIZE 0
#endif

/* If no standard C buffered I/O is not supported, then line-oriented buffering
 * cannot be supported.
 */

#if CONFIG_STDIO_BUFFER_SIZE == 0
# undef CONFIG_STDIO_LINEBUFFER
#endif

/* If the maximum message size is zero, then we assume that message queues
 * support should be disabled
 */

#if !defined(CONFIG_MQ_MAXMSGSIZE) || defined(CONFIG_DISABLE_MQUEUE)
# undef CONFIG_MQ_MAXMSGSIZE
# define CONFIG_MQ_MAXMSGSIZE 0
#endif

#if CONFIG_MQ_MAXMSGSIZE <= 0 && !defined(CONFIG_DISABLE_MQUEUE)
# define CONFIG_DISABLE_MQUEUE 1
#endif

/* If mountpoint support in not included, then no filesystem can be supported */

#ifdef CONFIG_DISABLE_MOUNTPOINT
# undef CONFIG_FS_FAT
# undef CONFIG_FS_ROMFS
# undef CONFIG_FS_NXFFS
# undef CONFIG_FS_SMARTFS
# undef CONFIG_FS_BINFS
# undef CONFIG_NFS
#endif

/* Check if any readable and writable filesystem (OR USB storage) is supported */

#if defined(CONFIG_FS_FAT) || defined(CONFIG_FS_ROMFS) || defined(CONFIG_USBMSC) || \
    defined(CONFIG_FS_NXFFS) || defined(CONFIG_FS_SMARTFS) || defined(CONFIG_FS_BINFS) || \
    defined(CONFIG_NFS) || defined(CONFIG_FS_PROCFS)
# undef  CONFIG_FS_READABLE
# define CONFIG_FS_READABLE 1
#endif

#if defined(CONFIG_FS_FAT) || defined(CONFIG_USBMSC) || defined(CONFIG_FS_NXFFS) || \
    defined(CONFIG_FS_SMARTFS) || defined(CONFIG_NFS)
# undef  CONFIG_FS_WRITABLE
# define CONFIG_FS_WRITABLE 1
#endif

/* There can be no network support with no socket descriptors */

#if CONFIG_NSOCKET_DESCRIPTORS <= 0
# undef CONFIG_NET
#endif

/* Conversely, if there is no network support, there is no need for
 * socket descriptors
 */

#ifndef CONFIG_NET
# undef CONFIG_NSOCKET_DESCRIPTORS
# define CONFIG_NSOCKET_DESCRIPTORS 0
#endif

/* Protocol support can only be provided on top of basic network support */

#ifndef CONFIG_NET
# undef CONFIG_NET_TCP
# undef CONFIG_NET_UDP
# undef CONFIG_NET_ICMP
#endif

/* NFS client can only be provided on top of UDP network support */

#if !defined(CONFIG_NET) || !defined(CONFIG_NET_UDP)
# undef CONFIG_NFS
#endif

/* Verbose debug and sub-system debug only make sense if debug is enabled */

#ifndef CONFIG_DEBUG
# undef CONFIG_DEBUG_VERBOSE
# undef CONFIG_DEBUG_SCHED
# undef CONFIG_DEBUG_MM
# undef CONFIG_DEBUG_PAGING
# undef CONFIG_DEBUG_DMA
# undef CONFIG_DEBUG_FS
# undef CONFIG_DEBUG_LIB
# undef CONFIG_DEBUG_BINFMT
# undef CONFIG_DEBUG_NET
# undef CONFIG_DEBUG_USB
# undef CONFIG_DEBUG_GRAPHICS
# undef CONFIG_DEBUG_GPIO
# undef CONFIG_DEBUG_SPI
# undef CONFIG_DEBUG_HEAP
#endif

/* User entry point. This is provided as a fall-back to keep compatibility
 * with existing code, for builds which do not define CONFIG_USER_ENTRYPOINT.
 */

#ifndef CONFIG_USER_ENTRYPOINT
# define CONFIG_USER_ENTRYPOINT main
#endif

#endif /* __INCLUDE_NUTTX_CONFIG_H */
