/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "cryptoauthlib.h"       

//These defines are used one at a time to provision specific keys
//#define PROVISION_IO_PROT_KEY
//#define PROVISION_AES_KEY

#define SLOT_5 5
#define SLOT_6 6

//Terminal coloring
#define ESC_CLEAR_TERMINAL          "\033[2J"
#define ESC_CURSOR_X1Y1             "\033[1;1H"
#define ESC_HIDE_CURSOR             "\033[?25l"
#define ESC_CLEAR_LINE              "\033[2K"
#define ESC_RESETCOLOR              "\033[0m"

#define ESC_BLACK                   "\033[0;30m"
#define ESC_RED                     "\033[0;31m"
#define ESC_GREEN                   "\033[0;32m"
#define ESC_YELLOW                  "\033[1;33m"
#define ESC_BLUE                    "\033[1;34m"
#define ESC_MAGENTA                 "\033[1;35m"
#define ESC_CYAN                    "\033[0;36m"
#define ESC_WHITE                   "\033[1;37m"

#define PRINT(...)                  printf(__VA_ARGS__)

#define CHECK_STATUS(s)                                                 				\
if(s != ATCA_SUCCESS)                                                   				\
{                                                                       			    \
	printf("Error: Line %d in %s\r\n", __LINE__, __FILE__);                             \
    	printf("STATUS = %X\r\n", s);                                       			\
    	printf("Code explanations can be found in atca_status.h \r\n\n");               \
    	while(1);                                                           			\
}

extern ATCAIfaceCfg atecc608_0_init_data; // Sets up configuration of interface
ATCA_STATUS status; // variable "status" is an enumerated list of statuses


const uint8_t IO_PROT_KEY[32] = {
    0x4D, 0x8F, 0x5C, 0x21, 0x3C, 0x15, 0x02, 0x0F, 0x24, 0x13, 0x89, 0x0F, 0x5A, 0x2A, 0xAA, 0x8A,
    0x70, 0x36, 0x61, 0xB9, 0x18, 0xAD, 0xB5, 0x1B, 0x7C, 0x8B, 0xD3, 0x70, 0xCE, 0xF2, 0xD2, 0x95
};

const uint8_t AES_KEY[16] = {
    0x11, 0x87, 0x4A, 0x43, 0xF9, 0xE2, 0x62, 0xD7, 0xB0, 0x3E, 0xB7, 0x7D, 0x7A, 0x65, 0x73, 0xF8
};

const uint8_t HOST_NONCE[20] = {
    0xB6, 0x7B, 0x97, 0x34, 0xD7, 0x79, 0x77, 0x7F, 0x35, 0x11, 0x9F, 0xC8, 0x77, 0xA6, 0x5C, 0xE0, 0x58, 0x00, 0x27, 0x9A
};


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
void print_bytes(uint8_t * ptr, uint16_t length);
void print_bytes2(uint8_t * ptr, uint16_t length);
bool compareByteArrays(const uint8_t array1[4], const uint8_t array2[4]);
void xor_arrays(const uint8_t *a, const uint8_t *b, uint8_t *result, size_t length);

