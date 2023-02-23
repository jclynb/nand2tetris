#!/usr/bin/env python3
import sys

class Parser:
    def __init__(self, line):
        self.line = line
        self.ctypes = {"add": "C_ARITHMETIC", "sub": "C_ARITHMETIC", "neg": "C_ARITHMETIC",
                       "not": "C_ARITHMETIC", "eq": "C_ARITHMETIC", "gt": "C_ARITHMETIC",
                       "lt": "C_ARITHMETIC", "and": "C_ARITHMETIC", "or": "C_ARITHMETIC",
                       "push": "C_PUSH", "pop": "C_POP"} #, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL}
        self.ctype = None
        self.arg1 = None
        self.arg2 = None

    def set_command_type(self):
        commands = self.line.split(" ")
        command = commands[0]
        self.ctype = self.ctypes[command]
        if self.ctype == "C_ARITHMETIC":
            self.arg1 = commands[0]
        elif self.ctype != "C_RETURN":
            self.arg1 = commands[1]
        if self.ctype == "C_PUSH" or self.ctype == "C_POP":
            self.arg2 = commands[2]

class codeWriter:
    def __init__(self, dest):
        self.dest = dest
        self.output = open(dest, "w")
        self.cmap = {"add": "+", "sub": "-", "neg": "-",
                    "not": "!", "and": "&", "or": "|",
                     "eq": "D;JEQ", "gt": "D;JGT", "lt": "D;JLT",
                     "temp": "5", "pointer": "3", "local": "LCL",
                     "argument": "ARG", "this": "THIS", "that": "THAT"}
        self.jumpcount = 0

    def writeArithmetic(self, arg1):
        # add: pop values from stack, perform operation, push result to stack
        if arg1 == "add" or arg1 == "sub":
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n") # RAM[SP] = RAM[SP] - 1
            self.output.write("D=M\n") # D = *RAM[A]
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n")
            self.output.write("M=M{operator}D\n".format(operator=self.cmap[arg1]))
            self.output.write("@SP\n")
            self.output.write("M=M+1\n")
        elif arg1 == "neg" or arg1 == "not":
            self.output.write("@SP\n")
            self.output.write("A=M-1\n")
            self.output.write("M={operator}M\n".format(operator=self.cmap[arg1]))
        elif arg1 == "and" or arg1 == "or":
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n")
            self.output.write("D=M\n")
            self.output.write("@SP\n")
            self.output.write("A=M-1\n")
            self.output.write("M=M{operator}D\n".format(operator=self.cmap[arg1]))
        elif arg1 == "eq" or arg1 == "gt" or arg1 =="lt":
            label = "label{count}".format(count=self.jumpcount)
            self.jumpcount += 1
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n") # RAM[SP] = RAM[SP] - 1
            self.output.write("D=M\n") # D = *RAM[A]
            self.output.write("@SP\n")
            self.output.write("A=M-1\n")
            self.output.write("D=M-D\n") # D is positive, negative, or 0
            self.output.write("M=-1\n") # RAM[SP] set to True
            self.output.write("@{l}\n".format(l=label))
            self.output.write("{operator}\n".format(operator=self.cmap[arg1]))
            self.output.write("@SP\n")
            self.output.write("A=M-1\n") # Pop true
            self.output.write("M=0\n") # Set to false
            self.output.write("({l})\n".format(l=label))
        else:
            raise NameError("Not a valid arithmetic command")


    def writePush(self, arg1, arg2):
        if arg1 == "constant" or arg1 == "static":
            # RAM[SP] = i, SP++
            self.output.write("@{value}\n".format(value=arg2))
            if arg1 == "constant":
                self.output.write("D=A\n")
            else: # static i -> access assembly variable Foo.i
                self.output.write("D=M\n")
            self.output.write("@SP\n")
            self.output.write("A=M\n")
            self.output.write("M=D\n")
            self.output.write("@SP\n")
            self.output.write("M=M+1\n")
        elif arg1 == "temp" or arg1 == "pointer":
            # temp i -> RAM[5+i]
            self.output.write("@{value}\n".format(value=arg2))
            self.output.write("D=A\n")
            self.output.write("@{num}\n".format(num=self.cmap[arg1]))
            self.output.write("A=A+D\n")
            self.output.write("D=M\n")
            self.output.write("@SP\n")
            self.output.write("A=M\n")
            self.output.write("M=D\n")
            self.output.write("@SP\n")
            self.output.write("M=M+1\n")
        elif arg1 == "local" or arg1 == "argument" or arg1 == "this" or arg1 == "that":
            # addr <- LCL + i, RAM[SP] <- RAM[addr], SP++
            self.output.write("@{value}\n".format(value=arg2))
            self.output.write("D=A\n")
            self.output.write("@{name}\n".format(name=self.cmap[arg1]))
            self.output.write("A=D+M\n")
            self.output.write("D=M\n")
            self.output.write("@SP\n")
            self.output.write("A=M\n")
            self.output.write("M=D\n")
            self.output.write("@SP\n")
            self.output.write("M=M+1\n")
        else:
            raise NameError("Not a valid push command")

    def writePop(self, arg1, arg2):
        if arg1 == "static":
            # pop Foo.i
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n")
            self.output.write("D=M\n")
            self.output.write("@{value}\n".format(value=arg2))
            self.output.write("M=D\n")
        elif arg1 == "temp" or arg1 == "pointer":
            # pop RAM[5+i]
            self.output.write("@{value}\n".format(value=arg2))
            self.output.write("D=A\n")
            self.output.write("@{num}\n".format(num=self.cmap[arg1]))
            self.output.write("D=A+D\n")
            self.output.write("@R13\n") # save D value in general purpose register
            self.output.write("M=D\n")
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n")
            self.output.write("D=M\n")
            self.output.write("@R13\n")
            self.output.write("A=M\n") # get addr from R13
            self.output.write("M=D\n")
        elif arg1 == "local" or arg1 == "argument" or arg1 == "this" or arg1 == "that":
            # addr <- LCL + i, SP--, RAM[addr] <- RAM[SP]
            self.output.write("@{value}\n".format(value=arg2))
            self.output.write("D=A\n")
            self.output.write("@{name}\n".format(name=self.cmap[arg1]))
            self.output.write("D=D+M\n")
            self.output.write("@R13\n") # save D value in general purpose register
            self.output.write("M=D\n")
            self.output.write("@SP\n")
            self.output.write("AM=M-1\n")
            self.output.write("D=M\n")
            self.output.write("@R13\n")
            self.output.write("A=M\n") # get addr from R13
            self.output.write("M=D\n") # set RAM[addr] to RAM[SP]
        else:
            raise NameError("Not a valid pop command")

    def closefile(self):
        self.output.close()

def main():
    '''
    Parses each line of a .vm file to exclude whitespace and comments
    Each command is translated to hack assembly and written to a .asm file
    '''
    if len(sys.argv) != 2 or sys.argv[1][-3:] != ".vm":
        raise IndexError("command line error: incorrect input or file type")

    filename = sys.argv[1]

    f = open(filename, "r")
    dest = "{name}.asm".format(name = filename[:-3]) # remove .vm
    writer = codeWriter(dest)

    for line in f:
        line = line.strip()
        if not line.startswith("/") and line: # Skip comments and blank lines
            parsed_line = Parser(line)
            parsed_line.set_command_type()

            if parsed_line.ctype == "C_PUSH":
                writer.writePush(parsed_line.arg1, parsed_line.arg2)

            elif parsed_line.ctype == "C_POP":
                writer.writePop(parsed_line.arg1, parsed_line.arg2)

            elif parsed_line.ctype == "C_ARITHMETIC":
                writer.writeArithmetic(parsed_line.arg1)

    writer.closefile()
    f.close()

if __name__ == "__main__":
    main()
