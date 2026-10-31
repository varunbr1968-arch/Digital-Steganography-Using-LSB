#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width = 0, height = 0;

    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");

    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");

    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");

    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    printf("INFO: Opened %s\n", encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");

    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    printf("INFO: Opened %s\n", encInfo->stego_image_fname);

    printf("INFO: Done\n");

    /* No failure return e_success */
    return e_success;
}

/* Check the requested operation type. */
OperationType check_operation_type(char *argv)
{
    if (argv == NULL)
    {
        return e_unsupported;
    }

    if (strcmp(argv, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv, "-d") == 0)
    {
        return e_decode;
    }
    /* Check for an unsupported operation */
    else
    {
        return e_unsupported;
    }
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    /* Validate source image argument */
    if (strstr(argv[2], ".bmp") == NULL)
    {
        return e_failure;
    }

    /* Store the source image filename */
    encInfo->src_image_fname = argv[2];

    /* Validate the secret file argument */
    if (strstr(argv[3], ".txt") == NULL)
    {
        return e_failure;
    }

    /* Store the secret file extension and filename */
    strcpy(encInfo->extn_secret_file, ".txt");

    encInfo->secret_fname = argv[3];

    // Output filename is optional
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        printf("INFO: Output File not mentioned. Creating stego.bmp as default\n");
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}

/* Perform the encoding operation. */
Status do_encoding(EncodeInfo *encInfo)
{
    /* Open the source image, secret file, and destination image */
    if (open_files(encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }

    printf("INFO: ## Encoding Procedure Started ##\n");

    /* Check whether the image has enough capacity */
    if (check_capacity(encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }

    printf("INFO: Copying Image Header\n");

    /* Copy the BMP header to the destination image */
    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Encoding Magic String Signature\n");

    /* Encode the magic string into the image. */
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);

    /* Encode the secret file extension size and extension */
    int extn_size = strlen(encInfo->extn_secret_file);

    if (encode_secret_file_extn_size(extn_size, encInfo) == e_failure ||
        encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);

    /* Encode the secret file size into the image */
    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);

    /* Encode the secret file data into the image */
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    printf("INFO: Copying Left Over Data\n");

    /* Copy the remaining image data */
    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_stego_image) == e_failure)
    {
        close_files(encInfo);
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Close all opened files */
    close_files(encInfo);

    printf("INFO: ########################## Encoding Done Successfully ########################## \n");

    return e_success;
}

/* Check whether the image has sufficient capacity. */
Status check_capacity(EncodeInfo *encInfo)
{
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);

    /* Get the size of the secret file */
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if (encInfo->size_secret_file == 0)
    {
        printf("INFO: Error: Secret file is Empty\n");
        return e_failure;
    }

    printf("INFO: Done. Not Empty\n");

    printf("INFO: Checking for %s capacity to handle %s\n",
           encInfo->src_image_fname, encInfo->secret_fname);

    /* Calculate the total image capacity */
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Calculate the space required to encode the secret file */
    uint size_required = (strlen(MAGIC_STRING) * 8) +
                         (sizeof(int) * 8) +
                         (strlen(encInfo->extn_secret_file) * 8) +
                         (sizeof(int) * 8) +
                         (encInfo->size_secret_file * 8);

    /* Reject the image if its capacity is insufficient */
    if (encInfo->image_capacity == 0 ||
        encInfo->image_capacity < size_required)
    {
        printf("INFO: Error: Insufficient image capacity\n");
        return e_failure;
    }

    printf("INFO: Done. Found OK\n");

    return e_success;
}
uint get_file_size(FILE *fptr)
{
    // moves the file position to the end.
    fseek(fptr, 0, SEEK_END);

    // gets the position that will give the  size of the file.
    uint size = ftell(fptr);

    // moves the file position back to the beginning.
    rewind(fptr);

    return size;
}

