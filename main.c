#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include<emscripten/emscripten.h>
#include <emscripten.h>
#include "includes/cpu.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include "test.h"
EMSCRIPTEN_KEEPALIVE
void tesqwe(char *input2);
// ANSI colors
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

void read_file(CPU* cpu, char *filename)
{
	FILE *file;
	uint8_t *buffer;
	unsigned long fileLen;

	//Open file
	file = fopen(filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", filename);
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(uint8_t *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(file);
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

    // Print file contents in hex
    for (int i=0; i<fileLen; i+=2) {
        if (i%16==0) printf("\n%.8x: ", i);
        printf("%02x%02x ", *(buffer+i), *(buffer+i+1));
    }
    /*printf("\n");*/

    // copy the bin executable to dram
    memcpy(cpu->bus.dram.mem, buffer, fileLen*sizeof(uint8_t));
	free(buffer);
}
int EMSCRIPTEN_KEEPALIVE sum(){

printf("sum = %i\n", 100);

return 1;
}

int EMSCRIPTEN_KEEPALIVE sum2(int a,int b){

printf("sum2 = %i\n",a+b);

return 0;
}


//-----------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE 
char *LoadData() {
   int fd;
   int size;
   char *buff;

   fd = open("/data/textfile.txt", O_RDONLY);

   if (fd == -1) return strerror(errno);

   size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   buff = (uint8_t *) malloc(size+1);
   read(fd, buff, size);
   buff[size] = '\0';
       for (int i=0; i<size; i+=2) {
        if (i%16==0) printf("\n%.8x: ", i);
        printf("%02x%02x ", *(buff+i), *(buff+i+1));
    }
   close(fd); 
   printf("read %s bytes...\n",buff);
   struct CPU cpu;
    cpu_init(&cpu);
    // Read input file
    read_file(&cpu,"/data/textfile2.txt");
    while (1) {
        // fetch
        uint32_t inst = cpu_fetch(&cpu);
        // Increment the program counter
        cpu.pc += 4;
        // execute
        if (!cpu_execute(&cpu, inst))
            break;

        // dump_registers(&cpu);

        if(cpu.pc==0)
            break;
    }
   return buff;
}

//-----------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE
void SaveData(char *data) {
   int fd;
   int size;

   if (data == NULL) return;

   fd = open("/data/textfile.txt", O_CREAT | O_WRONLY, 0666);
   if (fd == -1) {
       printf("ERROR: could not open the file for writing!\n, %s\n", strerror(errno));
       return;
   }
   size = strlen(data);
   printf("saving %i bytes... %s\n", size, data);
   write(fd, data, size);
   ftruncate(fd, size);
   tesqwe(data);
   close(fd);
   EM_ASM ( FS.syncfs(false, function (err) {} ); );
}

static char g_str[256] = "Hello, world!";
void EMSCRIPTEN_KEEPALIVE queryString(char** ppStr) {
   *ppStr = g_str;
}

void printArry(unsigned char arr[], int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        printf("%x ", arr[i]);
    }

    printf("\n");
}
void float2u8Arry(uint8_t *u8Arry, float *floatdata)
{
    uint8_t farray[4];
    *(float *)farray = *floatdata;
 
      u8Arry[3] = farray[0];
        u8Arry[2] = farray[1];
        u8Arry[1] = farray[2];
        u8Arry[0] = farray[3];
 
}
float EMSCRIPTEN_KEEPALIVE sum_up(float vals[], int size) {
  float res = 0;
  for(int i=0; i<size; i++){
    res += vals[i];

    printf("%f \n",vals[i]);
    uint8_t u8data[4];
    float2u8Arry(u8data, &vals[i]);
    printArry(u8data, 4);
    //    printf("saving %d bytes... %s\n",  vals[i]);
    }
 
    // write_to_file()
  return res;
}

//-----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     printf("Usage: rvemu <filename>\n");
    //     exit(1);
    // }
 EM_ASM(
      FS.mkdir('/data');
      FS.mount(IDBFS, {}, '/data');

      FS.syncfs(true, function (err) { 
         // provide the DOM side a way to execute code after directory is mounted
         if (typeof Module.OnDataMounted !== 'undefined') {
            Module.OnDataMounted();
         }
      } );
   );

    // Initialize cpu, registers and program counter
    struct CPU cpu;
    cpu_init(&cpu);
    // Read input file
    read_file(&cpu,"addi.bin");
    while (1) {
        // fetch
        uint32_t inst = cpu_fetch(&cpu);
        // Increment the program counter
        cpu.pc += 4;
        // execute
        if (!cpu_execute(&cpu, inst))
            break;

        // dump_registers(&cpu);

        if(cpu.pc==0)
            break;
    }
    
    emscripten_exit_with_live_runtime();
    return 0;
}
