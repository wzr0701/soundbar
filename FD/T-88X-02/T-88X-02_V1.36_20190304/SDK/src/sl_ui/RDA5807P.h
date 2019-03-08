#ifndef _RDA5807P_H_
#define _RDA5807P_H_

#define    u8        unsigned char 
#define    u16        unsigned int

#define    UINT8        unsigned char 
#define    UINT16        unsigned int

void CS1000_Write(u8 size);
void CS1000_Read(u8 size);


extern void rda5807p_init(void);
extern void rda5807p_set_freq(u16 freq);
extern void rda5807p_off(void);
extern void rda5807p_set_vol(u8 vol);
extern u8 rda5807p_seek(u16 freq);
extern void rda5807p_mute(void);
extern void rda5807p_unmute(void);
extern u8 rda5807p_online(void);
#endif
