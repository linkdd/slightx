#define __CPUID__
#include <cpu.h>
#include <util.h>

/* Print Registers */
void printregs (int eax, int ebx, int ecx, int edx)
{
	int i;
	char string[17] = { 0 };

	for (i = 0; i < 4; ++i)
    {
		string[i] = eax >> (8 * i);
		string[i + 4] = ebx >> (8 * i);
		string[i + 8] = ecx >> (8 * i);
		string[i + 12] = edx >> (8 * i);
	}

	printk ("%s", string);
}

/* Issue a complete request, storing general registers output as a string */
void cpuid (int code, unsigned long *eax, unsigned long *ebx, unsigned long *ecx, unsigned long *edx)
{
    asm volatile ("cpuid" : "=a" (*eax),
                            "=b" (*ebx),
                            "=c" (*ecx),
                            "=d" (*edx)
                            : "a" (code));
}

int detect_cpu (void)
{
    unsigned long eax, ebx, ecx, edx;
    cpuid (0, &eax, &ebx, &ecx, &edx);

    char proc[13] = { 0 };

    strncpy (&proc[0], (char *) &ebx, 4);
    strncpy (&proc[4], (char *) &edx, 4);
    strncpy (&proc[8], (char *) &ecx, 4);

    printk ("Processor type : %s\n", &proc[0]);

    switch (ebx)
    {
        case 0x756E6547: /* Intel Magic Code */
            do_intel();
            break;

        case 0x68747541: /* AMD Magic Code */
            do_amd();
            break;

        default:
            printk ("Unknown x86 CPU Detected\n");
            break;
    }
    return 0;
}

/* Intel-specific information */
int do_intel (void)
{
    unsigned long eax, ebx, ecx, edx, max_eax, signature, unused;
    int model, family, type, brand, stepping, reserved;
    int extended_family = -1;

    printk ("Intel Specific Features:\n");

    cpuid (1, &eax, &ebx, &unused, &unused);
    model = (eax >> 4) & 0xF;
    family = (eax >> 8) & 0xF;
    type = (eax >> 12) & 0x3;
    brand = ebx & 0xFF;
    stepping = eax & 0xF;
    reserved = eax >> 14;
    signature = eax;

    printk ("Type %d - ", type);

    switch (type)
    {
        case 0:
            printk ("Original OEM");
            break;
        case 1:
            printk ("Overdrive");
            break;
        case 2:
            printk ("Dual-capable");
            break;
        case 3:
            printk ("Reserved");
            break;
    }

    printk ("\n");
    printk ("Family %d - ", family);

    switch (family)
    {
        case 3:
            printk ("i386");
            break;
        case 4:
            printk ("i486");
            break;
        case 5:
            printk ("Pentium");
            break;
        case 6:
            printk ("Pentium Pro");
            break;
        case 15:
            printk ("Pentium 4");
            break;
    }

    printk ("\n");

    if (family == 15)
    {
        extended_family = (eax >> 20) & 0xFF;
        printk ("Extended family %d\n", extended_family);
    }

    printk ("Model %d - ", model);

    switch (family)
    {
        case 3:
            break;

        case 4:
            switch (model)
            {
                case 0:
                case 1:
                    printk ("DX");
                    break;
                case 2:
                    printk ("SX");
                    break;
                case 3:
                    printk ("487/DX2");
                    break;
                case 4:
                    printk ("SL");
                    break;
                case 5:
                    printk ("SX2");
                    break;
                case 7:
                    printk ("Write-back enhanced DX2");
                    break;
                case 8:
                    printk ("DX4");
                    break;
            }
            break;

        case 5:
            switch (model)
            {
                case 1:
                    printk ("60/66");
                    break;
                case 2:
                    printk ("75-200");
                    break;
                case 3:
                    printk ("for 486 system");
                    break;
                case 4:
                    printk ("MMX");
                    break;
            }
            break;

        case 6:
            switch (model)
            {
                case 1:
                    printk ("Pentium Pro");
                    break;
                case 3:
                    printk ("Pentium II Model 3");
                    break;
                case 5:
                    printk ("Pentium II Model 5/Xeon/Celeron");
                    break;
                case 6:
                    printk ("Celeron");
                    break;
                case 7:
                    printk ("Pentium III/Pentium III Xeon - external L2 cache");
                    break;
                case 8:
                    printk ("Pentium III/Pentium III Xeon - internal L2 cache");
                    break;
            }
            break;

        case 15:
            break;
    }

    printk ("\n");
    cpuid (0x80000000, &max_eax, &unused, &unused, &unused);

    /* Quok said: If the max extended eax value is high enough to support
     * the processor brand string (values 0x80000002 to 0x80000004), then
     * we'll use that information to return the brand information. Otherwise,
     * we'll refer back to the brand tables above for backwards compatibility
     * with older processors. According to the Sept. 2006 Intel Arch Software
     * Developer's Guide, if extended eax values are supported, then all 3
     * values for the processor brand string are supported, but we'll test
     * just to make sure and be safe.
     */

    if (max_eax >= 0x80000004)
    {
        printk ("Brand: ");

        if(max_eax >= 0x80000002)
        {
            cpuid (0x80000002, &eax, &ebx, &ecx, &edx);
            printregs (eax, ebx, ecx, edx);
        }

        if (max_eax >= 0x80000003)
        {
            cpuid (0x80000003, &eax, &ebx, &ecx, &edx);
            printregs (eax, ebx, ecx, edx);
        }

        if (max_eax >= 0x80000004)
        {
            cpuid (0x80000004, &eax, &ebx, &ecx, &edx);
            printregs (eax, ebx, ecx, edx);
        }

        printk ("\n");
    }
    else if (brand > 0)
    {
        printk ("Brand %d - ", brand);

        if (brand < 0x18)
        {
            if (signature == 0x000006B1 || signature == 0x00000F13)
            {
                printk ("%s\n", Intel_Other[brand]);
            }
            else
            {
                printk ("%s\n", Intel[brand]);
            }
        }
        else
        {
            printk ("Reserved\n");
        }
    }

    printk ("Stepping: %d Reserved: %d\n", stepping, reserved);

    return 0;
}

