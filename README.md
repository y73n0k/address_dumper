# Address dumper
Universal (tested only on x64 and only Arch, Ubuntu distros) tool for dumping base address of loaded to executable library.

## Tools
* `disable_aslr.c` - tool for executing process with disabled aslr :scream_cat:
* `address_dumper.c` - tool for dumping address

## Build
`address_dumper.c` has customizable parameters:
* `SUBSTR` - substring to match in path of library
* `WORDLEN` - length of machine word in `char`s
* `SKIPS` - number of execv syscalls to skip before dumping address
* `IS32` - this should be defined if target executable is compiled in 32-bti mode

You should manyally pass `WORDLEN` and `IS32` to `gcc` by `-D`:

```
gcc address_dumper.c -o address_dumper -DWORDLEN=4 -DIS32
```

Or to `make`:

```
make IS32=1 WORDLEN=4
```

Values of `SUBSTR` and `SKIPS` are params of function `dumper`.

## Usage
Very simple

```
./disable_aslr <exe> [params of exe]
```

```
./address_dumper <exe> [params of exe]
```
