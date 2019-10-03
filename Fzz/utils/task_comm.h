#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/prctl.h>

#define TASK_COMM_LEN 16

int set_task_comm() {
  char comm[TASK_COMM_LEN];
  int ret;
  sprintf(comm, "%s", "exec-ready");
  printf("Generated comm: '%s'\n", comm);
  ret = prctl(PR_SET_NAME, comm);
  if (ret < 0) {
    perror("prctl");	
		exit(-1);
	}
	return 0;
}

int clear_task_comm() {
	char comm[TASK_COMM_LEN];
	int ret;
	sprintf(comm, "%s", "exec-return");
	printf("Clear comm: '%s'\n", comm);
	ret = prctl(PR_SET_NAME, comm);
	if (ret < 0) {
		perror("prctl");
		exit(-1);
	}
	return 0;
}
