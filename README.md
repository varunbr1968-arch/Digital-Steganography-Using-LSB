# Digital Steganography Using LSB Mechanism in C

## Overview

A command-line application developed in C to hide text data inside BMP images using the Least Significant Bit (LSB) mechanism. It also supports extracting the hidden message from the encoded image.

The project demonstrates bitwise manipulation, binary file handling, image data processing, and modular programming in C.

## Features

- Hide text messages inside BMP image files.
- Extract hidden messages from stego images.
- Embed a magic string to identify encoded data.
- Store message metadata, including file extension and file size.
- Check image capacity before encoding.
- Perform encoding and decoding through command-line arguments.

## Technologies Used

- **Programming Language:** C
- **Compiler:** GCC
- **Technique:** Least Significant Bit (LSB) Steganography
- **Image Format:** BMP
- **Concepts:** Bitwise operations, pointers, structures, file handling, binary data processing, and command-line arguments.

## Project Files

| **File** | **Description** |
| --- | --- |
| `main.c` | Handles command-line arguments and selects the operation. |
| `encode.c` | Implements the message encoding process. |
| `encode.h` | Contains declarations related to encoding. |
| `decode.c` | Implements hidden message extraction. |
| `decode.h` | Contains declarations related to decoding. |
| `common.h` | Defines common constants used by the project. |
| `types.h` | Defines operation types and status values. |
| `beautiful.bmp` | Input BMP image used for testing, if included. |

## Compilation

Compile the project using GCC:

```bash
gcc -Wall -Wextra main.c encode.c decode.c -o steganography
```

## Usage

### Encoding

Hide a message inside a BMP image:

```bash
./steganography -e beautiful.bmp sample.txt stego.bmp
```

- `beautiful.bmp` — Original BMP image.
- `sample.txt` — Text file containing the message.
- `stego.bmp` — Output image containing the embedded message.

### Decoding

Extract the hidden message:

```bash
./steganography -d stego.bmp decoded.txt
```

- `stego.bmp` — Image containing the embedded message.
- `decoded.txt` — Output file for the extracted message.

Use the command-line options supported by the program when performing encoding and decoding.

## Working Principle

The Least Significant Bit (LSB) technique stores message bits in the least significant bits of image data bytes. During encoding, the application embeds the message and its metadata into the BMP image. During decoding, it retrieves the embedded bits and reconstructs the original text.

## Compatibility and Limitations

- The current implementation is designed for BMP images.
- The message size is limited by the available image capacity.
- Image resizing, modification, or compression may corrupt the embedded data.
- LSB steganography hides data but does not encrypt it.
- Only the file formats and metadata supported by the implementation can be processed.

## Author

Varun B R
