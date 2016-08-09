# tcp-echo
TCP echo service for localhost to help in autossh implementations.

This is not intended to be an all-bells-and-whistles TCP echo service (should such a thing be possible!) but to support autossh. 

Therefore it binds to port 7 and only to localhost and various other things which a fully featured program would have. Such features will probably not be added as the goal is minimal dependencies and clarity to head towards correctness.
