GPS/GNSS Tracker for STM32Primer2
====================================

Logging GNSS NMEA Datas using STM32 Primer2 hardware!

Functions
------
- General
-- 4-direction and center key inputs.  
-- Battery management with low voltage auto shutdown.  
-- Push key to select below three functions.  
-- Systick mSec Timer and uSec Timer(Using hardware timer).  
-- Indication LEDs.

- GNSS Data Logging  
-- Using ChaN's FatFs Filesystem.  
-- Using SDIO with DMA Driver.  
-- Supports MMC,SDSC,SDHC,SDXC and eMMC.  

- USB Mass Storage  
-- Using SDIO with DMA Driver.  
-- Supports MMC,SDSC,SDHC,SDXC and eMMC.  
-- Using doublebuffered endpoints(Bulk IN only).  
-- WR:0.55MB/Sec,RD:0.21MB/Sec(using sdsc 1GB card).  

- USB CDC Virtual Comport  
-- Using doublebuffered endpoints(Bulk IN only).  
-- Improved UART Rx performance and stability.  
-- It can send/receive datas to many GPS/GNSS Modules NMEA-0183 Sentences.
-- Supports 4800~115200Bps,8bit,noparity,1stopbit.  

Build
------
This program can build following compilers  
-GNU Tools for ARM Embedded Processors(Recommend)  
 https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm  

And MUST need Coreutils/GNUMake for windows to build this projects.  
 http://gnuwin32.sourceforge.net/packages/coreutils.htm  
 http://gnuwin32.sourceforge.net/packages/make.htm  
 
Misc
---------
see /doc/ReadFirst.txt  
see /doc/Whatnew.txt  

License
-------
see /doc/Copyright.txt