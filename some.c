#include <stdbool.h>
#include <blkid/blkid.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "bcache.h"
#include <uuid/uuid.h>
#include <string.h>

int list_bdevs(char devs[][50]){
//	char devs[10][20]; //TODO:prevent memory leak,for now you show set the size large enough.
	printf("hello\n");
	int i = 0;
	DIR *dir;
	struct dirent *ptr;
        blkid_probe pr;
        struct cache_sb sb;
	char uuid[40];
	char set_uuid[40];
	dir = opendir("/sys/block");
	while((ptr = readdir(dir))!=NULL){
	    if (strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
		continue;
	    }
            char dev[20];
    	    sprintf(dev,"/dev/%s",ptr->d_name);
		printf("%s\n",dev);
//	    char dev[20] = "/dev/";
//		strcat(dev,ptr->d_name);
	        int fd = open(dev, O_RDONLY);
                if (fd == -1)
                        continue;

                if (!(pr = blkid_new_probe()))
                        continue;
                if (blkid_probe_set_device(pr, fd, 0, 0))
                        continue;
                if (blkid_probe_enable_partitions(pr, true))
                        continue;
                if (!blkid_do_probe(pr)) {
                        continue;
                }

                if (pread(fd, &sb, sizeof(sb), SB_START) != sizeof(sb))
                        continue;

                if (memcmp(sb.magic, bcache_magic, 16))
                        continue;

                uuid_unparse(sb.uuid, uuid);
                uuid_unparse(sb.set_uuid, set_uuid);
                printf(" UUID=%s DEV=%s  SET_UUID=%s\n", uuid,dev,set_uuid);
		printf("i is %d",i);
		strcpy(devs[i],dev);
	//	devs[i]=dev;
		i++;
//		printf("sb.csum:%s\n",uuid);
//		printf(dev,sb.csum);

	}
	strcpy(devs[i],"\0");
	closedir(dir);
	return 0;
}

int registe(char *devname){
   // char *devname = "/dev/sdi";
    int fd;
    fd = open("/sys/fs/bcache/register", O_WRONLY);
    if (fd < 0)
    {
        perror("Error opening /sys/fs/bcache/register");
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%s\n", devname) < 0)
    {
        fprintf(stderr, "Error registering %s with bcache: %m\n", devname);
        return 1;
    }
    close(fd);
    return 0;
}

int unregiste_cset(char *cset){
//    char *devname = "/dev/sdi";
    int fd;
    char path[100];
    sprintf(path,"/sys/fs/bcache/%s/unregister",cset);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%s\n", cset) < 0)
    {
        fprintf(stderr, "Error registering %s with bcache: %m\n", cset);
        return 1;
    }

    return 0;
}

int stop_backdev(char *devname){
    char path[100];
    int fd;
    sprintf(path,"/sys/block/%s/bcache/stop",devname);
    fd = open(path,O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%s\n", "1") < 0)
    {
    //    fprintf(stderr, "Error stop back device %s\n", devname);
        return 1;
    }
    return 0;
}

int unregiste_both(char *cset){
    int fd;
    char path[100];
    sprintf(path,"/sys/fs/bcache/%s/stop",cset);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%d\n", 1) < 0)
    {
        return 1;
    }
    return 0;
}

int attach(char *cset,char *devname){
    int fd;
    char path[100];
    sprintf(path,"/sys/block/%s/bcache/attach",devname);
    fd = open(path,O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%s\n", cset) < 0)
    {
        return 1;
    }

    return 0;
}

int detach(char *devname){
    int fd;
    char path[100];
    sprintf(path,"/sys/block/%s/bcache/detach",devname);
    fd = open(path,O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%d\n", 1) < 0)
    {
        return 1;
    }
    return 0;
}

int set_backdev_cachemode(char *devname,char *cachemode){
    int fd;
    char path[100];
    sprintf(path,"/sys/block/%s/bcache/cache_mode",devname);
    fd = open(path,O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
        return 1;
    }
    if (dprintf(fd, "%s\n", cachemode) < 0)
    {
        return 1;
    }
    return 0;
}

int get_backdev_cachemode(char *devname,char *mode){
    int fd;
    char path[100];
    sprintf(path,"/sys/block/%s/bcache/cache_mode",devname);
    fd = open(path,O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening /sys/fs/bcache/register");
        fprintf(stderr, "The bcache kernel module must be loaded\n");
	return 1;
    }
    printf("size in func is %d",sizeof(mode));
    if (read(fd,mode,100) < 0)
    {
        fprintf(stderr, "failed to fetch device cache mode\n");
	return 1;
    }
    
    return 0;
}

int get_backdev_state(char *devname,char *state){
    int fd;
    char path[100];
    sprintf(path,"/sys/block/%s/bcache/state",devname);
    fd = open(path,O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "The bcache kernel module must be loaded\n");
	return 1;
    }
    if (read(fd,state,20) < 0)
    {
        fprintf(stderr, "failed to fetch device state\n");
	return 1;
    }
    return 0;
}

int main(){
//	list_bcacheset();
	//unregiste_cset("112edd51-a548-4945-b0eb-3df948e90fda");
//	stop_backdev("sdh");
//	unregiste_both("112edd51-a548-4945-b0eb-3df948e90fda");
//	detach("sdh");
	//attach("112edd51-a548-4945-b0eb-3df948e90fda","sdh");
	//int rt;
	//char mode[42];
	//rt = get_backdev_state("sdh",mode);

/*listdevstest
	char a[500][50];	
	list_bdevs(a);

 
 	printf("\n%d\n",sizeof(a));
	int i;
	for (i=0;i<5;i++){
	int r;
	r = strcmp(a[i],"\0");
	printf("r is %d",r);
	if  (strcmp(a[i],"\0")==0){
		printf("empty in %d",i);
		break;
	}
	printf("%d %s \n",i,a[i]);
}
*/
}
