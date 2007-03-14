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
 GVreplaceconst.sl GVreplaceplastic.sl

if AQSIS
geomshaderdata_DATA += $(shader_src:.sl=.slx)

.sl.slx:
	$(AQSL) -o $@ $<

clean-local:
	-rm -f *.slx
endif
