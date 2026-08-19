#ifndef DECODE_H
#define DECODE_H

#include"types.h" // Contains user defined types

typedef struct _DecodeInfo
{
    char stego_fname[100];
    char dest_fname[100];
    FILE *fptr_stego;
    FILE *fptr_dest;
} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status skip_bmp_header(FILE *fptr_stego);

Status decode_magic_string(FILE *stego, char *magic_string);

char lsb_to_byte(char *buffer);

int lsb_to_size(char *buffer);

Status decode_extn_size(FILE *stego, int *extn_size);

Status decode_extn(FILE *stego, char *extn, int extn_size);

Status decode_sec_file_size(FILE *stego, int *file_size);

Status decode_sec_data(FILE *stego, FILE *dest, int file_size);

#endif