/* AMD-specific information */
int do_amd (void)
{
	unsigned long extended, eax, ebx, ecx, edx, unused;
	int family, model, stepping, reserved;

    printk ("AMD Specific Features:\n");
	cpuid (1, &eax, &unused, &unused, &unused);

	model = (eax >> 4) & 0xF;
	family = (eax >> 8) & 0xF;
	stepping = eax & 0xF;
	reserved = eax >> 12;

	printk ("Family: %d Model: %d [", family, model);

	switch (family)
    {
		case 4:
            printk ("486 Model %d", model);
            break;
		case 5:
            switch (model)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                case 6:
                case 7:
                    printk ("K6 Model %d", model);
                    break;
                case 8:
                    printk ("K6-2 Model 8");
                    break;
                case 9:
                    printk ("K6-III Model 9");
                    break;
                default:
                    printk ("K5/K6 Model %d", model);
                    break;
            }
            break;
		case 6:
            switch(model)
            {
                case 1:
                case 2:
                case 4:
                    printk ("Athlon Model %d", model);
                    break;
                case 3:
                    printk ("Duron Model 3");
                    break;
                case 6:
                    printk ("Athlon MP/Mobile Athlon Model 6");
                    break;
                case 7:
                    printk ("Mobile Duron Model 7");
                    break;
                default:
                    printk ("Duron/Athlon Model %d", model);
                    break;
            }
            break;
	}
	printk ("]\n");

	cpuid (0x80000000, &extended, &unused, &unused, &unused);

	if (extended == 0)
    {
		return 0;
	}

	if (extended >= 0x80000002)
    {
		unsigned long long i;
		printk ("Detected Processor Name: ");

		for(i = 0x80000002; i <= 0x80000004; ++i)
        {
			cpuid (i, &eax, &ebx, &ecx, &edx);
			printregs (eax, ebx, ecx, edx);
		}
		printk ("\n");
	}

	if (extended >= 0x80000007)
    {
		cpuid (0x80000007, &unused, &unused, &unused, &edx);

		if(edx & 1)
        {
			printk ("Temperature Sensing Diode Detected\n");
		}
	}

	printk ("Stepping: %d Reserved: %d\n", stepping, reserved);
	return 0;
}
