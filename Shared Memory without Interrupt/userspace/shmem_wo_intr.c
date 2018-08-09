/***************************************************************************
 *  o Derived from "http:://nairobi-embedded.org/mmap_n_devmem.html".  *
 ****************************************************************************/
 
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/types.h>
#include <sys/io.h>

#include "shmem_wo_intr_wrapper.h"

#define PORT 0x0092 /* PS2 port (specific to x86) used for data synchronization */

static const char *prog;
static struct upci_dev_info info;

struct dev_params {
	__s32 num;
	int region;
	__u32 offset;
	union {
	__u8  u8val;
	__u16 u16val;
	__u32 u32val;
	};
};

static enum {
	PCI_READ = 0,
	PCI_WRITE
} pci_op;

/* r/w data formats (sizes and signedness) */
#define FMT_u8  1
#define FMT_u16 2
#define FMT_u32 3
#define FMTSTR(x) (#x + 4)
 
static char *format[] =
    { FMTSTR(FMT_s8), FMTSTR(FMT_u8), FMTSTR(FMT_s16), FMTSTR(FMT_u16), 
		  FMTSTR(FMT_s32), FMTSTR(FMT_u32), NULL };

static inline void do_pci_write(int data_region, struct dev_params *dev,
				int fmt)
{
	switch (fmt) {
	case FMT_u8:
		upci_write_u8(data_region, dev->offset, dev->u8val);
		break;
	case FMT_u16:
		upci_write_u16(data_region, dev->offset, dev->u16val);
		break;
	case FMT_u32:
	default:
		upci_write_u32(data_region, dev->offset, dev->u32val);
		break;
	}
}

static inline void do_pci_read(int data_region, struct dev_params *dev, int fmt)
{
	switch (fmt) {
	case FMT_u8:
		dev->u8val = upci_read_u8(data_region, dev->offset);
		break;
	case FMT_u16:
		dev->u16val = upci_read_u16(data_region, dev->offset);
		break;
	case FMT_u32:
	default:
		dev->u32val = upci_read_u32(data_region, dev->offset);
		break;
	}
}

static void usage(void);
static void bad_option(const char *option)
{
	fprintf(stderr, "\n\t!! Bad \"%s\" option !!\n", option);
	usage();
}

