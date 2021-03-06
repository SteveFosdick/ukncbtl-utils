/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// Sav2Cart.cpp

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>


#define BYTE uint8_t
#define WORD uint16_t


static WORD const loader[] =
{
    0000240,  // 000000  000240  NOP
    0012702,  // 000002  012702  MOV     #000104, R2    ; ����� ������� ����������
    0000104,
    0110062,  // 000006  110062  MOVB    R0, 000003(R2)
    0000003,
    0012701,  // 000012  012701  MOV     #000005, R1
    0000005,
    0012703,  // 000016  012703  MOV     #000116, R3
    0000116,
    0000402,  // 000022  000402  BR      000030
    0112337,  // 000024  112337  MOVB    (R3)+, @#176676
    0176676,
    0105737,  // 000030  105737  TSTB    @#176674
    0176674,
    0100375,  // 000034  100375  BPL     000030
    0077106,  // 000036  077106  SOB     R1, 000024
    0105712,  // 000040  105712  TSTB    (R2)
    0001356,  // 000042  001356  BNE     000000
    // ������� ����������� �����
    0005003,  // 000044  005003  CLR     R3
    0012701,  // 000046  012701  MOV     #001000, R1
    0001000,  // 000050  001000
    0012702,  // 000052  012702  MOV     #027400, R2
    0027400,  // 000054  027400
    0062103,  // 000056  062103  ADD     (R1)+, R3
    0005503,  // 000060  005503  ADC     R3
    0077203,  // 000062  077203  SOB     R2, 000056
    0020327,  // 000064  020327  CMP     R3, #CHKSUM
    0000000,  // 000066  ?????? <= CHKSUM
    0001343,  // 000070  001343  BNE     000000
    // ������ ����������� ��������� �� ����������
    0012706,  // 000072  016706  MOV	#STACK, SP
    0001000,  // 000074  ?????? <= STACK
    0000137,  // 000076  000137  JMP    START   ; ������� �� ����������� ���
    0001000,  // 000100  ?????? <= START
    0000240,  // 000102 NOP
    // ������ ���������� ��� ��������� ������ � ������� ��� ����� ����� 2
    0004000,  // 000104  004000   ; ������� (10) � �����
    0000021,  // 000106  000021   ; ����� ������� � ����� ����������
    0001000,  // 000110  001000   ; ����� �� ������ ������� ���
    0001000,  // 000112  001000   ; ����� � ���
    0027400,  // 000114  027400   ; ���������� ���� = 12032. ���� = 24064. ����
    0000104,  // 000116
    0177777,  // 000120
};

