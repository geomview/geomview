(*
 * Geomview.m selects Geomview graphics as the default for Show
 * of 3D graphics objects.  Others still use the default system.
 * To restore normality, load <<GL.m or <<X11.m or etc.
 *)

Needs["OOGL`"];

ClearAll[$DisplayFunction];

$DisplayFunction[g_Graphics3D] := Geomview[g];
$DisplayFunction[g_SurfaceGraphics] := Geomview[g];
$DisplayFunction[g_] := Display[$Display, g];

If[ !MemberQ[$CommandLine, "-noprompt"],
   ( Print[" -- Geomview graphics initialized -- "];
     If[(DisplayHost /. Options[Geomview]) != "",
	Print["Geomview.m: graphics will display remotely on ",
		DisplayHost /. Options[Geomview]];
	Print["To change this, see DisplayHost in Options[Geomview]."]
      ]
   )
  ];

