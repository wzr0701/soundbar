// font.h 
//

#if !defined(__FONT_H__)
#define __FONT_H__

#include "typedef.h"

#define PIXELS_PER_BYTE					8
#define	FONT_INDEX_TAB_SIZE				4    //�����ַ���Ӧ�����������Ϣ����Ϊ 4Byte  (b0~b25: ��¼������Ϣ����ʼ��ַ, b26~b31: ��¼��ǰ�ַ������ؿ��)
#define	GET_FONT_WIDTH(charinfo)		(charinfo >> 26)
#define	GET_FONT_OFFADDR(charinfo)		(charinfo & 0x3ffffff)
#define	DB2UC(c1,c2)					(c1 | (((U16)c2)<<8))

typedef struct tagFlSectionInfo{
            WORD  First;        
            WORD  Last;         
            DWORD OffAddr;     
       } FL_SECTION_INF, *PFL_SECTION_INF;

typedef struct tagFontLibHeader{
            BYTE    magic[4];    //'U'(or ��M��), 'F', 'L', X---Unicode(or MBCS) Font Library, X: ��ʾ�汾��. �ָߵ�4λ���� 0x12��ʾ Ver 1.2
            DWORD 	Size;            
            BYTE    nSection; // ���ּ������ݣ���Ҫ��� UNICODE ������Ч��
            BYTE    YSize;                 
            WORD    wCpFlag;    // codepageflag:  bit0~bit13 ÿ��bit�ֱ����һ��CodePage ��־�������1�����ʾ��ǰCodePage ��ѡ��������Ϊ��ѡ����
            char    reserved[4];      // Ԥ���ֽ�    
            FL_SECTION_INF  *pSection;
        } FL_HEADER, *PFL_HEADER;


int  font_init(char *pFontFile);
void font_exit();

BYTE font_get_ysize();
int  font_read_char_distX(WORD wCode);
int  font_read_char_dot_array(WORD wCode, BYTE *fontArray, WORD *bytesPerLine);

#endif // __FONT_H__