static WORD const loaderRLE[] =
{
    0000240,  // 000000  000240  NOP
    0012702,  // 000002  012702  MOV     #000104, R2    ; ����� ������� ����������
    0000104,
    0110062,  // 000006  110062  MOVB    R0, 000003(R2)
    0000003,
    0012701,  // 000012  012701  MOV     #000005, R1
    0000005,
    0012703,  // 000016  012703  MOV     #000116, R3
    0000116,
    0000402,  // 000022  000402  BR      000030
    0112337,  // 000024  112337  MOVB    (R3)+, @#176676
    0176676,
    0105737,  // 000030  105737  TSTB    @#176674
    0176674,
    0100375,  // 000034  100375  BPL     000030
    0077106,  // 000036  077106  SOB     R1, 000024
    0105712,  // 000040  105712  TSTB    (R2)
    0001356,  // 000042  001356  BNE     000000
    // ������� ����������� �����
    0005003,  // 000044  005003  CLR     R3
    0012701,  // 000046  012701  MOV     #001000, R1
    0100000,  // 000050  100000
    0012702,  // 000052  012702  MOV     #027400, R2
    0027400,  // 000054  027400
    0062103,  // 000056  062103  ADD     (R1)+, R3
    0005503,  // 000060  005503  ADC     R3
    0077203,  // 000062  077203  SOB     R2, 000056
    0020327,  // 000064  020327  CMP     R3, #CHKSUM
    0000000,  // 000066  ?????? <= CHKSUM
    0001343,  // 000070  001343  BNE     000000
    0000413,  // 000072  000413  BR      000122        ; ������� �� RLE decoder
    // ������ ����������� ��������� �� ����������
    0012706,  // 000074  016706  MOV	#STACK, SP
    0001000,  // 000076  ?????? <= STACK
    0000137,  // 000100  000137  JMP    START   ; ������� �� ����������� ���
    0001000,  // 000102  ?????? <= START
    // ������ ���������� ��� ��������� ������ � ������� ��� ����� ����� 2
    0004000,  // 000104  004000   ; ������� (10) � �����
    0000021,  // 000106  000021   ; ����� ������� � ����� ����������
    0001000,  // 000110  001000   ; ����� �� ������ ������� ���
    0100000,  // 000112  100000   ; ����� � ��� == RLESTA
    0027400,  // 000114  027400   ; ���������� ���� = 12032. ���� = 24064. ����
    0000104,  // 000116
    0177777,  // 000120
    // RLE decoder
    0012701,  // 000122  012701      MOV  #RLESTA, R1
    0100000,  // 000124  100000 <= RLESTA                ; start address of RLE block
    0012702,  // 000126  012702      MOV  #1000, R2
    0001000,  // 000130  001000                          ; destination start address
    0112100,  // 000132  112100  $1: MOVB (R1+), R0
    0001757,  // 000134  001757      BEQ  000074         ; 0 => decoding finished, let's start the application
    0010003,  // 000136  010003      MOV  R0, R3         ; prepare counter
    0042703,  // 000140  042703      BIC  #177740, R3    ; only lower 5 bits are significant
    0177740,  // 000142  177740
    0105700,  // 000144  105700      TSTB R0             ; 1-byte command?
    0100002,  // 000146  100002      BPL  $2             ; yes => jump
    0000303,  // 000150  000303      SWAB R3             ; move lower byte to high byte
    0152103,  // 000152  152103      BISB (R1)+, R3      ; set lower byte of counter
    0005004,  // 000154  005003  $2: CLR  R4             ; clear filler
    0142700,  // 000156  142700      BICB #237, R0
    0000237,  // 000160  000237
    0001411,  // 000162  001411      BEQ  $3             ; zero pattern? => jump
    0012704,  // 000164  012704      MOV  #377, R4       ;
    0000377,  // 000166  000377
    0122700,  // 000170  122700      CMPB #140, R0       ; #377 pattern?
    0000140,  // 000172  000140
    0001404,  // 000174  001404      BEQ  $3             ; yes => jump
    0122700,  // 000176  122700      CMPB #40, R0       ; given pattern?
    0000040,  // 000200  000040
    0001004,  // 000202  001004      BNE  $4             ; no => jump
    0112104,  // 000204  112104      MOVB (R1+), R4      ; read the given pattern
    0110422,  // 000206  110422  $3: MOVB R4, (R2+)      ; loop: write pattern to destination
    0077302,  // 000210  077302      SOB  R3, $3         ;
    0000747,  // 000212  000747      BR   $1
    0112104,  // 000214  112104  $4: MOVB (R1+), R4      ; loop: copy bytes to destination
    0110422,  // 000216  110422      MOVB R4, (R2+)
    0077303,  // 000220  077303      SOB  R3, $4
    0000743,  // 000222  000743      BR   $1
};

