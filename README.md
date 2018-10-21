# algo

Test for SUPSI project

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

### Adaptive Huffman

'A' compressed: 
* first 3 bit = header, it contains number of spare bits (3 in this example) for the last byte
* next 8 bit plain char, since it's a new char not in tree
* last 5 bit are ignored = 8 bit - 3 bit spare 
```
'A' = 011 01000001 00000
idx = 012 34567890 12345
                 1 11111                 
```

### Tests
```
--------------------------------------------------------------
ENCODING "abc(EOL)"

# REAL ALGORITHM
01100001  0  01100010    00     01100011    000   00001010  0000000110
   A     NYT    B      NYT(2)      C       NYT(3)    EOL    BOOOOOOOOH

# OUR ALGORITHM
011  01100001 01100010 01100011  00001010     00000
 H       A        B       C         EOL       BOOOH

--------------------------------------------------------------
ENCODING "abcd(EOL)"

# REAL ALGORITHM
01100001  0   01100010    00     01100011   000     01100100 100 00001010  0000000 00000001
    A    NYT      B     NYT(2)      C      NYT(3)       D    ???    EOL    BOOOOOOOOOOOOOOH

# OUR ALGORITHM
011   01100001 01100010 01100011 01100100  00001010  00000
 H       A        B       C         D         EOL    BOOOH

--------------------------------------------------------------
ENCODING "aba(EOL)"

# REAL ALGORITHM
01100001   0   01100010    1       00     00001010  000000000100
   A      NYT      B     PATH(A)  NYT(2)     EOL    BOOOOOOOOOOH

# OUR ALGORITHM
100 01100001 01100010   1      00001010 0000
 H      A       B     PATH(A)    EOL    SCARTO
```
