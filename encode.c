#include <stdio.h>
#include "encode.h"
#include "common.h"
#include "types.h"
#include <string.h>

#define RED     "\033[1;31m"   // Defining colour codes
#define GREEN   "\033[1;32m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status open_files(EncodeInfo *encInfo)
{
    printf(BLUE "INFO: Opening required files\n" RESET);

    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr,RED "ERROR: Unable to open file %s\n" RESET, encInfo->src_image_fname);

    	return e_failure;
    }
    printf(GREEN "INFO: Opened %s\n" RESET, encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr,RED "ERROR: Unable to open file %s\n" RESET, encInfo->secret_fname);

    	return e_failure;
    }
    printf(GREEN "INFO: Opened %s\n" RESET, encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, RED "ERROR: Unable to open file %s\n" RESET, encInfo->stego_image_fname);

    	return e_failure;
    }
    printf(GREEN "INFO: Opened %s\n" RESET, encInfo->stego_image_fname);
    printf(GREEN "INFO: Done\n" RESET);

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(argv[2] == NULL)
    {
        printf(RED ".bmp file not passed\n" RESET);
        return e_failure;
    }

    if(strstr(argv[2], ".bmp")==NULL)
    {
        printf(RED "Invalid image file name\n" RESET);
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    if(argv[3] == NULL)
    {
        printf(RED ".txt file not passed\n" RESET);
        return e_failure;
    }

    if(strstr(argv[3], ".txt") == NULL)
    {
        printf(RED "Invalid secret file name\n" RESET);
        return e_failure;
    }
    encInfo->secret_fname = argv[3];

    if (argv[4] == NULL)
    {
        printf(BLUE "INFO: Output File not mentioned. Creating steged_img.bmp as default\n" RESET);
        encInfo->stego_image_fname = "steged_img.bmp";
    }
    else
    {
        if (strstr(argv[4], ".bmp") == NULL)
        {
            printf(RED "Invalid stego image file name\n" RESET);
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
    }
   
    char *ch = strchr(encInfo->secret_fname, '.');
    strcpy(encInfo->extn_secret_file, ch);
   
    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
    printf(GREEN "INFO: ## Encoding Procedure Started ##\n" RESET);

    int ret = open_files(encInfo);
    if (ret == e_failure)
    {
        printf(RED "open files failed\n" RESET);
        return e_failure;
    }

    ret = check_capacity(encInfo);
    if (ret == e_failure)
    {
        printf(RED "check capacity failed\n" RESET);
        return e_failure;
    }
    
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    ret=encode_magic_string(MAGIC_STRING,encInfo);
    if(ret==e_failure)
    {
        printf(RED "encoding magic string failed\n" RESET);
        return e_failure;
    }
    
    ret = encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);
    if(ret==e_failure)
    {
        printf(RED "file extension size encoded failed\n" RESET);
        return e_failure;
    }
    
    ret = encode_secret_file_extn((encInfo->extn_secret_file), encInfo);
    if(ret==e_failure)
    {
        printf(RED "file extension encoded failed\n" RESET);
        return e_failure;
    }

    ret=encode_secret_file_size(encInfo->size_secret_file, encInfo);
    if(ret==e_failure)
    {
       printf(RED "Secret file size encoding failed\n" RESET);
       return e_failure;
    }

    ret=encode_secret_file_data(encInfo);
    if(ret==e_failure)
    {
       printf(RED "Encoding of secret file data failed\n" RESET);
       return e_failure;
    } 

    ret=copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    if(ret==e_failure)
    {
      printf(RED "Encoding remaining data failed\n" RESET);
      return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    printf(GREEN "INFO: ## Encoding Done Successfully ##\n" RESET);
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    int src_file_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    fseek(encInfo->fptr_src_image,0,SEEK_SET);
    fseek(encInfo->fptr_secret,0,SEEK_END);
    encInfo->size_secret_file=ftell(encInfo->fptr_secret);
    fseek(encInfo->fptr_secret,0,SEEK_SET);

    int count=((strlen(MAGIC_STRING)+sizeof(int)+strlen(encInfo->extn_secret_file)+
    sizeof(int)+encInfo->size_secret_file)*8)+54;
    if(count<=src_file_size)
    {
        return e_success;
    }
    return e_failure;

    printf(BLUE "INFO: Checking for %s size\n" RESET, encInfo->secret_fname);
    printf(GREEN "INFO: Done. Not Empty\n" RESET);
    printf(BLUE "INFO: Checking for %s capacity to handle %s\n" RESET, encInfo->src_image_fname, encInfo->secret_fname);
    printf(GREEN "INFO: Done. Found OK\n" RESET);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf(BLUE "INFO: Copying Image Header\n" RESET);

    char temp[55];
    fread(temp, 54, 1, fptr_src_image);
    fwrite(temp, 54, 1, fptr_dest_image);

    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}

Status encode_byte_to_lsb(char data, char* image_buffer)
{
    for(int i=0;i<8;i++)
    {
        char mask = 1<<7-i;
        char bit = data & mask;
        image_buffer[i] = image_buffer[i] & 0xfe;
        bit = bit >> 7-i;
        image_buffer[i]=image_buffer[i] | bit;
    }
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf(BLUE "INFO: Encoding Magic String Signature\n" RESET);

    for(int i = 0; i < 2; i++)
    {
        char temp[8];
        fread(temp, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], temp);
        fwrite(temp, 1, 8, encInfo->fptr_stego_image);
    }

    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}
   
Status encode_size_lsb(int data, char* image_buffer)
{
    for(int i=0;i<32;i++)
    {
        char mask = 1<<31-i;
        char bit = data & mask;
        image_buffer[i] = image_buffer[i] & 0xfe;
        bit = bit >> 31-i;
        image_buffer[i]=image_buffer[i] | bit;
    }
}

Status encode_secret_file_extn_size(int file_extn_size, EncodeInfo *encInfo)
{
    printf(BLUE "INFO: Encoding %s File Extension\n" RESET, encInfo->secret_fname);

    char temp_buffer[32];

    fread(temp_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_lsb(file_extn_size, temp_buffer);
    fwrite(temp_buffer, 1, 32, encInfo->fptr_stego_image);

    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
     for(int i=0;i<strlen(file_extn);i++)
     {
        char temp[8];
        fread(temp, 1, 8 , encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], temp);
        fwrite(temp, 1, 8, encInfo->fptr_stego_image);
     }
     return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    uint size=ftell(fptr);
    fseek(fptr,0,SEEK_SET);
    return size;
}

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    printf(BLUE "INFO: Encoding %s File Size\n" RESET, encInfo->secret_fname);

    char temp_buffer[32];
    fread(temp_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_lsb(file_size, temp_buffer);
    fwrite(temp_buffer, 1, 32, encInfo->fptr_stego_image);

    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf(BLUE "INFO: Encoding %s File Data\n" RESET, encInfo->secret_fname);

    char ch=0;
    char temp[8];

    while((ch=fgetc(encInfo->fptr_secret) )!=EOF) 
    {
        fread(temp, 1, 8 , encInfo->fptr_src_image);
        encode_byte_to_lsb(ch, temp);
        fwrite(temp, 1, 8, encInfo->fptr_stego_image);
        
    }
    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    printf(BLUE "INFO: Copying Left Over Data\n" RESET);

    char ch;
    ch= fgetc(fptr_src);
    while(!feof(fptr_src))
    {
       fputc(ch,fptr_dest);
       ch= fgetc(fptr_src);
      
    }
    printf(GREEN "INFO: Done\n" RESET);

    return e_success;
}