/***********************************************************
������	��c����д��һ����δ�unicode�����ʽ�ĵ����ֿ��ж�ȡ�ַ���Ϣ�����ؿ� +������Ϣ��
        �����ݴ��Ժ�Ч�ʷ��滹��ʹ�������и��ơ�лл���Ĳ��ģ�
���ߣ�  wujianguo
���ڣ� 	20090516
MSN:    wujianguo19@hotmail.com
qq��    9599598
*************************************************************/

#include "font.h"
#include "font_file.h"
#include "unicode.h"    

#include <stdio.h>
#include <stdlib.h>

extern FL_HEADER _fl_header;      

int font_read_sections()
{
	PFL_HEADER pfl_header = &_fl_header;

	font_release_sections();
	
	pfl_header->pSection = (FL_SECTION_INF *)malloc(pfl_header->nSection*sizeof(FL_SECTION_INF));
	if(pfl_header->pSection == NULL)
	{
		printf("Malloc fail!\n");
		return 0;
	}
	
	font_file_read(pfl_header->pSection, pfl_header->nSection*sizeof(FL_SECTION_INF));	
	return 1;
}

void font_release_sections()
{
	if(_fl_header.pSection != NULL)
	{
		free(_fl_header.pSection);
		_fl_header.pSection = NULL;
	}
}

DWORD read_char_info_unicode(WORD wCode)
{	
	DWORD offset ;
	int   i;
	DWORD dwCharInfo = 0;
	
	for(i = 0;i<_fl_header.nSection;i++)
	{
		if(wCode >= _fl_header.pSection[i].First && wCode <= _fl_header.pSection[i].Last)
			break;		
	}
	if(i >= _fl_header.nSection)	
		return 0;	
	
	offset = _fl_header.pSection[i].OffAddr+ FONT_INDEX_TAB_SIZE*(wCode - _fl_header.pSection[i].First );	
	font_file_seek(offset);
	font_file_read(&dwCharInfo, sizeof(DWORD));	

	return dwCharInfo;
}





