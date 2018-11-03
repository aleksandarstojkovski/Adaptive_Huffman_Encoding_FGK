# Adaptive Huffman Encoding (FGK)

## Description
Test for SUPSI project

## Compilation
````
make
````
or, manual compilation
````
gcc -lm *.c -o adaptive_huffman
````

## Execution
Encode
````
./adaptive_huffman -c <input_file> <output_file>
````
Decode
````
./adaptive_huffman -d <input_file> <output_file>
````

### utils
 hexadecimal dump of a file: **xxd**
 
```
xxd -b ../test-res/singlechar
00000000: 01000001                                               A

xxd -b ./out.compressed 
00000000: 01101000 00100000                                      ..
```

```
xxd -b ../test-res/ff_ff_ff 
00000000: 11111111 11111111 11111111                             ...
```

### ASCII table
https://en.wikipedia.org/wiki/ASCII

```
'A' = 0100 0001
idx = 0123 4567

'AB'= 0100 0001 0100 0010
idx = 0123 4567 8901 2345
                  11 1111
```

### Our Adaptive Huffman implementation

encoding "A": 
* first 3 bit = header, it contains number of bits (for last byte )to ignore. 5 in this example
* next  8 bit = plain char A, since it's a new char not in tree
* last  5 bit = bit to ignore. (see header)

```
101 01000001 00000
 H     A     -----                    
```

encoding "AB":   
* first 3 bit = header, it contains number of bits (for last byte )to ignore. 4 in this example
* next  8 bit = plain char A, since it's a new char not in tree
* next  1 bit = NYT code, since next char is new 
* next  8 bit = plain char B, since it's a new char not in tree
* last  4 bit = bit to ignore. (see header) 

```
10001000 00100100 00100000

100 01000001  0   01000010    0000
 H     A     NYT     B        ----                      
```

encoding "ABA":
```
01101000 00100100 00101000

011 01000001  0  01000010   1   000
 H     A     NYT     B     (A)  ---
```

encoding "ABAB":
```
00101000 00100100 00101010

001 01000001  0  01000010   1    01   0
 H     A     NYT     B     (A)  (B)   -
```

### Tests other implementations

encoding "abc(EOL)":

```
01100001  0  01100010    00     01100011    000   00001010  0000000110
   A     NYT    B      NYT(2)      C       NYT(3)    EOL    BOOOOOOOOH
```

encoding "abcd(EOL)":

```
01100001  0   01100010    00     01100011   000     01100100 100 00001010  0000000 00000001
    A    NYT      B     NYT(2)      C      NYT(3)       D    ???    EOL    BOOOOOOOOOOOOOOH
```

encoding "aba(EOL)":

```
01100001   0   01100010    1       00     00001010  000000000100
   A      NYT      B     PATH(A)  NYT(2)     EOL    BOOOOOOOOOOH

```
