(* Author: Silvio Levy *)

BeginPackage["BezierPlot`"]

BezierPatch::usage = "\n
  BezierPatch[ 4 by 4 array ] describes a bicubic Bezier patch\n
  to be plotted using OOGL.m, for example.  The elements of the\n
  array are usually 3D vectors, but the function doesn't care."

BezierPlot::usage = "\n
  BezierPlot[f,{t,tmin,tmax},{u,umin,umax}] \"plots\" the function f\n
  of two variables t and u, using Bezier patches.  (Usually f returns\n
  a 3D vector.)  BezierPlot returns an array of BezierPatch'es.\n
  Options:\n
    Epsilon is the increment used to compute derivatives (default: 10^-4)\n
    PlotPoints is a number or pair of numbers indicating the fineness\n
      of the subdivision (default: 5)";

BezierPlot::baditer = "bad iterator";
BezierPlot::badpp = "requested number of plot points is < 1";

Options[BezierPlot] = {Epsilon -> .0001, PlotPoints->5}
  
Begin["`private`"]

BezierPlot[f_,{t_,tmin_,tmax_},{u_,umin_,umax_},options___]:=
Block[{it,iu,nt,nu,dt,du,loc,dfdt,dfdt0,dfdu,dfdu0,dboth,all,
      plotPoints=PlotPoints/.{options}/.Options[BezierPlot],
      eps=Epsilon/.{options}/.Options[BezierPlot]},
  If[Length[plotPoints]==2,{nt,nu}=plotPoints,nt=plotPoints;nu=plotPoints];
  If[nx<1 || ny<1,Message[BezierPlot::badpp];Return[{}]];
  dt=N[(tmax-tmin)/nt]; du=N[(umax-umin)/nu];
  If[!(NumberQ[dt]&&dt>0&&NumberQ[du]&&du>0),
    Message[BezierPlot::baditer];Return[{}]];
  loc=Table[f/.{t->tmin+dt it,u->umin+du iu},{iu,0,nu},{it,0,nt}];
  dfdt0=-loc+Table[f/.{t->tmin+dt(it+If[it<nt,1,-1]eps),u->umin+du iu},
                   {iu,0,nu},{it,0,nt}];
  dfdt=MapAt[-1#&,#,-1]& /@ dfdt0/(3 eps);
  dfdu0=-loc+Table[f/.{t->tmin+dt it,u->umin+du(iu+If[iu<nu,1,-1]eps)},
                   {iu,0,nu},{it,0,nt}];
  dfdu=MapAt[-1#&,#,-1]& @ dfdu0/(3 eps);
  dboth=MapAt[-1#&,#,-1]& /@ MapAt[-1#&,#,-1]& @ 
        (-loc-dfdt0-dfdu0+Table[f/.{t->tmin+dt(it+If[it<nt,1,-1]eps),
                                     u->umin+du(iu+If[iu<nu,1,-1]eps)}, 
                                {iu,0,nu},{it,0,nt}])/(9 eps^2);
  all=Transpose[{loc,dfdt,dfdu,dboth},{3,1,2}]//N;
  Map[
  BezierPatch[
  {{#[[1,1]],#[[1,1]]+#[[1,2]],#[[2,1]]-#[[2,2]],#[[2,1]]},
  {#[[1,1]]+#[[1,3]],#[[1,1]]+#[[1,2]]+#[[1,3]]+#[[1,4]],
    #[[2,1]]-#[[2,2]]+#[[2,3]]-#[[2,4]],#[[2,1]]+#[[2,3]]},
  {#[[3,1]]-#[[3,3]],#[[3,1]]+#[[3,2]]-#[[3,3]]-#[[3,4]],
   #[[4,1]]-#[[4,3]]-#[[4,2]]+#[[4,4]], #[[4,1]]-#[[4,3]]},
  {#[[3,1]],#[[3,1]]+#[[3,2]],#[[4,1]]-#[[4,2]],#[[4,1]]}}]&,
    Transpose[{Drop[#,-1]&/@ Drop[#,-1]& @ all, Rest/@Drop[#,-1]& @ all,
                    Drop[#,-1]&/@ Rest @ all, Rest/@Rest @ all}, {3,1,2}],{2}]
]

Format[x_BezierPatch]:="-BezierPatch-"

End[]
EndPackage[]
