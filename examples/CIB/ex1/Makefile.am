## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/Make-rules

## Dimension-dependent testers
SOURCES = main.cpp
EXTRA_DIST  = input3d shell_3d_in.vertex shell_3d_out.vertex petsc_options.dat

EXTRA_PROGRAMS =
if SAMRAI3D_ENABLED
EXTRA_PROGRAMS += main3d
endif

main3d_CXXFLAGS = $(AM_CXXFLAGS) -DNDIM=3
main3d_LDADD = $(IBAMR_LDFLAGS) $(IBAMR3d_LIBS) $(IBAMR_LIBS)
main3d_SOURCES = $(SOURCES)

examples: $(EXTRA_PROGRAMS)
	if test "$(top_srcdir)" != "$(top_builddir)" ; then \
	  cp -f $(srcdir)/input3d $(PWD) ; \
	  cp -f $(srcdir)/*.vertex $(PWD) ; \
	  cp -f $(srcdir)/petsc_options.dat $(PWD) ; \
	fi ;

clean-local:
	rm -f $(EXTRA_PROGRAMS)
	if test "$(top_srcdir)" != "$(top_builddir)" ; then \
	  rm -f $(builddir)/input3d ; \
          rm -f $(builddir)/*.vertex ; \
          rm -f $(builddir)/petsc_options.dat ; \
	fi ;


