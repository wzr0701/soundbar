#ifndef MCU1007P_H
#define MCU1007P_H

/*说明:
[2010.4.26]
  1 前面小x打头的寄存器表示需要由dptr来访问;
  2 符号ASM: 用户程序若使用的是汇编语言,应定义ASM为1,即#define	ASM 1, C语言应使ASM不为1;
  3 符号EXSFR: EXSFR定义为1,增加扩展的寄存器定义;
*/

#define	ASM 0			// =1,用户编译的是汇编语言
#define	EXSFR 0			// =1,表示使用扩展寄存器

/*  BYTE Register  */

sfr P0   	= 0x80;
sfr P1   	= 0x90;
sfr P2   	= 0xA0;
sfr P3   	= 0xB0;
sfr PSW  	= 0xD0;
sfr ACC  	= 0xE0;
sfr B    	= 0xF0;
sfr SP   	= 0x81;
sfr DPL  	= 0x82;
sfr DPH  	= 0x83;
sfr PCON 	= 0x87;
sfr TCON 	= 0x88;
sfr TMOD 	= 0x89;
sfr TL0  	= 0x8A;
sfr TL1  	= 0x8B;
sfr TH0  	= 0x8C;
sfr TH1  	= 0x8D;
sfr IE   	= 0xA8;
sfr IP   	= 0xB8;

/*  BIT Register  */

/*	P0    */
sbit	P00 = P0^0;
sbit	P01 = P0^1;
sbit	P02 = P0^2;
sbit	P03 = P0^3;
sbit	P04 = P0^4;
sbit	P05 = P0^5;
sbit	P06 = P0^6;
sbit	P07 = P0^7;

/*	P1    */
sbit	P10 = P1^0;
sbit	P11 = P1^1;
sbit	P12 = P1^2;
sbit	P13 = P1^3;
sbit	P14 = P1^4;
sbit	P15 = P1^5;
sbit	P16 = P1^6;
sbit	P17 = P1^7;

/*	P2    */
sbit	P20 = P2^0;
sbit	P21 = P2^1;
sbit	P22 = P2^2;
sbit	P23 = P2^3;
sbit	P24 = P2^4;
sbit	P25 = P2^5;
sbit	P26 = P2^6;
sbit	P27 = P2^7;

/*	P3    */
sbit	P30 = P3^0;
sbit	P31 = P3^1;
sbit	P32 = P3^2;
sbit	P33 = P3^3;
sbit	P34 = P3^4;
sbit	P35 = P3^5;
sbit	P36 = P3^6;
sbit	P37 = P3^7;

/*  PSW   */
sbit CY   = 0xD7;
sbit AC   = 0xD6;
sbit F0   = 0xD5;
sbit RS1  = 0xD4;
sbit RS0  = 0xD3;
sbit OV   = 0xD2;
sbit P    = 0xD0;

/*  TCON  */
sbit TF1  = 0x8F;
sbit TR1  = 0x8E;
sbit TF0  = 0x8D;
sbit TR0  = 0x8C;
sbit IE1  = 0x8B;
sbit IT1  = 0x8A;
sbit IE0  = 0x89;
sbit IT0  = 0x88;

/*  IE   */
sbit EA   = 0xAF;
sbit ET2  = 0xAD;
sbit ES   = 0xAC;
sbit ET1  = 0xAB;
sbit EX1  = 0xAA;
sbit ET0  = 0xA9;
sbit EX0  = 0xA8;

/*  IP   */ 
sbit PT2  = 0xBD;
sbit PS   = 0xBC;
sbit PT1  = 0xBB;
sbit PX1  = 0xBA;
sbit PT0  = 0xB9;
sbit PX0  = 0xB8;

/*  P3  */
sbit RD   = 0xB7;
sbit WR   = 0xB6;
sbit T1   = 0xB5;
sbit T0   = 0xB4;
sbit INT1 = 0xB3;
sbit INT0 = 0xB2;
sbit TXD  = 0xB1;
sbit RXD  = 0xB0;

/*  SCON  */
sbit SM0  = 0x9E;
sbit SM1  = 0x9F;
sbit SM2  = 0x9D;
sbit REN  = 0x9C;
sbit TB8  = 0x9B;
sbit RB8  = 0x9A;
sbit TI   = 0x99;
sbit RI   = 0x98;

//WDT
sfr WDTCON  = 0xBD;	// WDT 分频控制
sfr WDTCLR0	= 0x84;	// WDT清0
sfr WDTCLR1	= 0xEB;	// WDT清0

