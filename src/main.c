#include <stdlib.h>

#include "generated/ass.tab.h"
#include "constants.h"
#include "hash_array.h"
#include "parameters.h"
#include "failure.h"

#define TABLE_SIZE 1024

int main(int argc, char** argv)
{
    enum_array = hash_init(TABLE_SIZE);
    bit_const_array = hash_init(TABLE_SIZE);
    int_const_array = hash_init(TABLE_SIZE);
    str_const_array = hash_init(TABLE_SIZE);
    build_ast(argc,argv);

    size_t count = hash_count(enum_array);
    bucket_t** csnt = hash_serialise(enum_array);

    puts("****PARAM****");

    printf("opcode_width : %i\n", parameters.opcode_width);  
    printf("memory_width : %i\n", parameters.memory_width);  
    printf("alignment : %i\n", parameters.alignment);     
    printf("address_width : %i\n", parameters.address_width); 
    printf("address_start : %i\n", parameters.address_start); 
    printf("address_stop : %i\n", parameters.address_stop);  
    printf("endianness : %s\n", (parameters.endianness == 0)?"UNDEF":((parameters.endianness == 1)?"LITTLE":"BIG"));
    printf("args_separator : %c\n", parameters.args_separator); 
    printf("*label_pattern : %s\n", parameters.label_pattern); 

    puts("****ENUM****");
    for(int i = 0; i < count; i++)
    {
        printf("%s = %i\n",csnt[i]->key, ((enumeration_t *)(csnt[i]->user_data))->width);
        print_list_enum(((enumeration_t *)(csnt[i]->user_data))->pattern_list);
    }

    count = hash_count(bit_const_array);
    csnt = hash_serialise(bit_const_array);
    
    puts("****BIT CONSTANTS****");
    for(int i = 0; i < count; i++)
    {
        printf("%s = %#x:%i\n",csnt[i]->key, ((data_t *)(csnt[i]->user_data))->bVal.val, ((data_t *)(csnt[i]->user_data))->bVal.width);
    }

    count = hash_count(int_const_array);
    csnt = hash_serialise(int_const_array);

    puts("****CONSTANTS****");
    for(int i = 0; i < count; i++)
    {
        printf("%s = %i\n",csnt[i]->key, ((data_t *)(csnt[i]->user_data))->iVal);
    }

    //Report parsing result
    int info_count = fail_get_info_count();
    if(info_count > 0)
        printf("\033[34m>>%i INFO%s<<\033[0m\n", info_count, (info_count==1)?"":"S");

    int warning_count = fail_get_warning_count();
    if(warning_count > 0)
        printf("\033[33m>>%i WARNING%s<<\033[0m\n", warning_count, (warning_count==1)?"":"S");

    int error_count = fail_get_error_count();
    if(error_count > 0)
    {
        printf("\033[31m>>%i ERROR%s<<\033[0m\n", error_count, (error_count==1)?"":"S");
        puts("Parsing failed, exiting.");
        exit(EXIT_FAILURE);
    }

}
