#if !defined __SYSCALL___H_
#define __SYSCALL___H_


int chprio(int pid, int newprio);
void cons_write(const char *str, unsigned long size);

int cons_read(char *string, unsigned long length);
void cons_echo(int on);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
int pcount(int fid, int *count);
int pcreate(int count);
int pdelete(int fid);
int preceive(int fid,int *message);
int preset(int fid);
int psend(int fid, int message);

void *shm_create(const char*);
void *shm_acquire(const char*);
void shm_release(const char*);

void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);

int start(const char *process_name, unsigned long ssize, int prio, void *arg);
int waitpid(int pid, int *retval);
void sys_info(char *output);

#endif 

