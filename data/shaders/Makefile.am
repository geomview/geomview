geomshaderdatadir=${geomdatadir}/shaders

#
# Old pre-compiled shaders, probably BMRT(?)
#
geomshaderdata_DATA = hlight.slo hmatte.slo hplastic.slo

shader_src = \
 hlight.sl hmatte.sl hplastic.sl \
 GVblendconstant.sl GVblendplastic.sl \
 GVdecalconstant.sl GVdecalplastic.sl \
 GVmodulateconstant.sl GVmodulateplastic.sl \
 GVreplaceconstant.sl GVreplaceplastic.sl

if AQSIS
geomshaderdata_DATA += $(shader_src:.sl=.slx)

.sl.slx:
	$(AQSL) -o $@ $<

endif

if PIXIE
geomshaderdata_DATA += $(shader_src:.sl=.sdr)

.sl.sdr:
	$(SDRC) -o $@ $<

endif

if DELIGHT
geomshaderdata_DATA += $(shader_src:.sl=.sdl)

#
# SDLFLAGS = -O3 --embed-source --dso etc.
#
.sl.sdl:
	$(SHADERDL) $(SDLFLAGS) -o $@ $<

endif

clean-local:
	-rm -f *.slx *.sdr
