// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"
#include <sys/stat.h>

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
    "ttyACM",   // linux
	"cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    "cu.SLAB_USBtoUART",
    //"tty.SLAB_USBtoUART",
    //"cu.usbserial-1410",

    //"tty.usbserial-1410", // my laptop??
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    for (int i = 0; ttyusb_prefixes[i]; i++) {
        //printf("prefix: %s\n", ttyusb_prefixes[i]);
        if (strcmp(d->d_name, ttyusb_prefixes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    char *dirp = "/dev/";
    struct dirent **namelist;
    int num = scandir(dirp, &namelist, filter, alphasort);
    if (num != 1) {
        panic("found %d tty devices!\n", num);
    }
    char *device = namelist[0]->d_name;
    char *full_path = (char *)malloc(strlen(dirp) + strlen(device) + 1);
    strncpy(full_path, dirp, strlen(dirp));
    strncat(full_path, device, strlen(device));
    return full_path;
}


// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) { 
    char *dirp = "/dev/";
    struct dirent **namelist;
    int num = scandir(dirp, &namelist, filter, alphasort);
    if (num < 0) {
        perror("scandir failed\n");
    }
    if (num == 0) {
        panic("found %d tty devices!\n", num);
    }
    char *device = namelist[0]->d_name;
    struct stat st;
    stat(device, &st);
    time_t most_recent = st.st_mtime;
    for (int i = 0; i < num; i++) {
        stat(namelist[i]->d_name, &st);
        time_t curr = st.st_mtime;
        if (curr > most_recent) {
            device = namelist[i]->d_name;
            most_recent = curr;
        }
    }
    char *full_path = (char *)malloc(strlen(dirp) + strlen(device) + 1);
    strncpy(full_path, dirp, strlen(dirp));
    strncat(full_path, device, strlen(device));
    return full_path;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) { 
    char *dirp = "/dev/";
    struct dirent **namelist;
    int num = scandir(dirp, &namelist, filter, alphasort);
    if (num < 0) {
        perror("scandir failed\n");
    }
    if (num == 0) {
        panic("found %d tty devices!\n", num);
    }
    char *device = namelist[0]->d_name;
    struct stat st;
    stat(device, &st);
    time_t oldest = st.st_mtime;
    for (int i = 0; i < num; i++) {
        stat(namelist[i]->d_name, &st);
        time_t curr = st.st_mtime;
        if (curr < oldest) {
            device = namelist[i]->d_name;
            oldest = curr;
        }
    }
    char *full_path = (char *)malloc(strlen(dirp) + strlen(device) + 1);
    strncpy(full_path, dirp, strlen(dirp));
    strncat(full_path, device, strlen(device));
    return full_path;
}
// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
/*static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    unimplemented();
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    unimplemented();
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    unimplemented();
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    unimplemented();
}*/
