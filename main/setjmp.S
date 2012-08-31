/* This is a simple version of setjmp and longjmp for MIPS 32 and 64.

   The MIPS 32 version does NOT save the floating point register, which is
   wrong, but I don't know how to cleanly handle machines without a
   floating point coprocessor.

   Ian Lance Taylor, Cygnus Support, 13 May 1993.

   The MIPS 64 version saves registers fp20 to fp31.  23 registers
   are saved in all.  */

/* int setjmp (jmp_buf);  */
	.globl	setjmp
	.ent	setjmp
setjmp:
	.frame	$sp,0,$31

	sd      $16, 000($4)    /* s0 */
	sd      $17, 010($4)    /* s1 */
	sd      $18, 020($4)    /* s2 */
	sd      $19, 030($4)    /* s3 */
	sd      $20, 040($4)    /* s4 */
	sd      $21, 050($4)    /* s5 */
	sd      $22, 060($4)    /* s6 */
	sd      $23, 070($4)    /* s7 */
	sd		$29, 0240($4)   /* sp */
	sd		$30, 0250($4)   /* fp */
	sd		$31, 0260($4)   /* ra */

	move	$2,$0

	j	$31

	.end	setjmp

/* volatile void longjmp (jmp_buf, int);  */
	.globl	longjmp
	.ent	longjmp
longjmp:
	.frame	$sp,0,$31

	ld      $16, 000($4)    /* s0 */
	ld      $17, 010($4)    /* s1 */
	ld      $18, 020($4)    /* s2 */
	ld      $19, 030($4)    /* s3 */
	ld      $20, 040($4)    /* s4 */
	ld      $21, 050($4)    /* s5 */
	ld      $22, 060($4)    /* s6 */
	ld      $23, 070($4)    /* s7 */
	ld		$29, 0240($4)   /* sp */
	ld		$30, 0250($4)   /* fp */
	ld		$31, 0260($4)   /* ra */

	bne	$5,$0,1f
	li	$5,1
1:
	move	$2,$5

	j	$31

	.end longjmp
