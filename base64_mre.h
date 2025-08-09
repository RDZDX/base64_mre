#ifndef _VRE_APP_WIZARDTEMPLATE_
#define	_VRE_APP_WIZARDTEMPLATE_

#include "vmio.h"
#include "string.h"
#include "stdint.h"
#include "ctype.h"
#include "vmsys.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include <stdio.h>
#include <stdlib.h>

#define CHUNK_SIZE 768
#define MAX_LINE_LENGTH 76
#define READ_SIZE 4096
#define B64_BLOCK 4
#define BASE64_INVALID 255

typedef struct {
    int val;
    int valb;
} base64_decoder_state_t;

void handle_sysevt(VMINT message, VMINT param);
VMINT job(VMWCHAR *FILE_PATH, VMINT wlen);
VMINT jobx(VMWCHAR *FILE_PATH, VMINT wlen);

void init_base64_table();
void bin2base64(const uint8_t *input, VMUINT length, uint8_t *output, const char *charset);
static void hex_dump(const uint8_t *data, size_t len);
size_t base64_decode_stream(base64_decoder_state_t *state, const char *input, size_t input_len, uint8_t *output);
void base64_decoder_init(base64_decoder_state_t *state);
VMINT job1(VMWCHAR *FILE_PATH, VMINT wlen);

#endif

