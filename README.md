# SuperDuperDnsServer
BigBoss experiment

As for 'Super Duper', thanks to Donald Trump.(doge)

# 配置文件说明
***

* 静态服务器解析地址，存放在当前目录下的server.cache文件中，信息格式为"Ipv4地址 空格 主机名",例如202.112.14.178 www.uestc.edu.cn
* 上层服务器解析地址，存放在当前目录下的upperproxy.confg文件中，信息格式同样为"Ipv4地址 空格 主机名",此处，主机名可以随意设置，此处为了方便便随意设置主机名。例子：8.8.8.8   8.8.8.8。
* 具体配置格式，可以参照以上提到两个文件中存在的信息

# Log
* version 1.0 achieved some class that may be used in the dns proxy server.
* version 2.0 fix some bugs, really tiny but dangerous bugs.
* Create new class toolMessage
* create tic-toc structure
* redefine the process function
* complete proxy function by the configuration of upper dns proxy server
* complete cache mechanism(simple version)
* combine the cache mechanism and proxy function
* complete the Makefile, Jesus this is the first written by myself in my whole life....


# TODO
* ~~version 1.0 use getaddrinfo() function to get the ip server, so next version will try to get the correct ip address by sending query to the upper dns server~~
* This is wait to be sure. And this is not the required point of this project. Currrently, searching the cache consume O(n) time. In my analyse, use the Red-Black tree will lower down that to O(logn). But this depends on whether I can finish other experiment in time. The other experiment is so much.
* ~~version 1.0 need at makefile, gosh... I have to learn another big deal...~~
* ~~complete the proxy function, just send the original message to the upper dns server (bingo)~~
* ~~add cache function later(bingo)~~
* ~~suppose there is a cache, using cache(bingo)~~
* ~~try to concatenate this two mechanism(bingo)~~
* ~~trying to update cache(bingo)~~
* ~~add tic-toc  structure in resolver to recording the time(bingo)~~
* ~~get new cache material from the response of upper server(bingo)~~
* ~~achieve decode response package(bingo)~~
* log function
