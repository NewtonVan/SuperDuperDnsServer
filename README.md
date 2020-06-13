# SuperDuperDnsServer
BigBoss experiment

As for 'Super Duper', thanks to Donald Trump.(doge)

# Log
* version 1.0 achieved some class that may be used in the dns proxy server.

# TODO
* version 1.0 use getaddrinfo() function to get the ip server, so next version will try to get the correct ip address by sending query to the upper dns server
* This is wait to be sure. And this is not the required point of this project. Currrently, searching the cache consume O(n) time. In my analyse, use the Red-Black tree will lower down that to O(logn). But this depends on whether I can finish other experiment in time. The other experiment is so much.
* version 1.0 need at makefile, gosh... I have to learn another big deal...
