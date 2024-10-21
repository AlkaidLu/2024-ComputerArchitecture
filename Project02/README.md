# README

这是我的流水线作业的说明！



## 项目结构 

项目包含以下关键文件： 

- **code.asm**：用于仿真的汇编代码。 

- **code.txt**：作为仿真输入的机器码文件。 

- **comm.h**，**fast.h**，**memory.h**，**Instruction.h**：定义仿真器各组件的头文件。 

-  **MIPSSimulator.cpp**，**Pipeline.cpp**，**producer_consumer.cpp**：实现仿真器、流水线阶段和同步机制的源文件。 

-  **pipeline_output.txt**：包含流水线阶段信息的输出文件。 

-  **result.txt**：存储最终仿真结果的文本文件。

  

## 运行说明

`make`

`./main "code.txt"`

## 结果

结果主要存在pipeline.txt里。看起来应该行为没有问题！

然后这个是运行完毕的截图。

这下面的不会出现在pipeline.txt里面。

![image-20241021221129958](C:\Users\Lianyi\AppData\Roaming\Typora\typora-user-images\image-20241021221129958.png)