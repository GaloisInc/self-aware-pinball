Pinball machine remote controller
=================================

This is a client for the remote control pinball. It creates a window
using SDL, and reads keyboard events. These are sent to the pinball
Arduino over UDP.

Instructions
------------

1. Install the Idris SDL bindings

2. Use `idris --build pinballcontroller.ipkg` to compile the program.

3. Run `pinballclient`.
