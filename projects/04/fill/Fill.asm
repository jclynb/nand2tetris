// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.
(KEYBOARD)
    // Set D to the code of currently pressed key
    // if no key is pressed, D=0
    @KBD
    D=M
    // if  D != 0, goto SETBLACK
    @SETBLACK
    D;JNE
    // if D == 0, goto SETWHITE
    @SETWHITE
    D;JEQ
    // goto KEYBOARD
    @KEYBOARD
    0;JMP

(SETBLACK)
    @COLOR
    M=-1
    // goto FILLSCREEN
    @FILLSCREEN
    0;JMP

(SETWHITE)
    @COLOR
    M=0
    // goto FILLSCREEN
    @FILLSCREEN
    0;JMP

(FILLSCREEN)
    // get color value
    @COLOR
    D=M
    D=A
    // start at 8192 (end of SCREEN memory map) and count down
    @8192
    D=A
    @offset
    M=D

    (LOOP)
    // goto KEYBOARD once offset = 0
    @offset
    D=M
    @KEYBOARD
    D;JEQ

    // decrement offset
    @offset
    M=M-1

    // write screen[offset] to color
    @offset
    D=M
    @SCREEN
    D=D+A

    @target
    M=D
    @COLOR
    D=M
    @target
    A=M
    // @SCREEN+target=color
    M=D

    @LOOP
    0;JMP
