#include <sys/io.h>

void init_pic (void)
{
	/* Master */
	outb (0x20, 0x11); /* ICW 1 */
	iowait ();
	outb (0x21, 0x20); /* ICW 2 */
	iowait ();
	outb (0x21, 0x04); /* ICW 3 */
	iowait ();
	outb (0x21, 0x01); /* ICW 4 */
	iowait ();
	outb (0x21, 0xFF); /* Mask interrupts */
	iowait ();

	/* Slave */
	outb (0xA0, 0x11); /* ICW 1 */
	iowait ();
	outb (0xA1, 0x70); /* ICW 2 */
	iowait ();
	outb (0xA1, 0x02); /* ICW 3 */
	iowait ();
	outb (0xA1, 0x01); /* ICW 4 */
	iowait ();
	outb (0xA1, 0xFF); /* Mask interrupts */
	iowait ();


}
