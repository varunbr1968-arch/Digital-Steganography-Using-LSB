/*
Documentation

Name        : VARUN B R
Student id  : 26018_040
Batch id    : 26018D
Start Date  : 08/10/2026
End date    : 22/10/2026

Description:
The following main functions are implemented in the Steganography project to hide secret text data inside a BMP image and retrieve the hidden data using the Least Significant Bit (LSB) technique.

main()                              :-> Handles command-line arguments and selects the required operation such as encoding or decoding.
                                     -> Validates the arguments and calls the appropriate encoding or decoding functions.

do_encoding()                       :-> Hides secret text data inside a BMP image using the LSB technique.
                                     -> Copies the BMP header, encodes the magic string, secret file extension, file size and secret data, and copies the remaining image data.

do_decoding()                       :-> Extracts hidden secret data from a stego image.
                                     -> Validates the magic string, retrieves the secret file extension and file size, and writes the decoded data into an output file.

read_and_validate_encode_args()     :-> Validates the source BMP image, secret text file and optional destination BMP filename.
                                     -> Stores the filenames required for encoding.

read_and_validate_decode_args()     :-> Validates the stego BMP image and optional output text filename.
                                     -> Sets the default output filename when one is not specified.

check_capacity()                    :-> Checks whether the source BMP image has sufficient capacity to store the secret data.
                                     -> Compares the image capacity with the space required for the secret file and its metadata.

encode_byte_to_lsb()                :-> Encodes one byte of secret data into the Least Significant Bits of image data bytes.

decode_byte_from_lsb()              :-> Extracts one byte of hidden data from the Least Significant Bits of image data bytes.

encode_magic_string()               :-> Stores a predefined magic string in the image to identify encoded images.

decode_magic_string()               :-> Extracts and validates the magic string from the stego image.

encode_secret_file_data()           :-> Reads the secret text data and embeds it into the source BMP image.

decode_secret_file_data()           :-> Extracts the hidden secret data and writes it into the output file.


Sample input:

Encoding:
$ ./a.out -e beautiful.bmp secret.txt stego.bmp

Decoding:
$ ./a.out -d stego.bmp output.txt

Sample output:
                Success: Arguments validated
                INFO: Opening required files
                INFO: Opened beautiful.bmp
                INFO: Opened secret.txt
                INFO: Opened stego.bmp
                INFO: Done
                INFO: ## Encoding Procedure Started ##
                INFO: Checking for secret.txt size
                INFO: Done. Not Empty
                INFO: Checking for beautiful.bmp capacity to handle secret.txt
                width = 1024
                height = 768
                INFO: Done. Found OK
                INFO: Copying Image Header
                INFO: Done
                INFO: Encoding Magic String Signature
                INFO: Done
                INFO: Encoding secret.txt File Extension
                INFO: Done
                INFO: Encoding secret.txt File Size
                INFO: Done
                INFO: Encoding secret.txt File Data
                INFO: Done
                INFO: Copying Left Over Data
                INFO: Done
                INFO: ########################## Encoding Done Successfully ##########################
                Success: Encoding completed

                INFO: ## Decoding Procedure Started ##
                INFO: Opening required files
                INFO: Opened stego.bmp
                INFO: Decoding Magic String Signature
                Magic string matched successfully
                INFO: Done
                INFO: Decoding Output File Extension
                Secret extension length: 4
                Secret extension: .txt
                INFO: Done
                INFO: Opened output.txt
                INFO: Done. Opened all required files
                INFO: Decoding output.txt File Size
                Secret file size: 24 bytes
                INFO: Done
                INFO: Decoding output.txt File Data
                INFO: Done
                INFO: ################### Decoding Done Successfully ###################
                Decoding completed successfully
*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
int main(int argc, char *argv[])
{
    /* step1 : Check whether an operation is provided */
    if (argc < 2)
    {
        printf("Error: Missing operation\n");
        printf("Usage: %s -e <source.bmp> <secret.txt> [output.bmp]\n", argv[0]);
        return 1;
    }
    /* Step 2: Identify the requested operation */
    OperationType op = check_operation_type(argv[1]);

    if (op == e_encode)
    {
        /* Step 3: Validate the number of command-line arguments for encoding */
        if (argc < 4 || argc > 5)
        {
            printf("Usage: %s -e <source.bmp> <secret.txt> [output.bmp]\n", argv[0]);
            return 1;
        }

        /* Step 4: Initialize the structure used to store encoding information */
        EncodeInfo encInfo = {0};

        /* Step 5: Validate the encoding filenames and select the output image */
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("Error: Argument validation failed\n");
            return 1;
        }

        printf("Success: Arguments validated\n");

        /* Step 6: Perform the encoding operation */
        if (do_encoding(&encInfo) == e_failure)
        {
            printf("Error: Encoding failed\n");
            return 1;
        }

        printf("Success: Encoding completed\n");
        return 0;
    }
    else if (op == e_decode)
    {
        /* Step 7: Validate the number of command-line arguments for decoding */
        if (argc < 3 || argc > 4)
        {
            printf("ERROR ");
            printf("Usage: %s -d [encoded_file].bmp [output_file]\n", argv[0]);
            return 1;
        }

        /* Step 8: Initialize the structure used to store decoding information */
        DecodeInfo decInfo = {0};

        /* Step 9: Validate the decoding filenames and select the output filename */
        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            return 1;
        }

        /* Step 10: Perform the decoding operation */
        if (do_decoding(&decInfo) == e_failure)
        {
            printf("ERROR : Decoding failed\n");
            return 1;
        }

        printf("Decoding completed successfully\n");

        return 0;
    }
    /* Handle an unsupported operation */
    else
    {
        printf("Error: Unsupported operation\n");
        return 1;
    }
}
