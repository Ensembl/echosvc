# echosvc
TCP echo service for localhost to help in autossh installations.

This is not an all-bells-and-whistles TCP echo service (should such a thing be conceivable!) but to support autossh. 

Therefore, it binds only to port 7, and only to localhost, and various other things which a fully featured program would make configurable. Such features will probably not be added, as the goal is for minimal dependencies and clarity, to aid travel towards correctness.
