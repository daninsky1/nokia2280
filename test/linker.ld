ENTRY(_start)       /* Program entry pont */

/* Memory layout */

SECTIONS
{
    /* Start program at address 0 */
    . = 0x00000000;

    /* Executable code */
    .text :
    {
        *(.text*)
    }

    . = 0x80000000;
    /* data */
    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(.bss*)
    }
}