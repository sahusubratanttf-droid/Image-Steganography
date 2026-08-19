#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

#define RED     "\033[1;31m"   //// Defining colour codes
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    // call function to Check operation type
    int ret = check_operation_type(argv);

    if (ret == e_unsupported)
    {
        printf(RED "Invalid args\n" RESET);
        return 0;
    }

    // Encoding
    if (ret == e_encode)
    {
        // Read and validate Encode args from argv 
        int ret;
        ret = read_and_validate_encode_args(argv, &encInfo);
        if (ret == e_failure)
        {
            printf(RED "Invalid args\n" RESET);
            return 0;
        }

        // Perform the encoding 
       ret = do_encoding(&encInfo);
       if (ret == e_failure)
       {
        printf(RED "Encoding failed\n" RESET);
        return 0;
       }
       else
       {
        printf(GREEN "Encoding is successful!\n" RESET);
        return 0;
       }
    }

    // Decoding
    if (ret == e_decode)
    {
        // decoding
        // 1. call read_and_validate_decode_args(argv, decInfo);
        // 2. do_encoding(&decInfo)

        // Read and validate Encode args from argv 
        int ret;
        ret = read_and_validate_decode_args(argv, &decInfo);
        if (ret == e_failure)
        {
            printf(RED "Invalid args\n" RESET);
            return 0;
        }

        // Perform the decoding 
       ret = do_decoding(&decInfo);
       if (ret == e_failure)
       {
        printf(RED "Decoding failed\n" RESET);
        return 0;
       }
       else
       {
        printf(GREEN "Decoding is successful!\n" RESET);
        return 0;
       }
    }

}

OperationType check_operation_type(char *argv[])
{
   // Step 1: Check if argv[1] is NULL
    if (argv[1] == NULL)
    {
        return e_unsupported;
    }

    // Step 2: Compare with "-e"
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }

    // Step 3: Compare with "-d"
    if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }

    // Step 4: Default case
    return e_unsupported;

}