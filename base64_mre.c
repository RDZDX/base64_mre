#include "base64_mre.h"

VMBOOL trigeris = VM_FALSE;
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64url_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static uint8_t base64_table[256];

void vm_main(void) {
	vm_reg_sysevt_callback(handle_sysevt);
}

void handle_sysevt(VMINT message, VMINT param) {

    switch (message) {
        case VM_MSG_CREATE:
        case VM_MSG_ACTIVE:
            break;

        case VM_MSG_PAINT:
            if (trigeris == VM_TRUE) {vm_exit_app();}
            if (vm_selector_run(0, 0, job) == 0) {trigeris = VM_TRUE;}	 	//Bin2B64
            //if (vm_selector_run(0, 0, jobx) == 0) {trigeris = VM_TRUE;}  	//Bin2B64U
            //if (vm_selector_run(0, 0, job1) == 0) {trigeris = VM_TRUE;}  	//B642bin
            break;

        case VM_MSG_INACTIVE:
            break;

        case VM_MSG_QUIT:
            break;
    }
}

VMINT job(VMWCHAR *FILE_PATH, VMINT wlen) {

    VMFILE f_read;
    VMFILE f_write;
    VMUINT nread;
    VMWCHAR autoFullPathName[100];
    VMWCHAR wfile_extension[8];

    uint8_t bufferBin[CHUNK_SIZE];
    uint8_t buffer64[(CHUNK_SIZE / 3) * 4 + 4];
    char lineBuffer[MAX_LINE_LENGTH];

    int lineLen = 0;
    size_t len;
    size_t i;

    init_base64_table();

    vm_ascii_to_ucs2(wfile_extension, 8, "b64");
    vm_wstrncpy(autoFullPathName, FILE_PATH, vm_wstrlen(FILE_PATH) - 3); 
    vm_wstrcat(autoFullPathName, wfile_extension);

    f_read = vm_file_open(FILE_PATH, MODE_READ, TRUE);
    if(f_read < 0){return -1;}
    f_write = vm_file_open(autoFullPathName, MODE_CREATE_ALWAYS_WRITE, TRUE);

    while (!vm_file_is_eof(f_read)) {
        vm_file_read(f_read, bufferBin, CHUNK_SIZE, &nread);
        //bin2base64(bufferBin, nread, buffer64);
        //bin2base64(bufferBin, nread, buffer64, charset);
        bin2base64(bufferBin, nread, buffer64, base64_chars); // base64_chars base64url_chars
        size_t len = strlen((char *)buffer64);

        for (i = 0; i < len; i++) {
            lineBuffer[lineLen++] = buffer64[i];

            if (lineLen == MAX_LINE_LENGTH) {
                lineBuffer[lineLen++] = '\n';           // įterpiam newline
                vm_file_write(f_write, lineBuffer, lineLen, &nread);
                lineLen = 0;
            }
        }
    }

    // Rašom paskutinę (nepilną) eilutę + newline
    if (lineLen > 0) {
        lineBuffer[lineLen++] = '\n';
        vm_file_write(f_write, lineBuffer, lineLen, &nread);
    }

    vm_file_close(f_read);
    vm_file_close(f_write);

    return 0;
}

VMINT jobx(VMWCHAR *FILE_PATH, VMINT wlen) {

    VMFILE f_read;
    VMFILE f_write;
    VMUINT nread;
    VMWCHAR autoFullPathName[100];
    VMWCHAR wfile_extension[8];

    uint8_t bufferBin[CHUNK_SIZE];
    uint8_t buffer64[(CHUNK_SIZE / 3) * 4 + 4];
    char lineBuffer[MAX_LINE_LENGTH];

    int lineLen = 0;
    size_t len;
    size_t i;

    init_base64_table();

    vm_ascii_to_ucs2(wfile_extension, 8, "b64");
    vm_wstrncpy(autoFullPathName, FILE_PATH, vm_wstrlen(FILE_PATH) - 3); 
    vm_wstrcat(autoFullPathName, wfile_extension);

    f_read = vm_file_open(FILE_PATH, MODE_READ, TRUE);
    if(f_read < 0){return -1;}
    f_write = vm_file_open(autoFullPathName, MODE_CREATE_ALWAYS_WRITE, TRUE);

    while (!vm_file_is_eof(f_read)) {
        vm_file_read(f_read, bufferBin, CHUNK_SIZE, &nread);
        bin2base64(bufferBin, nread, buffer64, base64url_chars); // base64_chars base64url_chars
        size_t len = strlen((char *)buffer64);

        for (i = 0; i < len; i++) {
            lineBuffer[lineLen++] = buffer64[i];

            if (lineLen == MAX_LINE_LENGTH) {
                lineBuffer[lineLen++] = '\n';           // įterpiam newline
                vm_file_write(f_write, lineBuffer, lineLen, &nread);
                lineLen = 0;
            }
        }
    }

    // Rašom paskutinę (nepilną) eilutę + newline
    if (lineLen > 0) {
        lineBuffer[lineLen++] = '\n';
        vm_file_write(f_write, lineBuffer, lineLen, &nread);
    }

    vm_file_close(f_read);
    vm_file_close(f_write);

    return 0;
}

