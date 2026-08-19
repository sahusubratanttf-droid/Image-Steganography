#include <stdio.h>
#include "decode.h"
#include "common.h"
#include <string.h>
#include "types.h"

#define RED     "\033[1;31m"   // Defining colour codes
#define GREEN   "\033[1;32m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
  if(strstr(argv[2], ".bmp") == NULL)
  {
      printf(RED "bmp file not passed\n" RESET);
      return e_failure;
  }
  strcpy(decInfo->stego_fname, argv[2]);
  
  if(argv[3] == NULL)
  {
      strcpy(decInfo->dest_fname, "decoded");
  }
  else
  {
      strcpy(decInfo->dest_fname, argv[3]);
  }
}


Status do_decoding(DecodeInfo *decInfo)
{
    printf(GREEN "INFO: ## Decoding Procedure Started ##\n" RESET);
    printf(BLUE  "INFO: Opening required files\n" RESET);

    decInfo->fptr_stego = fopen(decInfo->stego_fname, "r");

    if(decInfo->fptr_stego == NULL)
    {
      printf(RED "stego file opening failed" RESET);
      return e_failure;
    }
    printf(GREEN "INFO: Opened %s\n" RESET, decInfo->stego_fname); 

    int ret = skip_bmp_header(decInfo->fptr_stego);

    if(ret == e_failure)
    {
      return e_failure;
    }
    printf(GREEN "INFO: Skipped BMP Header\n" RESET);

    printf(BLUE "INFO: Decoding Magic String Signature\n" RESET);

    char magic_string[50];
    ret = decode_magic_string(decInfo->fptr_stego,magic_string);
    if(ret==e_failure)
    {
      printf(RED "Magic string decode failed\n" RESET);
      return e_failure;
    }
    printf(GREEN "INFO: Done\n" RESET);

    char user_magic_string[50];
    printf("Enter magic string: ");
    scanf("%s",user_magic_string);

    if(strcmp(user_magic_string,magic_string) != 0)
    {
      printf(RED "Invalid magic string\n" RESET);
      return e_failure;
    }
    printf(GREEN "INFO: Magic string verified successfully\n" RESET);
  
    printf(BLUE "INFO: Decoding Output File Extension\n" RESET);
    int extn_size;
    ret = decode_extn_size(decInfo->fptr_stego, &extn_size);
    if(ret==e_failure)
    {
      printf(RED "Decode extn size failed\n" RESET);
      return e_failure;
    }
   
    char extn[10];
    ret= decode_extn(decInfo->fptr_stego,extn,extn_size);
    if(ret==e_failure)
    {
      printf(RED "Decode extn failed\n" RESET);
      return e_failure;
    }
    printf(GREEN "INFO: Done\n" RESET);

    if(strcmp(decInfo->dest_fname, "decoded") == 0)  // default case
    {
      strcat(decInfo->dest_fname, extn);
      printf(BLUE "INFO: Output File not mentioned. Creating %s as default\n" RESET, decInfo->dest_fname);
    }
    else
    {
      // User already gave a filename
      printf(BLUE "INFO: Using user-provided output file: %s\n" RESET, decInfo->dest_fname);
    }

    decInfo->fptr_dest = fopen(decInfo->dest_fname, "w");
    if(decInfo->fptr_dest == NULL)
    {
      printf(RED "ERROR: dest file opening failed\n" RESET);
      return e_failure;
    }

    printf(GREEN "INFO: Opened %s\n" RESET, decInfo->dest_fname);
    printf(GREEN "INFO: Done. Opened all required files\n" RESET);

    printf(BLUE "INFO: Decoding %s File Size\n" RESET, decInfo->dest_fname);

    int file_size;
    ret= decode_sec_file_size(decInfo->fptr_stego, &file_size);
    {
      if(ret==e_failure)
      {
        printf(RED "Decode file size failed\n" RESET);
        return e_failure;
      }
    }
    printf(GREEN "INFO: Done\n" RESET);

    printf(BLUE "INFO: Decoding %s File Data\n" RESET, decInfo->dest_fname);
    ret=decode_sec_data(decInfo->fptr_stego,decInfo->fptr_dest,file_size);
    {
      if(ret==e_failure)
      {
        printf(RED "Decode file data failed\n" RESET);
        return e_failure;
      }
    }
    printf(GREEN "INFO: Done\n" RESET);

    fclose(decInfo->fptr_stego);
    fclose(decInfo->fptr_dest);

    printf(GREEN "INFO: ## Decoding Done Successfully ##\n" RESET);

    return e_success;
}

Status skip_bmp_header(FILE *fptr_stego)
{
  fseek(fptr_stego,54,SEEK_SET);
  return e_success; 
}

Status decode_magic_string(FILE *fptr_stego,char *magic_string)
{
  unsigned char temp[8];
  for(int i = 0; i < 2; i++)
  {
      fread(temp, 1, 8,fptr_stego);
      magic_string[i] = lsb_to_byte((char*)temp);
  }
  magic_string[2]='\0';
  return e_success;
}

char lsb_to_byte(char *buffer) 
{
  char data=0;
   for(int i=0;i<8;i++) //8 bytes
   {
    char lsb=buffer[i]&1;
    data = data | (lsb<<(7-i));
   }

   return data;
}

int lsb_to_size(char *buffer)
{
  char size=0;
   for(int i=0;i<32;i++)  //32 bytes
   {
    char lsb=buffer[i]&1;
    size = size | (lsb<<(31-i));
   }

   return size;
}

Status decode_extn_size(FILE *fptr_stego, int *extn_size)
{
   char temp[32];
   fread(temp, 1, 32 ,fptr_stego);
   *extn_size = lsb_to_size(temp);

   return e_success;
}

Status decode_extn(FILE *fptr_stego,char *extn,int extn_size)
{
  char temp[8];
  for(int i=0;i<extn_size;i++)
  {
    fread(temp, 1, 8 ,fptr_stego);
    extn[i]=lsb_to_byte(temp); 
  }
  extn[extn_size]='\0';

  return e_success;
}

Status decode_sec_file_size(FILE *fptr_stego, int *file_size)
{
  char temp[32];
  fread(temp, 1, 32 ,fptr_stego);
  *file_size = lsb_to_size(temp);

  return e_success;
}

Status decode_sec_data(FILE *fptr_stego,FILE *fptr_dest,int file_size)
{
  char ch=0;
  char temp[8];
  for(int i=0;i<file_size;i++)
  {
    fread(temp, 1, 8 , fptr_stego);
    char ch = lsb_to_byte(temp);
    fputc(ch,fptr_dest);
  }

  return e_success;
}




