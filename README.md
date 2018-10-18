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
