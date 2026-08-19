#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    char image_data[MAX_IMAGE_BUF_SIZE];
    long image_capacity;          // total bytes in source image


    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];  // e.g. ".txt"
    int extn_len;                 // length of extension string
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;      // size of secret file in bytes

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Metadata for encoding */
    int magic_str_len;            // length of magic string (e.g. "#*")

} EncodeInfo;


/* Encoding function prototype */

/* Check operation type */
OperationType check_operation_type(char *argv[]);

Status set_file_extension(EncodeInfo *encInfo);

Status open_files(EncodeInfo *encInfo);

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

Status do_encoding(EncodeInfo *encInfo);

Status check_capacity(EncodeInfo *encInfo);

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

Status encode_byte_to_lsb(char data, char* image_buffer);

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

Status encode_size_lsb(int data, char* image_buffer);

Status encode_secret_file_extn_size(int file_extn_size, EncodeInfo *encInfo);

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo);

Status encode_secret_file_data(EncodeInfo *encInfo);

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif