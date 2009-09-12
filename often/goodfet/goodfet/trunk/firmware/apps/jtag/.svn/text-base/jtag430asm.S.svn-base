.globl jtag430_tclk_flashpulses
.type jtag430_tclk_flashpulses,@function //for linking


//! At 3.68MHz, 7 to 14 cycles/loop are allowed for 257 to 475kHz
jtag430_tclk_flashpulses:
	mov #0x0031, r14
pulseloop:	
	bis.b #2, @r14		;SETTCLK, 3 cycles
	sub #1, r15		; 1 cycle
	;;  1+3+3+1+2=10, within limits
	bic.b #2, @r14		;CLRTCLK, 3 cycles
	tst r15			; 1 cycle
	jnz pulseloop		; 2 cycles
	ret

