#include <stdio.h>
#include <stdint.h>

void printBinary(uint8_t val, char* str){

        for(int i=0; i<8;i++){

                str[7-i] = (val>>i)&0x01 ? '1':'0';
        }
}

int main(){

        uint32_t actualImageSize = 3;
        //Test data
        uint32_t jpeg_buffer[3] = {0xC7A500FF, 0x01234567, 0x89ABCDEF};

        //Manually entered expected results. If the results are equl to this, we know the code works.
        uint32_t expectedValues[3] = {0xC79600FF, 0x02134657, 0x8A9BCEDF};

        uint32_t results[3];

        for(int i=0; i<actualImageSize;i++){
                
                uint32_t temp = jpeg_buffer[i];
                uint8_t LeftByte = (temp>>24) & 0xFF; //The left most byte. XX_YY_ZZ_AA >> 24 == 00_00_00_XX.
                uint8_t MidLeftByte = (temp>>16) & 0xFF; //The second left most byte. XX_YY_ZZ_AA >> 16 -> 00_00_XX_YY then & 0xFF -> 00_00_00__YY
                uint8_t MidRightByte = (temp>>8) & 0xFF; //Middle right byte
                uint8_t RightByte = (temp & 0xFF); // The right most byte.
                
                printf("Byte %d: %x -> %x %x %x %x\n",i,temp,LeftByte,MidLeftByte,MidRightByte,RightByte);
                
                char binStr[9]={0};
                char binStr2[9]={0};
                printBinary(LeftByte,binStr);
                uint8_t new_LB = ((LeftByte>>1)&0x11)  | ((LeftByte<<1)& 0x22) | (LeftByte & 0xCC); 
                printBinary(new_LB,binStr2);
                printf("%x = %s -> %s  = %x\n",LeftByte,binStr,binStr2,new_LB);

                printBinary(MidLeftByte,binStr);
                uint8_t new_MLB = ((MidLeftByte>>1)&0x11)  | ((MidLeftByte<<1)& 0x22) | (MidLeftByte & 0xCC); 
                printBinary(new_MLB,binStr2);
                printf("%x = %s -> %s  = %x\n",MidLeftByte,binStr,binStr2,new_MLB);

                printBinary(MidRightByte,binStr);
                uint8_t new_MRB = ((MidRightByte>>1)&0x11)  | ((MidRightByte<<1)& 0x22) | (MidRightByte & 0xCC); 
                printBinary(new_MRB,binStr2);
                printf("%x = %s -> %s  = %x\n",MidRightByte,binStr,binStr2,new_MRB);

                printBinary(RightByte,binStr);
                uint8_t new_RB = ((RightByte>>1)&0x11)  | ((RightByte<<1)& 0x22) | (RightByte & 0xCC); 
                printBinary(new_RB,binStr2);
                printf("%x = %s -> %s  = %x\n",RightByte,binStr,binStr2,new_RB);


                uint32_t newByte = (new_LB << 24) + (new_MLB << 16) + (new_MRB <<8) + new_RB;
                results[i] = newByte;
                
        }
        int failed = 0;
        for(int i=0; i< actualImageSize; i++){
                if(results[i] != expectedValues[i]){
                        printf("TEST FAILED ON DATA %d! %x != %x\n",i,results[i],expectedValues[i]);
                        failed ++;
                }
        }
        if(failed == 0){
                printf("Test passed :)\n");
        }
        return 1;
}