size_t EncodeRLE(const BYTE * source, size_t sourceLength, BYTE * buffer, size_t bufferLength)
{
    size_t destOffset = 0;
    size_t seqBlockOffset = 0;
    size_t seqBlockSize = 1;
    size_t varBlockOffset = 0;
    size_t varBlockSize = 1;
    BYTE prevByte = source[0];
    size_t currOffset = 0;
    size_t codedSizeTotal = 0;
    while (currOffset < sourceLength)
    {
        currOffset++;
        BYTE currByte = (currOffset < sourceLength) ? source[currOffset] : ~prevByte;

        if ((currOffset == sourceLength) ||
            (currByte != prevByte && seqBlockSize > 31) ||
            (currByte != prevByte && seqBlockSize > 1 && prevByte == 0) ||
            (currByte != prevByte && seqBlockSize > 1 && prevByte == 0xff) ||
            (seqBlockSize == 0x1fff || varBlockSize - seqBlockSize == 0x1fff))
        {
            if (varBlockOffset < seqBlockOffset)
            {
                size_t varSize = varBlockSize - seqBlockSize;
                if (currOffset == sourceLength && seqBlockSize < 2)
                    varSize = varBlockSize;  // Special case at the end of input stream
                size_t codedSize = varSize + ((varSize < 256 / 8) ? 1 : 2);
                //printf("RLE  at\t%06o\tVAR  %06o  %06o  %06o\t", varBlockOffset + 512, destOffset, varSize, codedSize);
                codedSizeTotal += codedSize;
                if (destOffset + codedSize < bufferLength)
                {
                    BYTE flagByte = 0x40;
                    if (varSize < 256 / 8)
                    {
                        //printf("%02x ", (BYTE)(flagByte | varSize));
                        buffer[destOffset++] = (BYTE)(flagByte | varSize);
                    }
                    else
                    {
                        //printf("%02x ", (BYTE)(0x80 | flagByte | ((varSize & 0x1f00) >> 8)));
                        buffer[destOffset++] = (BYTE)(0x80 | flagByte | ((varSize & 0x1f00) >> 8));
                        //printf("%02x ", (BYTE)(varSize & 0xff));
                        buffer[destOffset++] = (BYTE)(varSize & 0xff);
                    }
                    for (size_t offset = varBlockOffset; offset < varBlockOffset + varSize; offset++)
                    {
                        //printf("%02x ", source[offset]);
                        buffer[destOffset++] = source[offset];
                    }
                }
                //printf("\n");
            }
            if ((varBlockOffset < seqBlockOffset && seqBlockSize > 1) ||
                (varBlockOffset == seqBlockOffset && varBlockSize == seqBlockSize))
            {
                size_t codedSize = ((seqBlockSize < 256 / 8) ? 1 : 2) + ((prevByte == 0 || prevByte == 255) ? 0 : 1);
                //printf("RLE  at\t%06o\tSEQ  %06o  %06o  %06o\t%02x\n", seqBlockOffset + 512, destOffset, seqBlockSize, codedSize, prevByte);
                codedSizeTotal += codedSize;
                if (destOffset + codedSize < bufferLength)
                {
                    BYTE flagByte = ((prevByte == 0) ? 0 : ((prevByte == 255) ? 0x60 : 0x20));
                    if (seqBlockSize < 256 / 8)
                        buffer[destOffset++] = (BYTE)(flagByte | seqBlockSize);
                    else
                    {
                        buffer[destOffset++] = (BYTE)(0x80 | flagByte | ((seqBlockSize & 0x1f00) >> 8));
                        buffer[destOffset++] = (BYTE)(seqBlockSize & 0xff);
                    }
                    if (prevByte != 0 && prevByte != 255)
                        buffer[destOffset++] = prevByte;
                }
            }

            seqBlockOffset = varBlockOffset = currOffset;
            seqBlockSize = varBlockSize = 1;

            prevByte = currByte;
            continue;
        }

        varBlockSize++;
        if (currByte == prevByte)
        {
            seqBlockSize++;
        }
        else
        {
            seqBlockSize = 1;  seqBlockOffset = currOffset;
        }

        prevByte = currByte;
    }

    //printf("RLE Source size: %d  Coded size: %d  Dest offset: %d\n", sourceLength, codedSizeTotal, destOffset);
    printf("Source size: %d. RLE encoded size: %d.\n", sourceLength, codedSizeTotal);
    return codedSizeTotal;
}

size_t DecodeRLE(const BYTE * source, size_t sourceLength, BYTE * buffer, size_t bufferLength)
{
    size_t currOffset = 0;
    size_t destOffset = 0;
    BYTE filler = 0;
    while (currOffset < sourceLength)
    {
        BYTE first = source[currOffset++];
        if (first == 0)
            break;
        size_t count = 0;
        if ((first & 0x80) == 0)  // 1-byte command
            count = first & 0x1f;
        else  // 2-byte command
            count = (((size_t)first & 0x1f) << 8) | source[currOffset++];
        switch (first & 0x60)
        {
        case 0x00:
            filler = 0;
            for (size_t i = 0; i < count; i++)
                buffer[destOffset++] = filler;
            break;
        case 0x60:
            filler = 0xff;
            for (size_t i = 0; i < count; i++)
                buffer[destOffset++] = filler;
            break;
        case 0x20:
            filler = source[currOffset++];
            for (size_t i = 0; i < count; i++)
                buffer[destOffset++] = filler;
            break;
        case 0x40:
            for (size_t i = 0; i < count; i++)
                buffer[destOffset++] = source[currOffset++];
            break;
        }
    }

    return destOffset;
}

