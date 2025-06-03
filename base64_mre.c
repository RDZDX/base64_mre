#include "base64_mre.h"

VMBOOL trigeris = VM_FALSE;
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
            if (vm_selector_run(0, 0, job) == 0) {trigeris = VM_TRUE;}
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
        bin2base64(bufferBin, nread, buffer64);
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

VMINT job1(VMWCHAR *FILE_PATH, VMINT wlen) {

    VMFILE f_read;
    VMFILE f_write;
    VMUINT nread;
    VMWCHAR autoFullPathName[100];
    VMWCHAR wfile_extension[8];

    char readBuffer[READ_SIZE + 1]; // +1 - null terminatorui
    uint8_t decodedBuffer[(READ_SIZE * 3) / 4 + 4];  // max dekoduotas dydis
    size_t base64Len = 0;
    char base64Chunk[READ_SIZE + MAX_LINE_LENGTH]; // kaupiam base64 dalis tarp skaitymų
    size_t chunkLen = 0;
    size_t j;

    init_base64_table();

    vm_ascii_to_ucs2(wfile_extension, 8, "bin");
    vm_wstrncpy(autoFullPathName, FILE_PATH, vm_wstrlen(FILE_PATH) - 3); 
    vm_wstrcat(autoFullPathName, wfile_extension);

    f_read = vm_file_open(FILE_PATH, MODE_READ, TRUE);
    if(f_read < 0){return -1;}
    f_write = vm_file_open(autoFullPathName, MODE_CREATE_ALWAYS_WRITE, TRUE);

    while (!vm_file_is_eof(f_read)) {
        vm_file_read(f_read, readBuffer, READ_SIZE, &nread);
        readBuffer[nread] = '\0';

        // Kopijuojame naujus duomenis į base64Chunk | Input too large for buffer
        if (chunkLen + nread > sizeof(base64Chunk) - 1) {
            vm_file_close(f_read);
            vm_file_close(f_write);
            return -1;
        }
        memcpy(base64Chunk + chunkLen, readBuffer, nread);
        chunkLen += nread;
        base64Chunk[chunkLen] = '\0';

        // Apdorojame base64Chunk dalimis po 4 simbolius, ignoruodami whitespace
        size_t i = 0, outIndex = 0;
        int val = 0, valb = -8;
        for (i = 0; i < chunkLen; i++) {
            unsigned char c = base64Chunk[i];
            if (isspace(c))
                continue;
            if (base64_table[c] == 255)
                continue; // praleidžiame netinkamus simbolius

            val = (val << 6) + base64_table[c];
            valb += 6;
            if (valb >= 0) {
                decodedBuffer[outIndex++] = (val >> valb) & 0xFF;
                valb -= 8;
            }

            // Kai decodedBuffer užsipildo, išrašome į failą ir tęsime
            if (outIndex >= sizeof(decodedBuffer) - 1) {
                vm_file_write(f_write, decodedBuffer, outIndex, &nread);

                outIndex = 0;
            }
        }

        // Nukopijuojame likusius base64 simbolius į pradžią (tuos, kurie nepilnai sudavė 3 baitus)
        size_t leftover = 0;
        if (valb >= -6) {
            // Išsaugome paskutinius base64 simbolius, nes jie gali būti neišbaigti
            for (j = chunkLen - 1; j >= i; j--) {
                if (!isspace(base64Chunk[j]))
                    leftover = chunkLen - j;
                else
                    break;
            }
        }
        if (leftover > 0) {
            memmove(base64Chunk, base64Chunk + chunkLen - leftover, leftover);
        }
        chunkLen = leftover;

        // Išrašome likusį dekoduotą duomenų kiekį
        if (outIndex > 0) {
            vm_file_write(f_write, decodedBuffer, outIndex, &nread);


        }
    }

    vm_file_close(f_read);
    vm_file_close(f_write);

    return 0;
}

void init_base64_table() {

    int i;

    for (i = 0; i < 256; i++) {
        base64_table[i] = 255; // Invalid
    }
    for (i = 0; i < 64; i++) {
        base64_table[(unsigned char)base64_chars[i]] = i;
    }
    base64_table['='] = 0;
}

void bin2base64(const uint8_t *input, VMUINT length, uint8_t *output) {

    size_t i, o = 0;
    uint32_t val;
    int pad;

    for (i = 0; i < length; i += 3) {
        val = 0;
        pad = 0;

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

        output[o++] = base64_chars[(val >> 18) & 0x3F];
        output[o++] = base64_chars[(val >> 12) & 0x3F];
        output[o++] = (pad > 1) ? '=' : base64_chars[(val >> 6) & 0x3F];
        output[o++] = (pad > 0) ? '=' : base64_chars[val & 0x3F];
    }
    output[o] = '\0';
}

size_t base64_to_bin(const char *input, uint8_t *output) {

    size_t len = 0;
    int val = 0, valb = -8;
    while (*input) {
        unsigned char c = *input++;
        if (isspace(c))
            continue;

        if (base64_table[c] == 255)
            continue; // Skip invalid

        val = (val << 6) + base64_table[c];
        valb += 6;
        if (valb >= 0) {
            output[len++] = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    return len;
}

