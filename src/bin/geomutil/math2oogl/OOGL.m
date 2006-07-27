(* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/bin/geomutil/math2oogl/OOGL.m,v 1.3 2006/07/27 12:29:38 mphillips Exp $ *)

BeginPackage["OOGL`"]

(*

If you want to be able to use this package from a directory other
than the geomview distribution directory, or if you want to be able to
run it from a remote host, then
EITHER
   ensure that the programs
	"geomview", "math2oogl", "togeomview", and "oogl2rib"
   are in your search path (e.g. copy them to /usr/local/bin),

OR

  - Change the value of $GeomviewPath below to be the absolute pathname
	of the "geomview" shell script on your system and
  - Change the value of $GeomRoot below to be the absolute pathname
	of the geomview distribution directory on your system.

The first alternative is preferred when possible.
*)

(* $GeomviewPath = "/usr/local/bin/geomview"; *)
(* $GeomRoot = "/u/gcg/ngrap"; *)
$GeomviewPath = "geomview";
$GeomRoot = ".";

(* DO NOT CHANGE ANYTHING BELOW HERE *)


(*******************************************************************)

(* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Geomview; see the file COPYING.  If not, write to the
 * Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA,
 * or visit http://www.gnu.org.
 *)

(* Authors: Nils McCarthy
	    Stuart Levy
	    Tamara Munzner
	    Silvio Levy
*)


(* To use this, you also need to install the math2oogl and oogl2rib C programs.
   They actually do the grunt work, as Mathematica is too slow
   to do it in any reasonable amount of time. *)

WriteOOGL::usage = "
WriteOOGL[filename,-Graphics3D or SurfaceGraphics-] writes the given
graphics object or list of graphics objects to filename.  Graphics3D
objects are converted to OOGL OFF format, SurfaceGraphics objects are
converted to OOGL MESH format. WriteOOGL understands only Polygon,
Line, and RGBColor Graphics3D directives.  If WriteOOGL encounters any
errors in the Graphics3D object, it silently tries to cope. Depends on
the C program math2oogl.";