/* Copy the BMP header to the destination image. */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    /* Move the source image pointer to the beginning */
    rewind(fptr_src_image);

    /* Read the 54-byte BMP header */
    if (fread(buffer, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    /* Write the BMP header to the destination image */
    if (fwrite(buffer, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];

    // repeat this for strlen(magic_string) times from step 1
    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        // step 1 -> read 8 bytes from source file
        if (fread(buffer, sizeof(char), 8, encInfo->fptr_src_image) != 8)
            return e_failure;

        // step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
        if (encode_byte_to_lsb(magic_string[i], buffer) == e_failure)
        {
            return e_failure;
        }

        // step 3 -> write the buffer into dest file
        if (fwrite(buffer, sizeof(char), 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }

    return e_success;
}

/* Encodes one character using 8 image bytes */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        /* Embed one bit of the character into the LSB */
        image_buffer[i] = (image_buffer[i] & (~1)) | ((data >> (7 - i)) & 1);
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    // step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
        return e_failure;

    // step 2 -> call encode_size_to_lsb(file_size, buffer)
    if (encode_size_to_lsb((int)file_size, buffer) == e_failure)
    {
        return e_failure;
    }

    // step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;

    return e_success;
}

/* Encodes a 32-bit integer using 32 image bytes */
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // write logic to encode the int
    for (int i = 0; i < 32; i++)
    {
        /* Embed one bit of the integer into the LSB */
        imageBuffer[i] = (imageBuffer[i] & (~1)) | ((size >> (31 - i)) & 1); // if (32 - 0) -> UB so 31
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];

    // repeat this for strlen(file_extn) times from step 1 (Repeat for each character in the extension)
    for (size_t i = 0; i < strlen(file_extn); i++) // strlen len return size_t comparing int with size_t causes signed/unsigned warnings
    {
        // step 1 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        // step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)(Encode one extension character)
        if (encode_byte_to_lsb(file_extn[i], buffer) == e_failure)
        {
            return e_failure;
        }

        // step 3 -> write the buffer into dest file(Write the modified bytes to the destination image)
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];

    // Move pointer to beginning of secret file
    rewind(encInfo->fptr_secret);

    // repeat this for size_secret_file times from step 2
    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        // step 1 -> read secret data from file and store it into encInfo -> secret_data
        if (fread(encInfo->secret_data, 1, 1, encInfo->fptr_secret) != 1)
        {
            return e_failure;
        }

        // step 2 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        // step 3 -> call encode_byte_to_lsb(encInfo -> secret_data[i], buffer)(Encode the secret character into the image bytes)
        if (encode_byte_to_lsb(encInfo->secret_data[0], buffer) == e_failure)
        {
            return e_failure;
        }

        // step 4 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    /* Read remaining bytes */
    while (fread(&ch, 1, 1, fptr_src) == 1) // Read remaining bytes
    {
        /* Write to destination */
        if (fwrite(&ch, 1, 1, fptr_dest) != 1) // Write to destination
        {
            return e_failure;
        }
    }

    /* Check if both pointers match */
    if (ftell(fptr_src) == ftell(fptr_dest)) // Check if both pointers match
    {
        return e_success;
    }

    return e_failure;
}

Status encode_secret_file_extn_size(int extn_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    // read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    /* Embed the extension size into the image buffer */
    encode_size_to_lsb(extn_size, buffer);

    // write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

/* Close all opened files */
void close_files(EncodeInfo *encInfo)
{
    /* Close the secret file if it is open */
    if (encInfo->fptr_secret != NULL)
    {
        fclose(encInfo->fptr_secret);
        encInfo->fptr_secret = NULL;
    }

    /* Close the source image if it is open */
    if (encInfo->fptr_src_image != NULL)
    {
        fclose(encInfo->fptr_src_image);
        encInfo->fptr_src_image = NULL;
    }

    /* Close the destination image if it is open */
    if (encInfo->fptr_stego_image != NULL)
    {
        fclose(encInfo->fptr_stego_image);
        encInfo->fptr_stego_image = NULL;
    }
}