int main(void) {
    uint8_t data_buf[100];
    uint8_t hash_buffer[32];

    /* Initialize all modules */
    SYS_Initialize(NULL);

    PRINT("AES with KDF Proof Of Concept\r\n\r\n"); // beginning message 
    PRINT(ESC_BLUE "This demo uses ECC608 to store parent AES Key.\r\n");
    PRINT("Both the host MCU and ECC608 also know a shared IO Protection Key.\r\n");
    PRINT("Since encryption/decryption through the ECC608 results in data in the clear on the bus,\r\n");
    PRINT("it is more secure to perform the encryption/decryption in the host MCU.\r\n");
    PRINT("The key concept here is that the host MCU can seed the ECC608 KDF function to deterministically\r\n");
    PRINT("generate an AES key.  This key is returned encrypted back to the host MCU and the host MCU can than\r\n");
    PRINT("recover the AES key and begin to use it.  Any host MCU can recover the AES key as long as the ECC608\r\n");
    PRINT("contains the parent symmetric key and the host MCU & ECC608 shared a common IO protection key.\r\n\n" ESC_RESETCOLOR);

    // <editor-fold defaultstate="collapsed" desc="INIT DEVICE">
    status = atcab_init(&atecc608_0_init_data); // Passing info to address the device
    CHECK_STATUS(status);
    if (status == 0) {
        printf("Initialization Success!\r\n\r\n"); // Returns status.  Should be "0" for "success"
    } else {
        printf("Initialization Failed!\r\n");
        while (1);
    }
    // </editor-fold>

    //This provisioning steps are just simple self provisioning instead of using TPDS
    // <editor-fold defaultstate="collapsed" desc="PROVISION_IO_PROT_KEY">
#ifdef PROVISION_IO_PROT_KEY
    status = calib_write_zone(atcab_get_device(), ATCA_ZONE_DATA, SLOT_6, 0, 0, IO_PROT_KEY, 32);
    CHECK_STATUS(status);
    printf("Step 1 Complete. IO Protection Key Provisioning Complete!\r\nComment out line 32 #define PROVISION_IO_PROT_KEY to continue to next step and re-flash code.\r\n");
    while (1);
#endif
    // </editor-fold>

    //This provisioning steps are just simple self provisioning instead of using TPDS
    // <editor-fold defaultstate="collapsed" desc="PROVISION_AES_KEY">
#ifdef PROVISION_AES_KEY
    status = calib_write_enc(atcab_get_device(), SLOT_5, 0, AES_KEY, IO_PROT_KEY, SLOT_6, HOST_NONCE);
    CHECK_STATUS(status);
    printf("Step 2 Complete. AES Key Provisioning Complete!\r\nComment out line 33 #define PROVISION_AES_KEY to continue to next step and re-flash code.\r\n");
    while (1);
#endif
    // </editor-fold>

    // <editor-fold defaultstate="collapsed" desc="Perform KDF & RECOVER AES KEY">
    uint8_t mode = KDF_MODE_ALG_AES | KDF_MODE_SOURCE_SLOT | KDF_MODE_TARGET_OUTPUT_ENC;
    uint16_t key_id = 5; // Source key slot (slot 5)
    uint32_t details = 0; // Details parameter (can be customized as needed)
    uint8_t message[16] = {0}; // Input data for the KDF function (customize as needed)
    uint8_t out_data[32]; // Buffer to hold the KDF output
    uint8_t out_nonce[32]; // Buffer to hold the output nonce (if needed)

    for (uint8_t i = 0; i < 2; i++) {
        status = calib_kdf(atcab_get_device(), mode, key_id, details, message, out_data, out_nonce);
        CHECK_STATUS(status);
        PRINT(ESC_CYAN "KDF out (Encrypted Derived AES Key):\r\n" ESC_RESETCOLOR);
        print_bytes(out_data, 32);
        PRINT("\r\n");

        CHECK_STATUS(status);
        PRINT(ESC_CYAN"KDF Nonce out (Nonce used as part of encrypting key above):\r\n" ESC_RESETCOLOR);
        print_bytes(out_nonce, 32);
        PRINT("\r\n");

        //Copy kdf output to databuffer
        memcpy(&data_buf[0], IO_PROT_KEY, 32);
        //Copy first 16 byte nonce to databuffer
        memcpy(&data_buf[32], out_nonce, 16);

        status = (int) atcac_sw_sha2_256(data_buf, 48, hash_buffer); //SHA256 IO ProtKey with first 16 bytes of nonce
        CHECK_STATUS(status);

        //XOR Digest from preiovus step with 
        xor_arrays(out_data, hash_buffer, data_buf, 32);

        PRINT(ESC_GREEN "Derived AES Key: ");
        for (size_t i = 0; i < 16; i++) {
            PRINT("%02X ", data_buf[i]);
        }
        if(i==0)
        {
            PRINT(ESC_RESETCOLOR "\r\n\nPerforming a second iteration of this to show different data still results in same key.\r\n\n\n");
        }
    }
    // </editor-fold>

    while (1);
}

void print_bytes(uint8_t * ptr, uint16_t length) {
    uint16_t i = 0;
    uint8_t line_count = 0;
    for (i = 0; i < length; i++) {
        printf("%02x ", ptr[i]);
        line_count++;
        if (line_count == 16) {
            printf("\r\n");
            line_count = 0;
        }
    }
}

void print_bytes2(uint8_t * ptr, uint16_t length) {
    uint16_t i = 0;
    for (i = 0; i < length; i++) {
        printf("%02x", ptr[i]);
    }
    printf("\r\n");
}

bool compareByteArrays(const uint8_t array1[4], const uint8_t array2[4]) {
    for (int i = 0; i < 4; i++) {
        if (array1[i] != array2[i]) {
            return false; // Arrays are not equal
        }
    }
    return true; // Arrays are equal
}

void xor_arrays(const uint8_t *a, const uint8_t *b, uint8_t *result, size_t length) {
    for (size_t i = 0; i < length; i++) {
        result[i] = a[i] ^ b[i];
    }
}
/*******************************************************************************
 End of File
 */

