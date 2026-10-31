
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h> // Required for FILE type used in the decode structure
#include "types.h" // Contains user-defined types such as Status and uint

/* Buffer size used to read one hidden byte from the image data */
#define DECODE_IMAGE_BUF_SIZE 8

/* Maximum secret buffer size for temporary payload handling */
#define MAX_SECRET_BUF_SIZE 1

typedef struct _DecodeInfo
{
    /* Stego image information */
    char *stego_image_fname; // Path of the encoded stego image
    FILE *fptr_stego_image;  // File pointer to the stego image

    /* Output secret file information */
    char output_fname[256];   // Output file name for decoded secret data
    int output_name_provided; // Flag to indicate whether output filename was provided
    FILE *fptr_output;        // File pointer to the output secret file

    /* Secret file information */
    char extn_secret_file[5]; // Secret file extension buffer, including room for '\0'
    int size_secret_file;     // Size of the secret file in bytes
    unsigned int extn_size;   // Length of the secret file extension

    /* Image data buffer */
    unsigned char image_data[DECODE_IMAGE_BUF_SIZE]; // Temporary buffer containing bits for one hidden byte

} DecodeInfo;

/* Function prototypes for the decoding workflow */

/* Validate command-line arguments for decoding */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open the stego image and output file for decoding */
Status open_decode_files(DecodeInfo *decInfo);

/* Main decoding routine that extracts the hidden data */
Status do_decoding(DecodeInfo *decInfo);

/* Extract one byte from the least significant bits of the image data */
unsigned char decode_byte_from_lsb(unsigned char image_buffer[]);

/* Skip the BMP header before reading the hidden payload */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Recover a size value encoded into the image bits */
uint decode_size_from_lsb(unsigned char image_buffer[]);

/* Read the length of the secret file extension from the image */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Read and store the secret file extension from the image */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Read the secret file size stored in the image */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Create or open the output file for the decoded secret */
Status open_output_file(DecodeInfo *decInfo);

/* Extract the hidden secret file data and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Close all files used during decoding */
void close_decode_files(DecodeInfo *decInfo);
#endif
