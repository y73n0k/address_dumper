#include <stdio.h>
#include "address_dumper.h"


void printer(char *path, unsigned long long address) {
    printf("FOUND %s AT %llx\n", path, address);
} 


int main(int argc, char *argv[]){
    dumper(argv, "libc.so", 1, &printer);
    return 0;
}
