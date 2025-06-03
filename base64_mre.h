#ifndef _VRE_APP_WIZARDTEMPLATE_
#define	_VRE_APP_WIZARDTEMPLATE_

#include "vmio.h"
#include "string.h"
#include "stdint.h"

#define CHUNK_SIZE 768
#define MAX_LINE_LENGTH 76
#define READ_SIZE 4096

void handle_sysevt(VMINT message, VMINT param);
VMINT job(VMWCHAR *FILE_PATH, VMINT wlen);
VMINT job1(VMWCHAR *FILE_PATH, VMINT wlen);
void init_base64_table();
void bin2base64(const uint8_t *input, VMUINT length, uint8_t *output);
size_t base64_to_bin(const char *input, uint8_t *output);

#endif