Geomview::usage= "
Geomview [-Graphics3D or SurfaceGraphics-]\n 
Geomview [\"name\", -Graphics3D or SurfaceGraphics-] writes the given
graphics object or list of graphics objects to geomview, starting a
copy of geomview if necessary.  Graphics3D objects are converted to
OOGL OFF format, SurfaceGraphics objects are converted to OOGL MESH
format.  Polygons, Lines, Points, and RGBColor Graphics3D directives
are understood.  Geomview[\"name\", ...]  creates geomview object with
that name; default is \"Mathematica\".  Depends on the C program
math2oogl.";

WriteRIB::usage= "
WriteRIB [filename,-Graphics3D or SurfaceGraphics-]\n 
WriteRIB [filename,-Graphics3D or SurfaceGraphics-, \"options\"]]
writes the given graphics object or list of graphics objects as
RenderMan .rib files. It actually converts the Mathematica graphics
object to the OOGL data format, and then converts the OOGL files to
rib format.  Graphics3D objects become lists of Polygons, and
SurfaceGraphics objects become PatchMeshes. Polygons, Lines, Points,
and RGBColor Graphics3D directives are understood. Depends on the C
programs math2oogl and oogl2rib. WriteRIB[ ..., \"options\"] appends
the given text string to the invocation of oogl2rib.";

WriteChunk::usage= "

WriteChunk [filename,-Graphics3D or SurfaceGraphics-] is only for use
on machines that cannot start up a remote Unix shell on a machine that
has math2oogl, oogl2rib, and/or geomview installed (i.e. Macs, PCs).
It writes the Mathematica graphics object to a file in a format
accepted by shell scripts which should be run on a machine (Iris or
Next) that has the relevant programs installed. The shell scripts
\"writeoogl\",\"writegv\", and \"writerib\" correspond to WriteOOGL,
Geomview, and WriteRIB respectively: see their usage messages.";

DisplayHost::usage= "Option to Geomview[] specifying display hostname
for geomview.  Automatically set to REMOTEHOST or DISPLAY shell
variable if found.  Setting DisplayHost to \"local\" overrides this behavior.\n
To specify a different user name on the remote computer, use\n
    SetOptions[Geomview, DisplayHost -> \"user@host\" ];";

GeomviewCommand::usage =
"Option to Geomview[] specifying the command used to start geomview if\n
it is not already running.   May include geomview command-line options.";

MachType::usage= "Option to Geomview[] specifying machine type of
DisplayHost.  Should be either \"sgi\" or \"next\".  The default value
is set by Mathematica to be the same as the type of machine on which
Mathematica is running.";

(*Geomview /: Options[Geomview] := { DisplayHost -> "" };*)

MeshGraphics::usage = "\n
Head for mesh graphics object."

Options[Geomview] ^= {DisplayHost->"",
		      MachType->If [$System == "NeXT", "next",
			        If [$System == "SPARC", "sun4",
				   "sgi"] ],
		      XDisplay -> ":0",
		      GeomviewCommand -> $GeomviewPath};

Begin["`Private`"]

(* Use StringJoin not <> for backward compatibility *)

WriteChunk[filename_String, gobj_List] := (
  Map[ WriteChunk[filename,#] &, gobj]; 				   
);

WriteChunk[filename_String, gobj_Graphics3D] := (
  Write[filename, CForm[gobj[[0]]]];
  Write[filename, CForm[N[gobj[[1]]]]];
);

GoodHeadQ[gobj_] := (SameQ[gobj[[0]],SurfaceGraphics] ||
     SameQ[gobj[[0]],MeshGraphics] || SameQ[gobj[[0]],BezierPlot`BezierPatch]
	|| SameQ[gobj[[0]],BezierPatch])

WriteChunk[filename_String, gobj_?GoodHeadQ] := (
  Write[filename, CForm[gobj[[0]]]];
  WriteString[filename, "Dimensions\n"];
  Write[filename, Take[Dimensions[gobj[[1]]],2]];
  If[SameQ[gobj[[0]],SurfaceGraphics],
    WriteString[filename, "MeshRange\n"];
    Write[filename, MeshRange /. gobj[[-1]]]] ;
  (* write out the colors before the points, if they exist *)
  (* Backwards compatibility with v1.2: Dimensions[gobj] returns empty list *)
  If [$VersionNumber >= 2.0,
    If [Dimensions[gobj][[1]] > 2, Write[filename, CForm[gobj[[2]]]]],
    If [Dimensions[Dimensions[gobj[[2]]]][[1]] > 1, 
       Write[filename, CForm[gobj[[2]]]]]];
  Write[filename, CForm[N[gobj[[1]]]]];
);

If[$VersionNumber < 2.0, (
    StringTake[str_, take_] := Apply[StringJoin, Take[Characters[str], take]];
    StringPosition[str_, ch_] := Block[{chars = Characters[str]},
	Select[ Table[{i,i}, {i,1,StringLength[str]}], 
		(chars[[ #[[1]] ]] == ch)&] ];
    $Failed = Fail;
  )];

BinPrefix[mach_] := StringJoin[ $GeomRoot, "/bin/", mach ];
BinShPrefix[mach_] := StringJoin[ "PATH=$PATH:", BinPrefix[mach], " " ];

WriteOut[filename_String, gobj_, postpend_String:"", prepend_String:"",
         options___] :=(
  (* math2oogl expects '{}(), ' to be translated into newlines *)
  tr = "tr -s '{}(), ' '\\12\\12\\12\\12\\12\\12' | ";	
  (* Do the right thing if we've got a command instead of a file name. *)
  tmpfilename =
	If[StringMatchQ[filename, "!*"],
	    StringJoin[" | ",StringDrop[filename,1]],
	  If[filename != "",
	    StringJoin[" > ", filename],
	    filename]];
  newfilename =
    StringJoin["!",tr,prepend," ",
	       BinShPrefix[ MachType /. {options} /. Options[Geomview]],
	       "math2oogl ",postpend, tmpfilename];
  (* We might prepend 'rsh hostname' to the command, which must then be
   * quoted so that what comes after the pipes gets interpreted on
   * the remote not the local machine. The open quote is already in the
   * middle of the prepended string.
   * If we did prepend 'rsh hostname', also add a command to be executed in
   * case the rsh failed, to avoid e.g. mysterious "permission denied" errors.
   * StringMatchQ[prepend,"rsh*"] fails mysteriously in Math 1.2.
   *)
  If[StringLength[prepend]>3 && StringTake[prepend,3] == "rsh",
	newfilename = StringJoin[newfilename,
"'\\' || (echo Geomview graphics: Cannot rsh to machine ", rshhost, " as ",
		If[rshuser != "", StringJoin["user ", rshuser], "yourself"],
".; echo Check DisplayHost option, or your .rhosts file on ", rshhost, ".)"]];
  OpenWrite[newfilename];	
  WriteChunk[newfilename, gobj];
  Close[newfilename];	

);

WriteOOGL[filename_String,gobj_] :=(
   WriteOut[filename, gobj];
   gobj
);

WriteRIB[filename_String,gobj_, riboptions_String:"", options___] :=(
   WriteOut[filename,gobj, 
      StringJoin[" | oogl2rib -n mma.tiff ",riboptions]];
   gobj
);

(*
 * If the given filename is on the $PATH, returns the directory it's in,
 * else returns Null.  Used for load-time warning messages.
 *)
OnPathQ[filename_] :=
    Block[{pdir, i, posen, path = StringJoin[":",Environment["PATH"],":"] },
     (
	posen = StringPosition[path, ":"];
	Do[
	  (pdir = StringTake[path, {posen[[i]][[2]]+1, posen[[i+1]][[1]]-1}];
	   If[FileType[StringJoin[pdir,"/",filename]] === File,
	        Return[pdir]]),
	 {i, 1, Length[posen]-1}
	]
     )];
    


xhost[display_] := StringTake[display,
   StringPosition[StringJoin[display,":"], ":"] [[1]] [[1]]-1];

remotehost[host_String] := (
  newhost="";
  If[host != "",
    newhost = If[host == "local", "", host],
    (
      If[(rshhost = Environment["REMOTEHOST"]) =!= $Failed
	 || (rshhost = Environment["DISPLAY"]) =!= $Failed,
	  newhost = xhost[rshhost];
      ]
  )];
  If[newhost == $MachineName, newhost = ""];
  If[newhost != "" && Length[StringPosition[newhost, "@"]] == 0 &&
      ((ruser = Environment["REMOTEUSER"]) =!= $Failed
       || (ruser = Environment["USER"]) =!= $Failed
       || (ruser = Environment["LOGNAME"]) =!= $Failed),
    newhost = StringJoin[ruser, "@", newhost]];
  newhost
 );

toGV[host_, xdisplay_] :=(
  newhost = remotehost[host];

  {rshuser, rshhost} =
    If[Length[atposn = StringPosition[newhost, "@"]] > 0,
     {StringTake[newhost, {1,atposn[[1]][[1]]-1}],
	StringTake[newhost, {atposn[[1]][[2]]+1,StringLength[newhost]}]},
     {"", newhost}];

  (* Return our rsh command if any, else the empty string. *)
  If[newhost != "",
     StringJoin["rsh ", rshhost, 
		If[StringLength[rshuser] > 0, " -l ", ""], rshuser,
		" /bin/sh -c \\''DISPLAY=", xdisplay, "; export DISPLAY; "],
     ""
  ]
);

Geomview[ name_String, gobj_, options___] := (
   WriteOut["", gobj, StringJoin[" -togeomview ",name," ",
	GeomviewCommand /. {options} /. Options[Geomview]], 
         toGV[DisplayHost /. {options} /. Options[Geomview],
	      XDisplay /. {options} /. Options[Geomview]],
      options];
   gobj
);

Geomview[ gobj_, options___] := (
  Geomview[ "Mathematica", gobj, options ]; gobj
);

bpref = BinPrefix[MachType /. Options[Geomview]];

If[Not[StringQ[OnPathQ["math2oogl"]]],
  (Print["Warning: the program 'math2oogl' is not on your search path,\n",
	Environment["PATH"], "\n"];
   If[ FileType[StringJoin[bpref, "/math2oogl"]]===File,
      Print["using default path (in OOGL.m) of ", bpref, " instead."],
	Print[
"the Mathematica->geomview graphics package won't be able to work;\n
change the value of $Geomroot in OOGL.m, or ensure that the math2oogl programs\n
are installed somewhere on your search path."]
    ])
];

End[]
EndPackage[]

SetOptions[Geomview, DisplayHost -> OOGL`Private`remotehost[""] ];
