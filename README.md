# C0_Compiler
Final Project of ' Design of Compiler Technology' Course of SCSE,BUAA
### Overview
This is the repository for the project of "Design of Compiler Technology".This project contains a simple compiler which can translate a 'C0' language to MIPS assembly language.
### About the 'C0 language'
C0 language is a slightly modified and simplified High-level programming language based on C language. The grammar of this 'C0'  language is LL(1) grammar.<br>
The Grammar of this C0 language is shown bellow.<br>
[!](https://github.com/ComradeProgrammer/C0_Compiler_BUAA/blob/master/Annotation%202019-09-26%20123820.jpg?raw=true)
### About the simple compiler
This compiler contains a 'frontend' and a 'backend.<br>
In the front end we implemented Lexical analysis,Grammar analysis ( recursive-descend method), and the fault handling. The front end can translate the original C0 language to midcode.
The backend translates midcode to the target assembly language : mips. The following optimization is Implemented:

- Active variables analysis
- automatic inline function
- DAG optimization (elimination of shared expression)
- Elimination of dead code
- substitution for assign instruction
- constant proporgation and substitution
- peephole optimization
- Simple calculation in compilation phase
- graph coloring algorithm for distribution of global register
- distribution of temporary register based on active variables analysis
### Development Environment
Editor:VS2019<br>
Complier: clang++ 8.0 msvc g++7.2.0<br>
Debugger: gdb debugger of msvc
### How to use this compiler
In command line:
```
usage: C0compiler.exe sourcefile
[-o outputfile]
[-d debugInformation]
[-rdi recursiveDescendInformationFile]
[-h] help document
[-opt] turn on the optimization
```
if no argument is passed to the complier,the program will be run under HomeWork Acceptance Test Mode. Under such circumstance,the source file must be "testfile.txt" and the output file must be "mips.txt"

