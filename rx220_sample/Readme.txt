-------- PROJECT GENERATOR --------
PROJECT NAME :	rx220_sample
PROJECT DIRECTORY :	C:\githubwork\rx220_sample\rx220_sample
CPU SERIES :	RX200
CPU TYPE :	RX210
TOOLCHAIN NAME :	Renesas RX Standard Toolchain
TOOLCHAIN VERSION :	1.2.1.0
GENERATION FILES :
    C:\githubwork\rx220_sample\rx220_sample\dbsct.c
        Setting of B,R Section
    C:\githubwork\rx220_sample\rx220_sample\typedefine.h
        Aliases of Integer Type
    C:\githubwork\rx220_sample\rx220_sample\sbrk.c
        Program of sbrk
    C:\githubwork\rx220_sample\rx220_sample\iodefine.h
        Definition of I/O Register
    C:\githubwork\rx220_sample\rx220_sample\intprg.c
        Interrupt Program
    C:\githubwork\rx220_sample\rx220_sample\vecttbl.c
        Initialize of Vector Table
    C:\githubwork\rx220_sample\rx220_sample\vect.h
        Definition of Vector
    C:\githubwork\rx220_sample\rx220_sample\resetprg.c
        Reset Program
    C:\githubwork\rx220_sample\rx220_sample\rx220_sample.c
        Main Program
    C:\githubwork\rx220_sample\rx220_sample\sbrk.h
        Header file of sbrk file
    C:\githubwork\rx220_sample\rx220_sample\stacksct.h
        Setting of Stack area
START ADDRESS OF SECTION :
 H'1000	B_1,R_1,B_2,R_2,B,R,SU,SI
 H'FFFF8000	PResetPRG
 H'FFFF8100	C_1,C_2,C,C$*,D_1,D_2,D,P,PIntPRG,W*,L
 H'FFFFFFD0	FIXEDVECT

* When the user program is executed,
* the interrupt mask has been masked.
* 
* Program start 0xFFFF8000.
* RAM start 0x1000.

DATE & TIME : 2015/07/20 10:59:11
