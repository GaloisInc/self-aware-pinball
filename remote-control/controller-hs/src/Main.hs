module Main where

import System.Environment (getArgs)
import System.Exit
import System.IO

import Control.Monad (when)
import Control.Exception (bracket)

import Graphics.Gloss
import Graphics.Gloss.Interface.IO.Game hiding (shift)

import Network.Socket hiding (send, sendTo, recv, recvFrom)
import Network.Socket.ByteString

import Data.Bits
import qualified Data.ByteString as BS

data World = World { sock :: Socket
                   , address :: AddrInfo
                   , a :: Bool
                   , b :: Bool
                   , c :: Bool
                   , d :: Bool }

instance Show World where
  show (World _ _ a b c d) = show (a, b, c, d)

encode :: World -> BS.ByteString
encode (World _ _ a b c d) =
  BS.singleton $ 
    (if a then 1 else 0) .|.
    (if b then 1 `shift` 1 else 0) .|.
    (if c then 1 `shift` 2 else 0) .|.
    (if d then 1 `shift` 3 else 0)

update :: Event -> World -> IO World
update (EventKey (Char 'l') Up   _ _) w = return w { a = False }
update (EventKey (Char 'l') Down _ _) w = return w { a = True }
update (EventKey (Char 's') Up   _ _) w = return w { b = False }
update (EventKey (Char 's') Down _ _) w = return w { b = True }
update (EventKey (Char 'k') Up   _ _) w = return w { c = False }
update (EventKey (Char 'k') Down _ _) w = return w { c = True }
update (EventKey (Char 'a') Up   _ _) w = return w { d = False }
update (EventKey (Char 'a') Down _ _) w = return w { d = True }
update _ w = return w

sendState :: World -> IO ()
sendState w = sendAllTo (sock w) (encode w) (addrAddress $ address w)

getAddr :: IO String
getAddr = do args <- getArgs
             case args of
               [addr] -> return addr
               _ -> do hPutStrLn stderr "Please provide 1 host to connect to as arg"
                       exitFailure

main :: IO ()
main = withSocketsDo $
       do addr <- getAddr
          let hints = defaultHints { addrSocketType = Datagram
                                   , addrFlags = [AI_ADDRCONFIG]
                                   , addrFamily = AF_INET
                                   }
          ais <- getAddrInfo (Just hints) (Just addr) (Just "2811")
          ai  <- case ais of
            []     -> hPutStrLn stderr "Unknown host" >> exitFailure
            ai : _ -> return ai
          bracket (socket (addrFamily ai) (addrSocketType ai) (addrProtocol ai))
                  sClose
                  $ \s -> playIO (InWindow "GameEvent" (700, 100) (10, 10))
                                 white
                                 100
                                 (World s ai False False False False)
                                 -- world to picture
                                 (\st -> return . Translate (-340) 0 . Scale 0.1 0.1 . Text $
                                           show st ++ show (BS.head (encode st)))
                                 -- event to world to world - handle inputs
                                 update
                                 -- step world
                                 (\_ world -> sendState world >> return world)
