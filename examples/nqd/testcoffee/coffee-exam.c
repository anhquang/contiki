 #include "cfs/cfs.h" 
 #include "contiki.h"
 #include <stdio.h>
 /**/
 PROCESS(coffee_test_process, "Coffee test process");
 AUTOSTART_PROCESSES(&coffee_test_process);
 PROCESS_THREAD(coffee_test_process, ev, data)
 /**/
 {
  PROCESS_BEGIN();
  /*        */
  /* step 1 */
  /*        */
  char message[32];
  char buf[100];
  char *filename = "msg_file";
  char *file2 = "file_2";
  struct cfs_dir dir;
  struct cfs_dirent dirent;

  int fd_write, fd_read;
  int n;
  int i;

  struct abc{
	  char a;
	  char b;
  };
  struct abc a;

  a.a = 39;
  a.b = 40;

    strcpy(message,"#1.hello world.");

    fd_write = cfs_open(filename, CFS_WRITE);
    if(fd_write != -1) {
      n = cfs_write(fd_write, message, sizeof(message));

      cfs_close(fd_write);
      printf("step 2: successfully written to cfs. wrote %i bytes\n", n);
    } else {
      printf("ERROR: could not write to memory in step 2.\n");
    }

    fd_write = cfs_open(file2, CFS_WRITE);
    if(fd_write != -1) {
      n = cfs_write(fd_write, &a, sizeof(a));
      cfs_close(fd_write);
      printf("step 3: successfully written to cfs. wrote %i bytes\n", n);
    } else {
      printf("ERROR: could not write to memory in step 3.\n");
    }

    //read back from file2
    fd_read = cfs_open(file2, CFS_READ);
	  if(fd_read != -1) {
		cfs_read(fd_read, buf, sizeof(a));
		printf("value read back: ");
		for (i=0; i<sizeof(a); i++)
			printf("%d.",buf[i]);
		printf("\n");
		cfs_close(fd_read);
	  } else {
		printf("ERROR: could not read from memory in step 5.\n");
	  }



    while(cfs_readdir(&dir, &dirent) != -1) {
      printf("File: %s (%ld bytes)\n",
             dirent.name, (long)dirent.size);
    }
    cfs_closedir(&dir);



  /**/
  PROCESS_END();
 }
