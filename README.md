# RK-TW
一个使用TimeWheel思想实现的定时器。
a implementation of TimeWheel with C.

## 功能
1. 创建、启动和销毁时间轮
2. 添加、提前、延后和删除定时任务
3. 按照一定时间间隔循环执行定时任务

# 设计
![design](design.png)

## 待办
1. 每个slot使用多级队列
