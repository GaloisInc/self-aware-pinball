module Effect.Socket

import Effects

import Network.Socket

-- A simple UDP library

||| The internal UDP effect structure
data UDPClient : Effect where
  MakeSocket : UDPClient (Either SocketError ()) () (\res => case res of
                                                               Left err => ()
                                                               Right s => Socket)
  Send : SocketAddress -> Int -> String ->
         UDPClient (Either SocketError ByteLength) Socket (const Socket)
  Close : UDPClient () Socket (const ())


instance Handler UDPClient IO where
  handle () MakeSocket k = do s <- socket AF_INET Datagram 0
                              case s of
                                Left err => k (Left err) ()
                                Right sock => do bind sock Nothing 0
                                                 k (Right ()) sock
  handle s (Send address port msg) k = do res <- (sendTo s address port msg)
                                          k res s
  handle s Close k = do close s
                        k () ()


||| A UDP socket effect.
||| @ resource either `()` or `Socket`, depending on whether the library is initialised
UDP : (resource : Type) -> EFFECT
UDP res = MkEff res UDPClient

||| Initialise a socket to send UDP packets
mksocket : { [UDP ()] ==>
             [UDP (case result of {Left _ => () ; Right _ => Socket})]
           } Eff (Either SocketError ())
mksocket = call MakeSocket

||| Send a message to a given address and port
send : SocketAddress -> Int -> String -> { [UDP Socket] } Eff (Either SocketError ByteLength)
send addr port buf = call (Send addr port buf)

||| Clean up the socket
close : { [UDP Socket] ==> [UDP ()] } Eff ()
close = call Close
