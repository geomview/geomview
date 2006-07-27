{ # World
  {INST  unit {
      INST   geom {{ LIST # World list 
          {# cube1 - blue / purple
            INST   geom {appearance {
                material {
                  ambient 0.494118 0 0.894118
                  diffuse 0.494118 0 0.894118
                } 
              }
              geom INST
              tlist { = TLIST
                1             0             0             0
                0             1             0             0
                0             0             1             0
                0             0             0             1
                
              }
              geom { = INST
                tlist { = TLIST
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0            -1             1
                  
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0             1             1
                  
                }
                geom < /u/fowler/sosgv/maniview/torus_open/cubesides.0081
              }
          } } # end (geom and INST) cube1
          { # cube2 - green
            INST   geom {appearance {
                material {
                  ambient 0.094118 0.647059 0.400000
                  diffuse 0.094118 0.847059 0.600000
                }
              }
              geom INST
              tlist { = TLIST
                0             0            -1             0
                0             1             0             0
                1             0             0             0
                0             0             0             1
                
              }
              geom { = INST
                tlist { = TLIST
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0            -1             1
                  
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0             1             1
                  
                }
                geom < /u/fowler/sosgv/maniview/torus_open/cubesides.0081
              }
          } } # end (geom and INST) cube2
          { # cube3 - yellow
            INST   geom {appearance {
                material {
                  ambient 0.894118 0.600000 0.000000
                  diffuse 0.894118 0.600000 0.000000
                }
              }
              geom INST
              tlist { = TLIST
                1             0             0             0
                0             0             1             0
                0            -1             0             0
                0             0             0             1
                
              }
              geom { = INST
                tlist { = TLIST
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0            -1             1
                  
                  1             0             0             0
                  0             1             0             0
                  0             0             1             0
                  0             0             1             1
                  
                }
                geom < /u/fowler/sosgv/maniview/torus_open/cubesides.0081
              }
          } } # end (geom and INST) cube3
        } #end of World List
    } } 
    transform
    .5 0 0 0
    0 .5 0 0
    0 0 .5 0 
    0 0 0 1
} } # end of World and INST
