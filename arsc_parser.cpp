#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils/Unicode.h>

#include "androidfw/ResourceTypes.h"

using namespace android;

static size_t decodeLength(const uint8_t** str)
{
    size_t len = **str;
    if ((len & 0x80) != 0) {
        (*str)++;
        len = ((len & 0x7f) << 8) | **str;
    }

    (*str)++;
    return len;
}



/*
static inline size_t
decodeLength(const uint16_t** str)
{
    size_t len = **str;
    if ((len & 0x8000) != 0) {
        (*str)++;
        len = ((len & 0x7fff) << 16) | **str;
    }

    (*str)++;
    return len;
}
*/

static char* allocFromUTF16(const char16_t* in, size_t len)
{
    if (len == 0) return NULL;

    const ssize_t resultStrLen = utf16_to_utf8_length(in, len) + 1;
    if (resultStrLen < 1) {
        return NULL;
    }

    char* resultStr = (char*)malloc(resultStrLen);

    utf16_to_utf8(in, len, resultStr, resultStrLen);
    return resultStr;
}


int parse_ResTable_header(uint8_t *data) {

    ResTable_header *resHd = (ResTable_header*)data;

    printf(" ### res header ### \n");
    printf ("res type =  %d \n", resHd->header.type);
    printf ("res chunk hd size = %d\n", resHd->header.headerSize);
    printf ("res chunk size = %d \n", resHd->header.size);
    printf ("res packages count = %d \n", resHd->packageCount);

    return resHd->header.headerSize;

}

int parse_ResStringPool(uint8_t *data, uint32_t** mEntries, void ** mString ) {

    ResStringPool_header *resStrPoolHd = (ResStringPool_header *)data;

    printf("#### parse_ResStringPool #### \n");

    printf("type = %d \n", resStrPoolHd->header.type);
    printf("chunk hd size = %d \n", resStrPoolHd->header.headerSize);
    printf("chunk size = %d \n", resStrPoolHd->header.size);
    printf("stringCount = %d \n", resStrPoolHd->stringCount);
    printf("styleCount = %d \n", resStrPoolHd->styleCount);
    printf("flags = %p\n", resStrPoolHd->flags);
    printf("stringsStart = %d \n", resStrPoolHd->stringsStart);
    printf("stylesStart = %d \n", resStrPoolHd->stylesStart);


    *mEntries = (uint32_t*)(data + resStrPoolHd->header.headerSize);
    *mString = (void *)((const uint8_t*)resStrPoolHd + resStrPoolHd->stringsStart);

    printf(" in func parse_ResStringPool, mEntries = %p, mString = %p \n", *mEntries, *mString);

    for (int i = 0; i < resStrPoolHd->stringCount; i++) {
        const uint8_t *str = (((uint8_t *)(*mString)) + (*mEntries)[i]); 
        const uint8_t *str2 = str;
        int len = decodeLength(&str);
        uint8_t *p = (uint8_t *)malloc(len + 1);
        memcpy(p, str2 +2, len);
        p[len] = 0;
        printf("IDX : %d, len : %d,  string:  %s \n", i, len, p);
        free(p);
    }

    return resStrPoolHd->header.size;
}

