#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>



// The page frame shifted left by PAGE_SHIFT will give us the physcial address of the frame
// Note that this number is architecture dependent. For me on x86_64 with 4096 page sizes,
// it is defined as 12. If you're running something different, check the kernel source
// for what it is defined as.
#define PAGE_SHIFT 12
#define PAGEMAP_LENGTH 8


unsigned long get_page_frame_number_of_address(void *addr) {
   // Open the pagemap file for the current process
   FILE *pagemap = fopen("/proc/self/pagemap", "rb");
   if (pagemap == NULL) 
   {
      printf("read file error\n");
      exit(1);
   }

   // Seek to the page that the buffer is on
   unsigned long offset = (unsigned long)addr / getpagesize() * PAGEMAP_LENGTH;

   if(fseek(pagemap, (unsigned long)offset, SEEK_SET) != 0) {
      fprintf(stderr, "Failed to seek pagemap to proper location\n");
      exit(1);
   }

   // The page frame number is in bits 0-54 so read the first 7 bytes and clear the 55th bit
   unsigned long page_frame_number = 0;
   fread(&page_frame_number, 1, PAGEMAP_LENGTH-1, pagemap);

   page_frame_number &= 0x7FFFFFFFFFFFFF;

   fclose(pagemap);

   return page_frame_number;
}

void main() {
  
   char *line;
   char a[10];
   size_t len = 0;
   FILE *fp = fopen("/proc/self/maps", "rb");
   if (fp == NULL) 
   {
      printf("read file error\n");
      exit(1);
   }
   while(!feof(fp))
   {
      int read = getline(&line, &len, fp);
      if (read == -1)
         break;
      printf("%s", line);
      char *ptr = strtok(line, " ");
      
      ptr = strtok(line, "-");
      unsigned long start = strtoul(ptr, NULL, 16);
      ptr = strtok(NULL, "-");
      unsigned long end = strtoul(ptr, NULL, 16);
      
      for(unsigned long addr = start; addr < end; addr += 0x1000) {
         unsigned long frame_num = get_page_frame_number_of_address((void*)addr);
 	 
 	 if (frame_num != 0)
            for (int i=0; i<4096;i++) {
               //if(addr + i - (unsigned long)a < 10)
               printf("%02x", a[addr + i - (unsigned long)a]);
            }	
         printf("\n");								
      }
   }
   fclose(fp);
}







