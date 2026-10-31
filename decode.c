#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

/* Extract one byte from the least significant bits of eight image bytes */
unsigned char decode_byte_from_lsb(unsigned char image_buffer[])
{
    unsigned char data = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        data = (data << 1) | (image_buffer[i] & 1);
    }

    return data;
}

/* Open the encoded stego image so the hidden payload can be read */
Status open_decode_files(DecodeInfo *decInfo)
{
    printf("INFO: Opening required files\n");

    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->stego_image_fname);

    return e_success;
}

/* Skip the 54-byte BMP header to reach the actual embedded data */
Status skip_bmp_header(FILE *fptr_stego_image)
{
    if (fseek(fptr_stego_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }

    return e_success;
}

/* Decode the magic string used to verify whether the image contains hidden data */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic_string[strlen(MAGIC_STRING) + 1]; // 1 byte reserved for '\0'
    unsigned char image_buffer[8];
    size_t i;

    /* Read the embedded magic string byte by byte from the image data */
    for (i = 0; i < strlen(MAGIC_STRING); i++)
    {
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        magic_string[i] = decode_byte_from_lsb(image_buffer);
    }

    magic_string[i] = '\0';

    /* Compare the recovered marker with the expected hidden-signature value */
    if (strcmp(magic_string, MAGIC_STRING) == 0)
    {
        printf("Magic string matched successfully\n");
        return e_success;
    }

    /* Stop decoding if the image does not contain the expected steganographic signature */
    printf("Error: Magic string does not match\n");
    return e_failure;
}

/* Perform the complete decoding flow: open file, validate signature, read metadata, and recover data */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: ## Decoding Procedure Started ##\n");

    if (open_decode_files(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }

    if (skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }

    printf("INFO: Decoding Magic String Signature\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Decoding Output File Extension\n");

    /* Retrieve the encoded secret-file extension metadata before reading the payload */
    if (decode_secret_file_extn_size(decInfo) == e_failure ||
        decode_secret_file_extn(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    /* If no output filename was provided, use the default decoded.txt file */
    if (decInfo->output_name_provided == 0)
    {
        printf("INFO: Output File not mentioned. Creating decoded.txt as default\n");
    }

    /* Create the output file before writing the recovered secret data */
    if (open_output_file(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }

    printf("INFO: Done. Opened all required files\n");

    printf("INFO: Decoding %s File Size\n", decInfo->output_fname);

    /* Read the embedded size value so the decoder knows how many bytes to recover */
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Decoding %s File Data\n", decInfo->output_fname);

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        close_decode_files(decInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    close_decode_files(decInfo);

    printf("INFO: ################### Decoding Done Successfully ###################\n");

    return e_success;
}

/* Reconstruct an integer value from 32 bits embedded in the image LSBs */
uint decode_size_from_lsb(unsigned char image_buffer[])
{
    unsigned int size = 0;
    int i;

    for (i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);
    }

    return size;
}

/* Decode the length of the secret file extension stored in the image */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];

    if (fread(image_buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decInfo->extn_size = decode_size_from_lsb(image_buffer);

    /* Extension length must be valid; a typical file extension is at most 4 characters */
    if (decInfo->extn_size == 0 || decInfo->extn_size > 4)
    {
        printf("Error: Invalid extension size\n");
        return e_failure;
    }

    printf("Secret extension length: %u\n", decInfo->extn_size);

    return e_success;
}

/* Decode the file extension characters embedded in the image payload */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    unsigned int i;

    for (i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf("Secret extension: %s\n", decInfo->extn_secret_file);

    return e_success;
}

/* Decode the original secret file size that was embedded in the stego image */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];

    if (fread(image_buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decInfo->size_secret_file = (int)decode_size_from_lsb(image_buffer);

    printf("Secret file size: %d bytes\n", decInfo->size_secret_file);

    return e_success;
}

/* Open the output file where the recovered secret data will be written */
Status open_output_file(DecodeInfo *decInfo)
{
    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");

    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->output_fname);

    return e_success;
}

/* Recover each hidden byte from the payload and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    unsigned char data;
    int i;

    for (i = 0; i < decInfo->size_secret_file; i++)
    {
        /* Read 8 image bytes */
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        /* Extract one secret byte */
        data = decode_byte_from_lsb(image_buffer);

        /* Write the recovered byte */
        if (fwrite(&data, 1, 1, decInfo->fptr_output) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}

/* Validate command-line arguments and prepare the decode configuration */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    size_t len;

    /* The stego image path is mandatory for decoding operations */
    if (argv[2] == NULL)
    {
        printf("Error: Missing stego image filename\n");
        return e_failure;
    }

    len = strlen(argv[2]);

    /* Ensure the selected file is a valid BMP image before decoding */
    if (len < 4 || strcmp(argv[2] + len - 4, ".bmp") != 0)
    {
        printf("Error: Stego image must have .bmp extension\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if (argv[3] == NULL)
    {
        strcpy(decInfo->output_fname, "decoded.txt");
        decInfo->output_name_provided = 0;
    }
    else
    {
        decInfo->output_name_provided = 1;
        len = strlen(argv[3]);

        if (len >= 4 &&
            strcmp(argv[3] + len - 4, ".txt") == 0)
        {
            if (len >= sizeof(decInfo->output_fname))
            {
                printf("Error: Output filename too long\n");
                return e_failure;
            }

            strcpy(decInfo->output_fname, argv[3]);
        }
        else
        {
            if (len + 4 >= sizeof(decInfo->output_fname))
            {
                printf("Error: Output filename too long\n");
                return e_failure;
            }

            strcpy(decInfo->output_fname, argv[3]);
            strcat(decInfo->output_fname, ".txt");
        }
    }

    return e_success;
}

/* Close all files opened during decoding to avoid resource leaks */
void close_decode_files(DecodeInfo *decInfo)
{
    if (decInfo->fptr_output != NULL)
    {
        fclose(decInfo->fptr_output);
        decInfo->fptr_output = NULL;
    }

    if (decInfo->fptr_stego_image != NULL)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
    }
}