//MOVXRH
sfr MOVXRH = 0xA1;
sfr	AUXR1 = 0xA2;		// 双DPTR切换,可以用INC AUXR1实现

//IOPort寄存器
sfr	P0MDL	= 0xEC;	//P0.0~P0.3 IO模式控制，00:施密特输入，无上拉，01：输入，有上拉，10：推挽输出，11：开漏输出
sfr	P0MDH	= 0xFC;	//P0.4~P0.7
			
sfr	P1MDL	= 0xED;	//P1.0~P1.3
sfr	P1MDH	= 0xFD;	//P1.4~P1.7

sfr	P2MDL	= 0xEE;	//P2.0~P2.3

sfr	P3MDL	= 0xEF;	//P3.0~P3.3

//中断扩展寄存器
sfr	EIE1	= 0xA9;	//中断允许控制
sfr	EIP1	= 0xB9;	//中断优先级控制
sfr	EIE2	= 0xAA;	//中断允许控制
sfr	EIP2	= 0xBA;	//中断优先级控制
sfr EINTCS0	= 0xAC;	// 外部中断控制与状态寄存器

//LVR
sfr	LVRCON	= 0xA6;		// 低压复位电压设置，Enable

//复位
sfr RSTSRC = 0xA7;      // 复位标志

//系统时钟
sfr WPKEY   = 0x95;
sfr MCKSET  = 0x91;

//----------------------------------------------------------------------------------------------------------------------------
#if	ASM ==1
xSYSCFG			EQU	0FFA2H		// 时钟配置
xTESTCON		EQU	0FFAFH		// 测试控制寄存器，测试脚,LVR_TE,SysRst_TE,CKOS[1:0],CKTE
xTIMPRS			EQU	0FFCAH		// Timer预分频控制寄存器
xSTOPCFG		EQU 0FFA0H
#else
#define xSYSCFG		(*(volatile unsigned char xdata *)0xFFA2)		//时钟配置
#define xTESTCON	(*(volatile unsigned char xdata *)0xFFAF)		//测试控制寄存器，测试脚,LVR_TE,SysRst_TE,CKOS[1:0],CKTE
#define xTIMPRS		(*(volatile unsigned char xdata *)0xFFCA)		// Timer预分频控制寄存器
#define xSTOPCFG    (*(volatile unsigned char xdata *)0xFFA0)
#endif


//----------------------------------------------------------------------------------------------------------------------------
#if	ASM == 1
xKBICON			EQU	0FFAAH	//设置对应的P1[7:0]为输入
xKBIWKP         EQU	0FFABH  	//KBI唤醒极性设置寄存器
#else
#define	xKBICON		(*(volatile unsigned char xdata *)0xFFAA)		//设置对应的P1[7:0]为输入
#define	xKBIWKP		(*(volatile unsigned char xdata *)0xFFAB)		//KBI唤醒极性设置寄存器
#endif

sfr KBIFLG = 0x9E;

//载波Timer接口寄存器
sfr	CRGCON1			= 0xE1;			//载波定时器控制，CRTEN,CRTMODE,CRTEXEN,CRTOVEN,REMSYN,REMOPS,CRTEXF,CRTF
sfr	CRGCON2			= 0xE2;			//载波定时器分频系数：CRTCKD3,CRTCKD2,CRTCKD1,CRTCKD0
sfr	CRGCNT			= 0xE3;			//载波定时器计数，CRTCNT
sfr	CRGDATA0		= 0xE4;			//载波定时器初值，CRTDATA0
sfr	CRGDATA1		= 0xE5;			//载波定时器初值，CRTDATA1

//动态键盘
#if	ASM == 1
xDKWCON		EQU	0FFDAH	
xDKWSELP0       EQU	0FFD0H 
xDKWSELP1       EQU	0FFD1H
xDKWSELP2       EQU	0FFD2H 
#else
#define	xDKWCON		(*(volatile unsigned char xdata *)0xFFDA)		
#define	xDKWSELP0	(*(volatile unsigned char xdata *)0xFFD0)
#define	xDKWSELP1	(*(volatile unsigned char xdata *)0xFFD1)
#define	xDKWSELP2	(*(volatile unsigned char xdata *)0xFFD2)
#endif

//----------------------------------------------------------------------------------------------------------------------------
#if	ASM == 1
xIOMUX0		EQU			0FFE0H			// I2C,--,CPOE,BUZEN,NBUZEN,WTIRQOE,PWMEN,--
#else
#define	xIOMUX0		(*(volatile unsigned char xdata *)0xFFE0)		// I2C,--,CPOE,BUZEN,NBUZEN,WTIRQOE,PWMEN,--
#endif

#endif