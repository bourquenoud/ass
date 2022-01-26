#include <stdlib.h>

#include "generated/ass.tab.h"
#include "constants.h"
#include "enumerations.h"
#include "hash_array.h"


#define TABLE_SIZE 1024

int main(int argc, char** argv)
{
    enum_array = hash_init(TABLE_SIZE);
    const_array = hash_init(TABLE_SIZE);
    build_ast(argc,argv);

    size_t count = hash_count(enum_array);
    bucket_t** csnt = hash_serialise(enum_array);

    puts("****ENUM****");
    for(int i = 0; i < count; i++)
    {
        printf("%s = %i\n",csnt[i]->key, ((data_t *)(csnt[i]->user_data))->iVal);
    }

    count = hash_count(const_array);
    csnt = hash_serialise(const_array);
    
    puts("****CONSTANTS****");
    for(int i = 0; i < count; i++)
    {
        printf("%s = %#x:%i\n",csnt[i]->key, ((data_t *)(csnt[i]->user_data))->bVal.val, ((data_t *)(csnt[i]->user_data))->bVal.width);
    }
}