int parse_ResTablePackage(uint8_t *data, uint32_t* mEntries, void* mString ) {
    ResTable_package  *resTablePackageHd = (ResTable_package *)data;

    printf(" ###############ResTablePackage Header ####\n");
    printf("resTablePackage type = %p \n", resTablePackageHd->header.type);
    printf("resTablePackage chunk hd size = %d \n", resTablePackageHd->header.headerSize);
    printf("resTablePackage chunk size  = %d \n", resTablePackageHd->header.size);

    printf("resTablePackage id = %p \n", resTablePackageHd->id);

    char *name = allocFromUTF16((char16_t* )resTablePackageHd->name, 128);
    printf("resTable Package NAME = %s \n", name );
    free(name);
    name = NULL;

    printf("resTablePackage typeStrings = %d\n", resTablePackageHd->typeStrings);
    printf("resTablePackage lastPublicType = %d \n", resTablePackageHd->lastPublicType);
    printf("resTablePackage keyStrings = %d \n", resTablePackageHd->keyStrings);
    printf("resTablePackage lastPublicKey = %d \n", resTablePackageHd->lastPublicKey);
    printf("resTablePackage typeIdOffset = %d \n", resTablePackageHd->typeIdOffset);

    printf("Type string pool info ....\n");

    uint32_t *mTypeEntries;
    void *mTypeString;
    uint32_t *mKeyEntries;
    void* mKeyString;

    int offset1 = parse_ResStringPool(data + resTablePackageHd->typeStrings, &mTypeEntries, &mTypeString);
    int offset2 = parse_ResStringPool(data + resTablePackageHd->keyStrings, &mKeyEntries, &mKeyString);

    printf("mTypeEntries = %p, mTypeString = %p ...\n", mTypeEntries, mTypeString);
    printf("mKeyEntries = %p, mKeyString = %p ...\n", mKeyEntries, mKeyString);
 

    printf(" offset1 = %d , offset2 = %d ..\n", offset1, offset2);
    printf(" #### type spec info ####\n");

    ResTable_typeSpec *resTableTypeSpecHd = NULL;
    ResTable_type *resTableTypeHd = NULL;

    struct ResChunk_header* chunk_hd = (struct ResChunk_header *) ((uint8_t*) (data + resTablePackageHd->header.headerSize + offset1 + offset2) );
    
    //resTableTypeSpecHd = (struct ResTable_typeSpec *) ((uint8_t*) (data + resTablePackageHd->header.headerSize + offset1 + offset2) );
    
    int count = resTablePackageHd->lastPublicType;

    char* resName;

    for (int i = 0; i< count; i++) {
        
         if (chunk_hd->type == RES_TABLE_TYPE_SPEC_TYPE ) {
            resTableTypeSpecHd = (ResTable_typeSpec*)chunk_hd;

            resName = (char *)((uint8_t *)mTypeString + mTypeEntries[resTableTypeSpecHd->id -1 ] +2);
            printf(" ##### resTableTypeSpec ### \n");
            printf("resTableTypeSpecHd name = %s \n", resName );

            printf("resTableTypeSpecHd type = %p \n", resTableTypeSpecHd->header.type);
            printf("resTableTypeSpecHd chunk hd size = %d \n", resTableTypeSpecHd->header.headerSize);
            printf("resTableTypeSpecHd chunk size = %d \n", resTableTypeSpecHd->header.size);
            printf("resTableTypeSpecHd id = %d \n", resTableTypeSpecHd->id);
            printf("resTableTypeSpecHd entryCount = %d \n", resTableTypeSpecHd->entryCount);

            if (!strcmp(resName, "drawable")) {
                uint32_t *specs =  (uint32_t *)((uint8_t *)resTableTypeSpecHd + resTableTypeSpecHd->header.headerSize );
                for (int m = 0; m < resTableTypeSpecHd->entryCount; m++) { 
                    printf (" 0x %x \n", specs[m] );
                }
            }

            //resTableTypeSpecHd = resTableTypeSpecHd + resTableTypeSpecHd->header.size;
            chunk_hd = (struct ResChunk_header*)((uint8_t*)chunk_hd + chunk_hd->size);
        }

         while (chunk_hd->type == RES_TABLE_TYPE_TYPE) {

             resTableTypeHd = (ResTable_type*)chunk_hd;
             if (!strcmp(resName, "drawable")) {
                 printf("RES_TABLE_TYPE_TYPE id = %d \n", resTableTypeHd->id );
                 printf("ResTable_type.entryCount = %d \n", resTableTypeHd->entryCount);

                 uint32_t *su = (uint32_t *)((uint8_t *)resTableTypeHd + resTableTypeHd->header.headerSize);

                 // ResTable_entry data starts
                 uint8_t *addr = (uint8_t*)((uint8_t *)resTableTypeHd + resTableTypeHd->header.headerSize + resTableTypeHd->entryCount*sizeof(uint32_t));
                 for (int k = 0; k < resTableTypeHd->entryCount ; k++) {

                     if (su[k] != 0xffffffff) {

                         ResTable_entry *entry = (ResTable_entry *) (addr + su[i] );

                         printf ("entry flags: 0x %x\n", entry->flags);
                         if (entry->flags & ResTable_entry::FLAG_COMPLEX != 0) {
                             continue;
                         } else {
                             //printf (" string : %s \n", ((char*) mKeyString)+mKeyEntries[entry->key.index]);
                             Res_value* value = (Res_value*)((uint8_t*)entry + entry->size);
                         }

                     }

                 }
             }
             chunk_hd = (struct ResChunk_header*)((uint8_t*)chunk_hd + chunk_hd->size);
         }
        
    }

    return resTablePackageHd->header.size;
}



int main(int argc, char** argv)
{

    struct stat buf;
    stat("./resources.arsc", &buf);

    int fd = open("./resources.arsc", 0644);
    uint8_t *data = (uint8_t*)mmap(NULL, buf.st_size, PROT_READ,MAP_PRIVATE, fd, 0);

    int offset1, offset2, offset3;

    uint32_t *mEntries ;
    void* mString;

    offset1 = parse_ResTable_header(data);

    offset2 = parse_ResStringPool((uint8_t *)(data + offset1), &mEntries, &mString);

    offset3 = parse_ResTablePackage((uint8_t *)(data + offset1 + offset2), mEntries, mString);

    close(fd);
    return 0;
}
