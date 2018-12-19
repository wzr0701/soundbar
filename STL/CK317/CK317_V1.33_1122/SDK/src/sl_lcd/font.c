/***********************************************************
������	��c����д��һ����δӵ����ֿ��ж�ȡ�ַ���Ϣ�����ؿ� +������Ϣ��
        �����ݴ��Ժ�Ч�ʷ��滹��ʹ�������и���,лл���Ĳ��ģ�

�ļ���ʽ�� 
		Unicode -- �ļ�ͷ+���ݶ�(section)+������+������Ϣ
		MBCS_Ladin-�ļ�ͷ+������+������Ϣ
		MBCS_CJK-- �ļ�ͷ+������Ϣ
		
���ߣ�  wujianguo
���ڣ� 	20090516
MSN:    wujianguo19@hotmail.com
qq��    9599598
*************************************************************/

#include "font.h"
#include "font_file.h"
#include "unicode.h"
#include "mbcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FL_HEADER _fl_header;
static DWORD g_dwCharInfo = 0;    // �洢��ǰ�ַ��ļ�����Ϣ��  bit0��bit25����ŵ�����Ϣ����ʼ��ַ�� bit26��bit31��������ؿ�ȡ�

static int ReadFontHeader(PFL_HEADER pfl_header)
{	
	font_file_read(pfl_header, sizeof(FL_HEADER) -4);

	//����ʾͷ
	if((_fl_header.magic[0] != 'U' && _fl_header.magic[0] != 'M')
		|| _fl_header.magic[1] != 'F' || _fl_header.magic[2] != 'L')
	{
		printf("Cann't support file format!\n");
		return 0;
	}

	if('U' == pfl_header->magic[0])     //unicode ����
	{
		return font_read_sections();
	}

	return 1;	
}

static int OpenFontFile(char *pFontFile)
{
	if(!font_file_open(pFontFile))
	{
		printf("Cann't open : %s\n", pFontFile);
		return 0;
	}
		
	return 1;
}

/***************************************************************
���ܣ� ��ʼ�����塣 ���������ļ����Ҷ�ȡ��Ϣͷ��
������ pFontFile--�ֿ��ļ���      
***************************************************************/
int font_init(char *pFontFile)
{
	memset(&_fl_header, 0, sizeof(FL_HEADER));

	if(OpenFontFile(pFontFile))
		return ReadFontHeader(&_fl_header);	
	else
		return 0;
}

void font_exit()
{
	if('U' == _fl_header.magic[0])     //unicode ����
		font_release_sections();

	font_file_close();
}

BYTE font_get_ysize()
{
	return _fl_header.YSize;
}


/********************************************************************
���ܣ� ��ȡ��ǰ�ַ������ؿ��, �ҽ�������Ϣ����һ��ȫ�ֱ�����g_dwCharInfo��
        ����������Ϣ����ͬʱ�ܻ�ȡ��ǰ�ַ��ĵ�����Ϣ����ʼ��ַ��
������ wCode -- ���ֿ�Ϊunicode�����ʽʱ����wCode��unicode���봦��
                �������루MBCS)����
********************************************************************/
int font_read_char_distX(WORD wCode)
{
	if('U' == _fl_header.magic[0])     // unicode ����
		g_dwCharInfo = read_char_info_unicode(wCode);
	else  // MBCS
		g_dwCharInfo = read_char_info_mbcs(wCode);

	return GET_FONT_WIDTH(g_dwCharInfo);
}

/**********************************************************************
���ܣ� ��ȡ������Ϣ
������ wCode ������Ԥ������Ҫ����Ϊǰ���б���һ��ȫ�ֵ�g_dwCharInfo��Ҳ��֪���˸��ַ�����Ӧ��Ϣ(���+������Ϣ����ʼ��ַ)��
       fontArray ��ŵ�����Ϣ
	   bytesPerLine ÿһ��ռ���ٸ��ֽڡ�
**********************************************************************/
int font_read_char_dot_array(WORD wCode, BYTE *fontArray, WORD *bytesPerLine)
{	
	*bytesPerLine= (WORD)((GET_FONT_WIDTH(g_dwCharInfo))+7)/PIXELS_PER_BYTE;	

	if(g_dwCharInfo > 0)
	{		
		DWORD nDataLen = *bytesPerLine * _fl_header.YSize;			
		DWORD  dwOffset = GET_FONT_OFFADDR(g_dwCharInfo);    //��ȡ�ַ�����ĵ�ַ��Ϣ(��26λ)
		
		font_file_seek(dwOffset);
		font_file_read(fontArray, nDataLen);	
		
		return 1;
	}
	
	return 0;
}
