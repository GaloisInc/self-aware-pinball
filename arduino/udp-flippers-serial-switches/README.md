Remote and switch-activation of flippers
========================================

This program listens on UDP for incoming packets, the lower-order four
bits of which specify the state to put the flippers into. It also
listens on the serial port for the switch matrix, and activates the
lower-right flipper on a timer when the corresponding switch is
flipped.

It can be used together with the Haskell and Idris remote control
clients elsewhere in the repo.
