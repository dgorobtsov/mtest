## mtest

A set of simple memory tests implemented in assembler for rv32im architecture. 
Created in educational purposes to get familiarized with RISC-V ISA. To be executed 
as baremetal application in Spike simulator environment. 

## Usage 

The required RISC-V tools (https://github.com/riscv/riscv-tools) including Spike ISA simulator 
and gcc newlib toolchain are supposed to be preliminary installed in the directory specified 
by `RISCV` environment variable.

As normal issue `make all` to get the application built and then `spike pk mtest -h`
for details regarding command line options.

## License      

MIT License

Copyright (c) 2018 Dmitry Gorobtsov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
   
