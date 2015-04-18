tinyweb - tiny web server library and daemon
============================================

The tinyweb project includes a rudimentary web server in library form
(libtinyweb), and a simple web server daemon (tinywebd) which uses it.

Author: John Tsiombikas <nuclear@member.fsf.org>

I disclaim all copyright to this program, and place it in the public domain.
Feel free to use it any way you like. Mentions and retaining the attribution
headers at the top of the source code would be appreciated, but not required.

Feel free to send me an email if you find a cool use for this code.

Download
--------
Latest release: no releases yet, just grab the source from the repo.

Code repository: https://github.com/jtsiomb/tinyweb

Usage
-----
See ``src/main.c`` as an example on how to use libtinyweb, and read the header
file ``libtinyweb/src/tinyweb.h`` which is very simple, and heavily commented.

It will serve everything under the current working directory. Default port is
8080.

Bugs
----
Issues that I intend to fix or improve at some point:

- Only GET and HEAD HTTP requests are currently implemented.
- Doesn't support partial downloads.
- It's supposed to be cross platform, but it isn't yet (UNIX only).

Issues that are the way they are by design:

- Tinywebd is not really a daemon (doesn't release controlling terminal).
- Doesn't scale well to lots of simultaneous clients (to keep it simple).
