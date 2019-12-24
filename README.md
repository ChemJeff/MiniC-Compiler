## MiniC Comipler

MiniC compiler for compiling practise in PKU, fall 2019

### Build

requirements:

+ `cmake >= 3.0`
+ `flex`
+ `bison`
+ c++ compiler with `c++11` standard support

commands:

```bash
$ cmake ./code
$ cd code
$ make clean && make
$ cd ..
```

### Usage

if input file not specified, then read source code from `stdin`.

if output file not specified, then write output to `stdout` .

##### MiniC2Eeyore

```shell
$ ./code/bin/Minic2Eeyore [<filename> [-o <filename>]]
```

##### Eeyore2Tigger

```shell
$ ./code/bin/Eeyore2Tigger [<filename> [-o <filename>]]
```

##### Tigger2RISCV32

```shell
$ ./code/bin/Tigger2RISCV32 [<filename> [-o <filename>]]
```

##### MiniC2RISCV32

```shell
$ ./code/bin/MiniC2RISCV32 [<filename> [-o <filename>]]
```

