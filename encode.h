#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>     // Required for FILE type used in the encode structure
#include "types.h"      // Contains user-defined data types used by the steganography encoder

/* Maximum payload buffer size for the secret file content */
#define MAX_SECRET_BUF_SIZE 1

/* Image buffer size: one byte per bit of the secret data, using 8 pixels per byte */
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)

/* Maximum supported file extension length including the terminating null character */
#define MAX_FILE_SUFFIX 4

typedef struct _EncodeInfo
{
    /* Source image details */
    char *src_image_fname;               // Input BMP image path
    FILE *fptr_src_image;                // File pointer for source image
    uint image_capacity;                 // Total image capacity available for embedding data
    uint bits_per_pixel;                 // Bits used per pixel in the image format
    char image_data[MAX_IMAGE_BUF_SIZE]; // Buffer storing pixel data to be modified

    /* Secret file details */
    char *secret_fname;                         // Secret file path to be embedded
    FILE *fptr_secret;                          // File pointer for the secret file
    char extn_secret_file[MAX_FILE_SUFFIX + 1]; // File extension like ".txt" with room for '\0'
    char secret_data[MAX_SECRET_BUF_SIZE];      // Temporary data buffer for secret file bytes
    long size_secret_file;                      // Size of the secret file in bytes

    /* Output stego image details */
    char *stego_image_fname; // Output image path containing the embedded data
    FILE *fptr_stego_image;  // File pointer for the generated stego image

} EncodeInfo;

/* -----------------------------------------------------------
 * Function prototypes for the encoding workflow.
 * Each function performs one logical step in the steganography process.
 * ----------------------------------------------------------- */

/* Determine the requested operation mode from the command line arguments */
OperationType check_operation_type(char *argv);

/* Validate command-line arguments for the encode operation */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Main encode routine that orchestrates the entire process */
Status do_encoding(EncodeInfo *encInfo);

/* Open input and output files required for encoding */
Status open_files(EncodeInfo *encInfo);

/* Check whether the source image can hold the secret data */
Status check_capacity(EncodeInfo *encInfo);

/* Compute the BMP image size in bytes */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Fetch the size of the provided file in bytes */
uint get_file_size(FILE *fptr);

/* Duplicate the BMP header into the destination image */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Embed the magic string header into the image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Embed the secret file extension into the image metadata area */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Embed the secret file size into the image */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Embed the contents of the secret file into the image */
Status encode_secret_file_data(EncodeInfo *encInfo);

/* General encoding routine that writes a data buffer into the image */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode a single byte into the least-significant bits of the image buffer */
Status encode_byte_to_lsb(char data, char *image_buffer);

/* Copy the remaining image bytes after the encoded payload has been written */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

/* Encode a size value into the least-significant bits of the image buffer */
Status encode_size_to_lsb(int size, char *imageBuffer);

/* Store the length of the secret file extension in the image header */
Status encode_secret_file_extn_size(int extn_size, EncodeInfo *encInfo);

/* Close all open files associated with the encoding process */
void close_files(EncodeInfo *encInfo);

#endif
