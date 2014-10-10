module Pinball


import Effects
import Effect.SDL
import Effect.State
import Effect.StdIO
import Effect.Socket

record Flippers : Type where
  MkFlippers : (f1, f2, f3, f4 : Bool) -> Flippers

msg : Flippers -> String
msg (MkFlippers f1 f2 f3 f4) =
  singleton $ prim__intToChar $ prim__zextB8_Int $
  (if f1 then 0x1 else 0) `prim__orB8` 
  ((if f2 then 0x2 else 0) `prim__orB8`  
   ((if f3 then 0x4 else 0) `prim__orB8`
    ((if f4 then 0x8 else 0) `prim__orB8` 0x10)))

doFlippers : { [ STATE Flippers, UDP Socket ] } Eff ()
doFlippers = do send (IPv4Addr 192 168 43 119) 2811 (msg !get)
                return ()

keys : Maybe Event -> { [ SDL_ON, STATE Flippers, UDP Socket ] } Eff Bool
keys (Just AppQuit) = return False
keys (Just (KeyDown (KeyAny 'a'))) = do update (record { f1 = True })
                                        doFlippers
                                        return True
keys (Just (KeyUp (KeyAny 'a'))) = do update (record { f1 = False})
                                      doFlippers
                                      return True
keys (Just (KeyDown (KeyAny 's'))) = do update (record { f2 = True })
                                        doFlippers
                                        return True
keys (Just (KeyUp (KeyAny 's'))) = do update (record { f2 = False})
                                      doFlippers
                                      return True
keys (Just (KeyDown (KeyAny 'k'))) = do update (record { f3 = True })
                                        doFlippers
                                        return True
keys (Just (KeyUp (KeyAny 'k'))) = do update (record { f3 = False})
                                      doFlippers
                                      return True
keys (Just (KeyDown (KeyAny 'l'))) = do update (record { f4 = True })
                                        doFlippers
                                        return True
keys (Just (KeyUp (KeyAny 'l'))) = do update (record { f4 = False})
                                      doFlippers
                                      return True
keys _ = return True


draw : { [ SDL_ON, STATE Flippers ] } Eff ()
draw = with Effects do
         MkFlippers a b c d <- get
         ellipse (colour a) 40 40 10 10
         ellipse (colour b) 80 40 10 10
         ellipse (colour c) 120 40 10 10
         ellipse (colour d) 160 40 10 10                           
   where colour : Bool -> Colour
         colour True = green
         colour False = white

main' : { [ SDL (), STATE Flippers, STDIO, UDP () ] } Eff ()
main' = do putStrLn "Starting"
           case !mksocket of
             Left err => putStrLn "No connect"
             Right () => do initialise 600 100
                            loop
                            quit
                            close

  where
    loop : { [ SDL_ON, STATE Flippers, STDIO, UDP Socket ] } Eff ()
    loop = do draw
              flip
              flippers <- get
              when !(keys !poll) loop

namespace Main
  main : IO ()
  main = runInit [(), MkFlippers False False False False, (), ()] main'
