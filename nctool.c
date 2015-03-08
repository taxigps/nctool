#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct st_head {
  unsigned int Version;
  unsigned int UpdateID;
  unsigned short Year;
  unsigned char Day;
  unsigned char Month;
  unsigned int CPUID;
  unsigned int Checksum;
  unsigned int LoadVers;
  unsigned int Platform;
  unsigned int Size;
  unsigned int Start;
  unsigned int End;
} head_msgs[50];

int head_no;

void Usage(void) {
  puts("NCTOOL.EXE V1.00 [2013/10/23] (C)taxigps 2013");
  puts("Syntax:");
  puts("     NCTOOL.EXE InputFile [-optons]");
  puts("          InputFile   : NCPUCODE.BIN file to be modified");
  puts("          Default     : Display all microcode messages in BIN file");
  puts("          -d num      : Delete microcode at position number num");
}

int getMsg(char* filepath) {
	FILE *in;
	unsigned int pos = 0;
  unsigned int d;
  unsigned int numread;

  if ((in = fopen(filepath, "rb")) == NULL) {
    printf("Error: Can't open NCPUCODE.BIN file (%s).\n\n", filepath);
    Usage();
    return 0;
  }
  head_no = 0;
  numread = fread(&head_msgs[head_no], sizeof(char), 32, in);
  while (numread == 32 && head_no <=50) {
    //Version, UpdateID, Year, Day, Month, CPUID, Checksum, LoadVers, Platform, Size = struct.unpack('IIHBBIIIII', data)
    head_msgs[head_no].Start = pos;
    if (head_msgs[head_no].Size == 0)
      d = 0x800;
    else
      d = head_msgs[head_no].Size + 0x30;
    pos += d;
    head_msgs[head_no].End = pos;
    head_no++;
    fseek(in, pos, SEEK_SET);
    numread = fread(&head_msgs[head_no], sizeof(char), 32, in);
  }
  fclose(in);
  return 1;
}

void showMsg(void) {
  int i;
  puts("NO Version  UpdateID Date       CPUID    Checksum LoadVers Platform Size");
  for (i = 0; i < head_no; i++) {
    unsigned int s = (head_msgs[i].End - head_msgs[i].Start) / 0x400;
    printf("%2d %08X %08X %04X.%02X.%02X %08X %08X %08X %08X %dk\n", \
      i+1, head_msgs[i].Version, head_msgs[i].UpdateID, head_msgs[i].Year, head_msgs[i].Month, head_msgs[i].Day,\
      head_msgs[i].CPUID, head_msgs[i].Checksum, head_msgs[i].LoadVers, head_msgs[i].Platform, s);
  }
}

int main(int argc, char* argv[]) {

	if (argc > 1) {
    if (getMsg(argv[1])) {
      if (argc > 2) {
        if (strcmp(argv[2], "-d") == 0)
        {
          if (argc > 3) {
            int num;
            num = atoi(argv[3]);
            if (num > 0 && num <= head_no) {
            	FILE *fin, *fout;
              int bufsize = head_msgs[0].End - head_msgs[0].Start;
              void * buf = malloc(bufsize);
              int i;
              int n = strlen(argv[1]) + 5;
              char *tempfile = (char *)malloc(n*sizeof(char));
              strcpy(tempfile, argv[1]);
              strcat(tempfile, ".tmp");
              fin = fopen(argv[1], "rb");
              fout = fopen(tempfile, "wb");
              for (i=0; i<num-1;i++) {
                int size = head_msgs[i].End - head_msgs[i].Start;
                if (size > bufsize) {
                  realloc(buf, size);
                  bufsize = size;
                }
                fread(buf, sizeof(char), size, fin);
                fwrite(buf, sizeof(char), size, fout);
              }
              fseek(fin, head_msgs[num].Start, SEEK_SET);
              for (i=num; i<head_no;i++) {
                int size = head_msgs[i].End - head_msgs[i].Start;
                if (size > bufsize) {
                  realloc(buf, size);
                  bufsize = size;
                }
                fread(buf, sizeof(char), size, fin);
                fwrite(buf, sizeof(char), size, fout);
              }
              free(buf);
              fclose(fin);
              fclose(fout);
              if (remove(argv[1])) {
                perror(argv[1]);
                remove(tempfile);
              }
              else {
                rename(tempfile, argv[1]);
                printf("Succeed: Delete microcode No%d(CPUID:%08X Platform:%08X Date:%04X.%02X.%02X)\n", \
                  num, head_msgs[num-1].CPUID, head_msgs[num-1].Platform,\
                  head_msgs[num-1].Year, head_msgs[num-1].Month, head_msgs[num-1].Day);
              }
              free(tempfile);
            }
            else {
              puts("Error: Wrong num! Please check the NO row in list.\n");
              showMsg();
            }
          }
          else
            Usage();
        }
      }
      else
        showMsg();
    }
	}
  else
    Usage();
  return 0;
}
