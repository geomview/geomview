geomshaderdatadir=${geomdatadir}/shaders

#
# Old pre-compiled shaders, probably BMRT(?)
#
dist_geomshaderdata_DATA = hlight.slo hmatte.slo hplastic.slo

shader_src = \
 hlight.sl hmatte.sl hplastic.sl \
 GVblendconstant.sl GVblendplastic.sl \
 GVdecalconstant.sl GVdecalplastic.sl \
 GVmodulateconstant.sl GVmodulateplastic.sl \
 GVreplaceconstant.sl GVreplaceplastic.sl

EXTRA_DIST = $(shader_src) # hlight.slo hmatte.slo hplastic.slo

if AQSIS_SHADER_COMPILER
dist_geomshaderdata_DATA += $(shader_src:.sl=.slx)

.sl.slx:
	$(AQSIS_SHADER_COMPILER) -o $@ $<

endif

if PIXIE_SHADER_COMPILER
dist_geomshaderdata_DATA += $(shader_src:.sl=.sdr)

.sl.sdr:
	$(PIXIE_SHADER_COMPILER) -o $@ $<

endif

if DELIGHT_SHADER_COMPILER
dist_geomshaderdata_DATA += $(shader_src:.sl=.sdl)

#
# SDLFLAGS = -O3 --embed-source --dso etc.
#
.sl.sdl:
	$(DELIGHT_SHADER_COMPILER) $(SDLFLAGS) -o $@ $<

endif

clean-local:
	-rm -f *.slx *.sdr *.sdl
