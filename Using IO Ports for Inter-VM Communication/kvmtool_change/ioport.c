#include "kvm/ioport.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static bool debug_io_out(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	return 0;
}

static struct ioport_operations debug_ops = {
	.io_out		= debug_io_out,
};

static bool seabios_debug_io_out(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	char ch;

	ch = ioport__read8(data);

	putchar(ch);

	return true;
}

static struct ioport_operations seabios_debug_ops = {
	.io_out		= seabios_debug_io_out,
};

static bool dummy_io_in(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	return true;
}

static bool dummy_io_out(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	return true;
}

static struct ioport_operations dummy_read_write_ioport_ops = {
	.io_in		= dummy_io_in,
	.io_out		= dummy_io_out,
};

static struct ioport_operations dummy_write_only_ioport_ops = {
	.io_out		= dummy_io_out,
};

/*
 * The "fast A20 gate"
 */

static bool ps2_control_a_io_in(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	/*
	 * A20 is always enabled.
	 */
	
	int success = system("cat /proc/bridge_comm_VM > out.txt");
	printf("success = %d\n",success);
	FILE *fd=fopen("/proc/bridge_comm_VM","r");
	//char recv[1];
	
	success = fgetc(fd); 
	//const char *temp_ch = recv;
	ioport__write8(data, (success-65));
	
	//u8 ch;
	//value=atoi(buf);
	printf("input = %d\n",success);
	fclose(fd);

	return true;
}

/* added */
static bool ps2_control_a_io_out(struct ioport *ioport, struct kvm_cpu *vcpu, u16 port, void *data, int size)
{
	u8 ch;
	ch = ioport__read8(data);
	
	//pChars[0];//one byte is here
	//pChars[1];//another byte here
	
	//char* str1;
	//str1 =  data;
	
	char temp_ch = 65 + ch;
	
	//char *ch_ptr = &temp_ch;
	
	fprintf(stderr, "The output of port 0x0092 : %u \n", ch);
	fprintf(stderr, "The output of port 0x0092 : %c \n", temp_ch);
	//fprintf(stderr, "The output of port 0x0092 : %u \n", *ch_ptr);	
	
	
	//putchar(ch);
	char command[80];		
	//char *val = itoa(ch);
	strcpy(command,"echo ");
	command[5] = temp_ch;
	//sprintf(ch_ptr, "%d\n" , ch);
	//strcat(command, str1);
	strcat(command," > /proc/bridge_comm_VM");
	int success = system(command);
	printf("input = %d\n",success);
	printf("command = %s\n",command);
	//FILE *fd = fopen("/proc/bridge_comm_VM","a");
	//fputs(ch_ptr,fd); 
	//value=atoi(buf);
	//printf("\n input = %d\n",value);
	//fclose(fd);

	return true;	
}

/*modified*/
static struct ioport_operations ps2_control_a_ops = {
	.io_in		= ps2_control_a_io_in,
	.io_out		= ps2_control_a_io_out, //dummy_io_out, 
};

void ioport__map_irq(u8 *irq)
{
}

void ioport__setup_arch(struct kvm *kvm)
{
	/* Legacy ioport setup */

	/* 0000 - 001F - DMA1 controller */
	ioport__register(kvm, 0x0000, &dummy_read_write_ioport_ops, 32, NULL);

	/* 0x0020 - 0x003F - 8259A PIC 1 */
	ioport__register(kvm, 0x0020, &dummy_read_write_ioport_ops, 2, NULL);

	/* PORT 0040-005F - PIT - PROGRAMMABLE INTERVAL TIMER (8253, 8254) */
	ioport__register(kvm, 0x0040, &dummy_read_write_ioport_ops, 4, NULL);

	/* 0092 - PS/2 system control port A */
	/*modified this operation handler to handle our PORT mapped I/O implementation */
	ioport__register(kvm, 0x0092, &ps2_control_a_ops, 2, NULL); 

	/* 0x00A0 - 0x00AF - 8259A PIC 2 */
	ioport__register(kvm, 0x00A0, &dummy_read_write_ioport_ops, 2, NULL);

	/* 00C0 - 001F - DMA2 controller */
	ioport__register(kvm, 0x00C0, &dummy_read_write_ioport_ops, 32, NULL);

	/* PORT 00E0-00EF are 'motherboard specific' so we use them for our
	   internal debugging purposes.  */
	ioport__register(kvm, IOPORT_DBG, &debug_ops, 1, NULL);

	/* PORT 00ED - DUMMY PORT FOR DELAY??? */
	ioport__register(kvm, 0x00ED, &dummy_write_only_ioport_ops, 1, NULL);

	/* 0x00F0 - 0x00FF - Math co-processor */
	ioport__register(kvm, 0x00F0, &dummy_write_only_ioport_ops, 2, NULL);

	/* PORT 0278-027A - PARALLEL PRINTER PORT (usually LPT1, sometimes LPT2) */
	ioport__register(kvm, 0x0278, &dummy_read_write_ioport_ops, 3, NULL);

	/* PORT 0378-037A - PARALLEL PRINTER PORT (usually LPT2, sometimes LPT3) */
	ioport__register(kvm, 0x0378, &dummy_read_write_ioport_ops, 3, NULL);

	/* PORT 03D4-03D5 - COLOR VIDEO - CRT CONTROL REGISTERS */
	ioport__register(kvm, 0x03D4, &dummy_read_write_ioport_ops, 1, NULL);
	ioport__register(kvm, 0x03D5, &dummy_write_only_ioport_ops, 1, NULL);

	ioport__register(kvm, 0x402, &seabios_debug_ops, 1, NULL);

	/* 0510 - QEMU BIOS configuration register */
	ioport__register(kvm, 0x510, &dummy_read_write_ioport_ops, 2, NULL);
}
