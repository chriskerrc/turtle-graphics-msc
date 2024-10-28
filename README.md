Project for Programming in C MSc module. A recursive descent parser and interpreter for a "turtle graphics" language (similar to [LOGO](https://en.wikipedia.org/wiki/Logo_\(_programming_language_\)). 

The program parses and interprets .TTL files stored in the /TTL folder. See grammar.txt for the formal grammar of the TTL language. 

To parse: 

```
./parse TTLs/filename.ttl
```

To interpret with output to terminal:

```
./interp TTLs/filename.ttl
```

To interpret with output to .txt file: 

```
./interp TTLs/filename.ttl filename.txt
```

Requires [Valgrind](https://valgrind.org/), which has limited support on macOS.