void init_base64_table() {

    int i;

    for (i = 0; i < 256; i++)
        base64_table[i] = 255; // reikšmė 255 reiškia „invalid“

    for (i = 0; i < 64; i++) {
        base64_table[(unsigned char)base64_chars[i]] = i;
    }

    base64_table[(unsigned char)'-'] = 62;
    base64_table[(unsigned char)'_'] = 63;
    base64_table[(unsigned char)'='] = 0;
}

void bin2base64(const uint8_t *input, VMUINT length, uint8_t *output, const char *charset) {

    size_t i, o = 0;

    for (i = 0; i < length; i += 3) {
        uint32_t val = 0;
        int pad = 0;

        // Surenkame iki 3 baitų į 24 bitų „val“
        val |= input[i] << 16;
        if (i + 1 < length) {
            val |= input[i + 1] << 8;
        } else {
            pad++;
        }

        if (i + 2 < length) {
            val |= input[i + 2];
        } else {
            pad++;
        }

        // Iš 24-bitų „val“ ištraukiame po 6 bitus
        output[o++] = charset[(val >> 18) & 0x3F];
        output[o++] = charset[(val >> 12) & 0x3F];
        output[o++] = (pad > 1) ? '=' : charset[(val >> 6) & 0x3F];
        output[o++] = (pad > 0) ? '=' : charset[val & 0x3F];
    }

    // Uždarome eilutę
    output[o] = '\0';
}

static void hex_dump(const uint8_t *data, size_t len) {

    size_t i;
    char text[128];
    size_t offset = 0;

    if (len > 16) len = 16;
    for (i = 0; i < len; i++) {
        offset += sprintf(&text[offset], "%02X ", data[i]);
    }
    text[offset] = '\0';
    //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
}

void base64_decoder_init(base64_decoder_state_t *state) {

    state->val = 0;
    state->valb = -8;
}

size_t base64_decode_stream(base64_decoder_state_t *state, const char *input, size_t input_len, uint8_t *output) {

    size_t i;
    size_t len = 0;
    for (i = 0; i < input_len; i++) {
        unsigned char c = input[i];
        if (isspace(c)) continue;

        uint8_t d = base64_table[c];
        if (d == 255) continue;  // invalid simbolis, praleidžiame

        state->val = (state->val << 6) | d; //mažiau efektyvus analogas: state->val = state->val * 64 + d;
        state->valb += 6;

        if (state->valb >= 0) {
            output[len++] = (state->val >> state->valb) & 0xFF;
            state->valb -= 8;
        }
    }
    return len;
}

//VMINT job_base64_decode_debug(VMWCHAR *file_path) {
VMINT job1(VMWCHAR *file_path, VMINT wlen) {

    VMFILE f_read, f_write;
    VMUINT nread;
    VMWCHAR out_path[100];
    VMWCHAR wfile_extension[8];
    char inBuf[READ_SIZE + 1];
    uint8_t decodedBuf[(READ_SIZE * 3) / 4 + 4];
    char text[256];
    VMUINT i;

    base64_decoder_state_t state;
    base64_decoder_init(&state);

    init_base64_table();

    vm_ascii_to_ucs2(wfile_extension, 8, "bin");
    vm_wstrncpy(out_path, file_path, vm_wstrlen(file_path) - 3);
    vm_wstrcat(out_path, wfile_extension);

    f_read = vm_file_open(file_path, MODE_READ, TRUE);
    if (f_read < 0) {
        return -1;
    }

    f_write = vm_file_open(out_path, MODE_CREATE_ALWAYS_WRITE, TRUE);
    if (f_write < 0) {
        vm_file_close(f_read);
        return -2;
    }

    while (1) {
        VMINT ret = vm_file_read(f_read, inBuf, READ_SIZE, &nread);
        if (ret < 0) {
            break;
        }
        if (nread == 0) {
            break;
        }

        size_t outLen = base64_decode_stream(&state, inBuf, nread, decodedBuf);
        if (outLen > 0) {
            hex_dump(decodedBuf, outLen);
            vm_file_write(f_write, decodedBuf, outLen, &nread);
        }
    }

    vm_file_close(f_write);
    vm_file_close(f_read);

    return 0;
}
