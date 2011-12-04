/*
 * boote.c
 *
 *  Created on: Dec 4, 2011
 *      Author: laszlo
 */
#include <common.h>
#include <command.h>
#include <image.h>

int do_bootm_philips_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	void	(*theKernel) (int, char **, char **, int *);
	char	*commandline = getenv ("bootargs");

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* find kernel entry point */
	theKernel = (void (*)(int, char **, char **, int *))images->ep;

	show_boot_progress (15);

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);
#endif

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");

	theKernel (1, &commandline, NULL, 0);
	/* does not return */
	return 1;
}
