#! /usr/local/bin/wish4.0

proc readsomething {} {
  if {[gets stdin line] < 0} {
        puts stderr "EOF on input, exiting..."
        exit
  }
  # Translate geomview's lisp-style ()'s into tcl-friendly {} braces.
  regsub -all {\(} $line "\{" line
  regsub -all {\)} $line "\}" line
  # Strip outermost set of braces
  set stuff [lindex $line 0]
  # invoke handler for whichever command we got
  switch [lindex $stuff 0] {
        pick     {handlepick $stuff}
        rawevent {handlekey $stuff}
  }
}

# Having asked for notification of these raw events, we report when
# the user pressed these keys in the geomview graphics windows.
proc handlekey {event} {
  global lastincr
  switch [lindex $event 1] {
    32 {msg "Pressed space bar"}
     8 {msg "Pressed backspace key"}    # Backspace: decrement it.
  }
}

# Fields of a "pick" response, from geomview manual:
#     (pick COORDSYS GEOMID G V E F P VI EI FI)
#          The pick command is executed internally in response to pick
#          events (right mouse double click).
#
#          COORDSYS = coordinate system in which coordinates of the following
#              arguments are specified.   This can be:
#               world: world coord sys
#               self:  coord sys of the picked geom (GEOMID)
#               primitive: coord sys of the actual primitive within
#                   the picked geom where the pick occurred.
#          GEOMID = id of picked geom
#          G = picked point (actual intersection of pick ray with object)
#          V = picked vertex, if any
#          E = picked edge, if any
#          F = picked face
#          P = path to picked primitive [0 or more]
#          VI = index of picked vertex in primitive
#          EI = list of indices of endpoints of picked edge, if any
#          FI = index of picked face

# User picked something.
# 
proc handlepick {pick} {
  global nameof selvert seledge order
  set obj [lindex $pick 2]
  set xyzw [lindex $pick 3]
  set fv [lindex $pick 6]
  set vi [lindex $pick 8] 
  set ei [lindex $pick 9] 
  set fi [lindex $pick 10]

  # Report result.
  set w [lindex $xyzw 3]
  set x [expr [lindex $xyzw 0]/$w]
  set y [expr [lindex $xyzw 1]/$w]
  set z [expr [lindex $xyzw 2]/$w]
  set s "$x $y $z "
  if {$vi >= 0} {
        set s "$s  vertex #$vi"
  }
  if {$ei != {}} {
        set s "$s  edge [lindex $ei 0]-[lindex $ei 1]"
  }
  if {$fi != -1} {
        set s "$s  face #$fi ([expr [llength $fv]/4]-gon)"
  }
  msg $s
}


proc loadobject {fname} {
  if {$fname != ""} {
        puts "(geometry thing < $fname)"
        # Be sure to flush output to ensure geomview receives this now!
        flush stdout
  }
}

#
# Display a message on the control panel
#
proc msg {str} {
  global msgtext
  puts stderr $str
  set msgtext $str
  update
}


# Build "user interface"
# This doesn't need to be an entry box, but we want to be able to
# use X selection to copy text from it.
entry .msg -textvariable msgtext -width 30
pack .msg -fill x -expand true

frame .f
label .f.l -text "File to load:"
pack .f.l -side left
entry .f.ent -textvariable fname
pack .f.ent -side left -expand true -fill x
pack .f

bind .f.ent <Return> { loadobject $fname }

# End UI definition.


# Call "readsomething" when data arrives from geomview.

fileevent stdin readable {readsomething}

# Geomview initialization

puts {
        (interest (pick primitive))
        (interest (rawevent 32))
        (interest (rawevent 8))
        (geometry thing < hdodec.off)
        (normalization world none)
}
flush stdout

wm title . {Sample external module}

msg "Click right mouse in graphics window"
