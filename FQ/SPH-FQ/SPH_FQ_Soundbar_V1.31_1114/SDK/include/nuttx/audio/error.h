#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <stddef.h>

#define AV_ERROR_MAX_STRING_SIZE   64
#if EDOM > 0
#define AVERROR(e)                (-(e)) 
#define AVUNERROR(e)              (-(e))
#else
#define AVERROR(e)                (e)
#define AVUNERROR(e)              (e)
#endif

#define FFERRTAG(a, b, c, d)       (-(int)MKTAG(a, b, c, d))

#define AVERROR_BSF_NOT_FOUND      FFERRTAG(0xF8,'B','S','F') 
#define AVERROR_BUG                FFERRTAG( 'B','U','G','!')
#define AVERROR_BUFFER_TOO_SMALL   FFERRTAG( 'B','U','F','S')
#define AVERROR_DECODER_NOT_FOUND  FFERRTAG(0xF8,'D','E','C')
#define AVERROR_DEMUXER_NOT_FOUND  FFERRTAG(0xF8,'D','E','M')
#define AVERROR_ENCODER_NOT_FOUND  FFERRTAG(0xF8,'E','N','C')
#define AVERROR_EOF                FFERRTAG( 'E','O','F',' ')
#define AVERROR_EXIT               FFERRTAG( 'E','X','I','T')
#define AVERROR_EXTERNAL           FFERRTAG( 'E','X','T',' ')
#define AVERROR_FILTER_NOT_FOUND   FFERRTAG(0xF8,'F','I','L')
#define AVERROR_INVALIDDATA        FFERRTAG( 'I','N','D','A')
#define AVERROR_MUXER_NOT_FOUND    FFERRTAG(0xF8,'M','U','X')
#define AVERROR_OPTION_NOT_FOUND   FFERRTAG(0xF8,'O','P','T')
#define AVERROR_PATCHWELCOME       FFERRTAG( 'P','A','W','E')
#define AVERROR_PROTOCOL_NOT_FOUND FFERRTAG(0xF8,'P','R','O')
#define AVERROR_STREAM_NOT_FOUND   FFERRTAG(0xF8,'S','T','R')
#define AVERROR_BUG2               FFERRTAG( 'B','U','G',' ')
#define AVERROR_UNKNOWN            FFERRTAG( 'U','N','K','N')
#define AVERROR_EXPERIMENTAL       (-0x2bb2afa8)
#define AVERROR_INPUT_CHANGED      (-0x636e6701)
#define AVERROR_OUTPUT_CHANGED     (-0x636e6702)
#define AVERROR_HTTP_BAD_REQUEST   FFERRTAG(0xF8,'4','0','0')
#define AVERROR_HTTP_UNAUTHORIZED  FFERRTAG(0xF8,'4','0','1')
#define AVERROR_HTTP_FORBIDDEN     FFERRTAG(0xF8,'4','0','3')
#define AVERROR_HTTP_NOT_FOUND     FFERRTAG(0xF8,'4','0','4')
#define AVERROR_HTTP_OTHER_4XX     FFERRTAG(0xF8,'4','X','X')
#define AVERROR_HTTP_SERVER_ERROR  FFERRTAG(0xF8,'5','X','X')
#define AVERROR_REDO               FFERRTAG('R','E','D','O')


#endif /* ERROR_H */