static void usage(void)
{
	int i;
	fprintf(stderr,
		"\nusage: %s [OPTS] <ARGS>\n\n"
		"OPTS:\n"
		"-h, --help         This menu\n"
		"-a, --action       0 (PCI read - default) / 1 (PCI write)\n"
		"-w, --value        Value for PCI write operation\n"
		"-f, --format       Data format: ", prog);

	for (i = 0; format[i+1]; i++)
		fprintf(stderr, "%s, ", format[i]);
	fprintf(stderr, "or %s (default)", format[i]);
	fprintf(stderr, "\n\n");

	fprintf(stderr,
		"ARGS (all required):\n"
		"-V, --vendor       VendorID\n"
		"-D, --device       DeviceID\n"
		"-v, --subvendor    SubVendorID\n"
		"-d, --subdevice    SubDeviceID\n"
		"-i, --instance     Card Instance (typically, 0)\n"
		"-r, --region       PCI region to r/w\n"
		"-o, --offset       Offset into PCI region\n\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	struct option long_option[] = {
		{"help", 0, NULL, 'h'},
		{"device", 1, NULL, 'D'},
		{"subdevice", 1, NULL, 'd'},
		{"vendor", 1, NULL, 'V'},
		{"subvendor", 1, NULL, 'v'},
		{"instance", 1, NULL, 'i'},
		{"region", 1, NULL, 'r'},
		{"offset", 1, NULL, 'o'},
		{"action", 1, NULL, 'a'},
		{"value", 1, NULL, 'w'},
		{"format", 1, NULL, 'f'},
		{NULL, 0, NULL, 0},
	};
	int i, retval, data_region, fmt = FMT_u32;
	char *eptr;
	__u16 vendor_id = 0, device_id = 0, ss_vendor_id = 0,
	    ss_device_id = 0, instance = 0;
	struct dev_params dev;
	memset(&dev, 0, sizeof(struct dev_params));

	prog = argv[0];
	/* if we are setuid, drop privs until needed */
	retval = seteuid(getuid());
	if (retval != 0) {
		printf("failed to set euid to uid %d: %s\n", getuid(),
		      strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (1) {
		int c;
		if ((c =
		     getopt_long(argc, argv, "hV:D:v:d:i:r:o:a:w:f:",
				 long_option, NULL)) < 0)
			break;

		switch (c) {

		case 'V':
			vendor_id = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("VendorID");
			break;

		case 'D':
			device_id = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("DeviceID");
			break;

		case 'v':
			ss_vendor_id = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("SubVendorID");
			break;

		case 'd':
			ss_device_id = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("SubDeviceID");
			break;

		case 'i':
			instance = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("Instance number");
			break;

		case 'r':
			dev.region = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("PCI Region");
			break;

		case 'o':
			dev.offset = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("PCI Region Offset");
			break;

		case 'a':
			pci_op = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("PCI Operation");
			if ((pci_op != PCI_READ) && (pci_op != PCI_WRITE)) {
				printf("Assuming PCI_READ operation ...\n");
				pci_op = PCI_READ;
			}
			break;

		case 'w':
			dev.u32val = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("Bad PCI write value\n");
			break;

		case 'f':

			for (i = 0; format[i]; i++) {
				if (!strncmp
				    (optarg, format[i], strlen(format[i])))
					break;
			}

			if (format[i] == NULL)
				bad_option("Data Format");
			else
				fmt = i;
			break;

		case 'h':
		default:
			usage();
			break;
		}		
	}			

	
	retval = upci_scan_bus();
	if (retval < 0) {
		printf("PCI bus data missing");
		exit(EXIT_FAILURE);
	}

	
	info.vendor_id = vendor_id;
	info.device_id = device_id;
	info.ss_vendor_id = ss_vendor_id;
	info.ss_device_id = ss_device_id;
	info.instance = instance;
	dev.num = upci_find_device(&info);
	if (dev.num < 0) {
		printf("DeviceID %d, VendorID %d: "
		      "subDeviceID %d, SubVendorID %d: "
		      "Instance %d not found. Quiting...\n",
		      device_id, vendor_id, ss_device_id, ss_vendor_id,
		      instance);
		exit(EXIT_FAILURE);
	}

	if(ioperm(PORT,3,1))	
	{
		//perror("permission could not be set");
		exit(1);
	}

	/* perform pci operation */
	data_region = upci_open_region(dev.num, dev.region);
	switch (pci_op) {
	case PCI_READ:
		{	
			int count = 0;
			for(;;)
			{ 
				dev.offset = count;
				count ++;
				do_pci_read(data_region, &dev, fmt);			
				//printf("Received value in the port 0x0092 : %d \n", (inb(PORT)));
				inb(PORT);
				printf("Read value from the shared memory : %u \n", dev.u8val);				
				if(dev.u8val == 0)
				{
					printf("Read is outpacing write! wait for write to catch up \n");				
					count--;
				}	
				if (count == 32)
					break;//count = 0;
					
				sleep(5);	
			}
			break;
		}

	case PCI_WRITE:
		{	
			int count = 0;
			for(;;)
			{
				dev.offset = count;
				count ++;
				dev.u8val = count;				
				//printf("Ready to send value in the port 0x0092: %d \n", (count));								
				outb((count),PORT);
				do_pci_write(data_region, &dev, fmt);
				printf("Wrote value in the shared memory: %u \n", dev.u8val);
				if (count == 32)
					break;//count = 0;
			}
			break;
		}
	}
	
	ioperm(PORT,3,0);	
	
	exit(EXIT_SUCCESS);
}
