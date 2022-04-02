# webassembly-vm

Webassembly local machine that execute webassembly binary code in C.

Collaborated with Tianze Shan, Yinghan Dai.


Version 1.0 Supporting Arithmatic Operation and Logical Operation

Open for pull request for adding more features :)


## To Run

First make the executable by doing 

`make vm`

To execute by calling the funciton in the binary file following this format 

`./vm arith.wasm <function name>  <input 1> <input 2>`

For example, you can perform "add 2 and 5" by this command line

`./vm arith.wasm add 2 5`


Note that if you are getting "Permission denied" as error, do the following command

`chomd +x vm`