char inputfilename[256];
char outputfilename[256];
FILE* inputfile;
FILE* outputfile;
BYTE* pFileImage = NULL;
BYTE* pCartImage = NULL;
WORD wStartAddr;
WORD wStackAddr;
WORD wTopAddr;
errno_t err;

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: Sav2Cart <inputfile.SAV> <outputfile.BIN>\n");
        return 255;
    }

    strcpy_s(inputfilename, argv[1]);
    strcpy_s(outputfilename, argv[2]);

    printf("Input file: %s\n", inputfilename);

    err = fopen_s(&inputfile, inputfilename, "rb");
    if (err != 0)
    {
        printf("Failed to open the input file (%d).", err);
        return 255;
    }
    ::fseek(inputfile, 0, SEEK_END);
    uint32_t inputfileSize = ::ftell(inputfile);

    pFileImage = (BYTE*) ::malloc(inputfileSize);

    ::fseek(inputfile, 0, SEEK_SET);
    size_t bytesRead = ::fread(pFileImage, 1, inputfileSize, inputfile);
    if (bytesRead != inputfileSize)
    {
        printf("Failed to read the input file.");
        return 255;
    }
    ::fclose(inputfile);
    printf("Input file size %d. bytes.\n", inputfileSize);

    wStartAddr = *((WORD*)(pFileImage + 040));
    wStackAddr = *((WORD*)(pFileImage + 042));
    wTopAddr = *((WORD*)(pFileImage + 050));
    printf("SAV Start\t%06o  %04x  %5d\n", wStartAddr, wStartAddr, wStartAddr);
    printf("SAV Stack\t%06o  %04x  %5d\n", wStackAddr, wStackAddr, wStackAddr);
    printf("SAV Top  \t%06o  %04x  %5d\n", wTopAddr, wTopAddr, wTopAddr);
    size_t savImageSize = ((size_t)wTopAddr + 2 - 01000);
    printf("SAV Image Size\t%06o  %04x  %5d\n", savImageSize, savImageSize, savImageSize);

    pCartImage = (BYTE*) ::calloc(24576, 1);
    if (pCartImage == NULL)
    {
        printf("Failed to allocate memory.");
        return 255;
    }

    if (inputfileSize <= 24576)  // Copy SAV image as is, add loader
    {
        ::memcpy(pCartImage, pFileImage, inputfileSize);

        // Prepare the loader
        memcpy(pCartImage, loader, sizeof(loader));
        *((WORD*)(pCartImage + 0074)) = wStackAddr;
        *((WORD*)(pCartImage + 0100)) = wStartAddr;
    }
    else  // Use RLE for cartridge image
    {
        printf("Input file is too big for cartridge (%d. bytes, max 24576. bytes)\n", inputfileSize);

        size_t rleCodedSize = EncodeRLE(pFileImage + 512, savImageSize, pCartImage + 512, 24576 - 512);
        if (rleCodedSize > 24576 - 512)
        {
            printf("RLE encoded size too big (%d. bytes, max %d. bytes)\n", rleCodedSize, 24576 - 512);
            return 255;
        }

        // Trying to decode to make sure encoder works fine
        BYTE* pTempBuffer = (BYTE*) ::calloc(savImageSize, 1);
        if (pCartImage == NULL)
        {
            printf("Failed to allocate memory.");
            return 255;
        }
        size_t decodedSize = DecodeRLE(pCartImage + 512, 24576 - 512, pTempBuffer, savImageSize);
        for (size_t offset = 0; offset < savImageSize; offset++)
        {
            if (pTempBuffer[offset] == pFileImage[512 + offset])
                continue;

            printf("RLE decode failed at offset %06o (%02x != %02x)\n", 512 + offset, pTempBuffer[offset], pFileImage[512 + offset]);
            return 255;
        }
        ::free(pTempBuffer);
        printf("RLE decode check done, decoded size %d. bytes.\n", decodedSize);

        ::memcpy(pCartImage, pFileImage, 512);

        // Prepare the loader
        memcpy(pCartImage, loaderRLE, sizeof(loaderRLE));
        *((WORD*)(pCartImage + 0076)) = wStackAddr;
        *((WORD*)(pCartImage + 0102)) = wStartAddr;
    }

    // Calculate checksum
    WORD* pData = ((WORD*)(pCartImage + 01000));
    WORD wChecksum = 0;
    for (int i = 0; i < 027400; i++)
    {
        WORD src = wChecksum;
        WORD src2 = *pData;
        wChecksum += src2;
        if (((src & src2) | ((src ^ src2) & ~wChecksum)) & 0100000)  // if Carry
            wChecksum++;
        pData++;
    }
    *((WORD*)(pCartImage + 0066)) = wChecksum;

    ::free(pFileImage);

    printf("Output file: %s\n", outputfilename);
    err = fopen_s(&outputfile, outputfilename, "rb");
    if (err != 0)
    {
        printf("Failed to open output file (%d).", err);
        return 255;
    }

    size_t bytesWrite = ::fwrite(pCartImage, 1, 24576, outputfile);
    if (bytesWrite != 24576)
    {
        printf("Failed to write to the output file.");
        return 255;
    }
    ::fclose(outputfile);

    ::free(pCartImage);

    printf("Done.\n");
    return 0;
}
