
EXTRA_DIST = coot.tex derivatives.tex tutorial.tex coot-plain-2.ps \
	function-filter.scsh functions.tex derivatives-part.tex \
	crib-sheet.tex coot-user-manual.texi \
	3-amino-benzamide-2.eps  3-amino-benzamide-2.png  \
        xml2texi.xsl

# why doesn't this pick up the script for the dist tar file?  I don't know. 
# let's give up and add xml2texi.xsl to EXTRA_DIST
noinst_SCRIPTS = $(top_srcdir)/doc/xml2texi.xsl

coot-user-manual.dvi: $(top_srcdir)/doc/coot-user-manual.texi c-interface_8h.texi cc-interface_8hh.texi \
	              coot-scheme-functions.texi
	if [ ! -e coot-plain-2.eps ] ; then cp $(top_srcdir)/doc/coot-plain-2.ps coot-plain-2.eps ; fi
	texi2dvi $<

coot-user-manual.ps: coot-user-manual.dvi
	dvips -o $@ $^

coot-user-manual.pdf: coot-user-manual.ps
	ps2pdf $^

coot-user-manual.html: $(top_srcdir)/doc/coot-user-manual.texi
	texi2any --html $^
	texi2any --html --no-split $^


# splitstr='-split_chapter' ; \
# texi2any --html --version ; \
# if [ $$? = 0 ] ; then splitstr='-split=chapter' ; fi ; \
# texi2any --html $$splitstr $^ ; \
# cd ..

# old target
coot.html: $(top_srcdir)/doc/coot-user-manual.texi
	makeinfo --html --no-split --force $^
	makeinfo --html            --force $^

# and similarly for the coot FAQ
# makeinfo --html --no-split --force $(top_srcdir)/doc/coot-faq.texi


# old target
coot.dvi: $(top_srcdir)/doc/coot.tex
	if [ ! -e coot-plain-2.ps ] ; then cp $(top_srcdir)/doc/coot-plain-2.ps . ; fi
	-latex -interaction=nonstopmode $^
	-makeindex coot
	-latex -interaction=nonstopmode $^
	latex -interaction=nonstopmode $^

booklet.ps: coot-user-manual.dvi
	dvips $^ -f | psbook | psnup -2 | psset -t > $@


# these files are included by coot-user-manual.texi
#
# we need doxygen to have been run in the $(top_srcdir)/src directory
#
c-interface_8h.texi: $(top_srcdir)/src/doxygen/xml/c-interface_8h.xml
	xsltproc $(top_srcdir)/doc/xml2texi.xsl $< > $@

cc-interface_8hh.texi: $(top_builddir)/src/doxygen/xml/cc-interface_8hh.xml
	xsltproc $(top_srcdir)/doc/xml2texi.xsl $< > $@

coot-scheme-functions.texi:
	guile -s $(top_srcdir)/scheme/snarf-coot-docs.scm $(top_srcdir)/scheme

functions.tex: $(top_srcdir)/src/c-interface.h
# scsh -s $(top_srcdir)/doc/function-filter.scsh $^ > $@
	echo passing on making function.tex

functions.texi: $(top_srcdir)/src/c-interface.h
# scsh -s $(top_srcdir)/doc/function-filter.scsh $^ > $@
	echo passing on making functions.texi

# reference.dvi: $(top_srcdir)/doc/reference.tex functions.tex
# 	-latex -interaction=nonstopmode $^
# 	latex -interaction=nonstopmode $^

# coot-reference-manual.dvi: $(top_srcdir)/doc/coot-reference-manual.texi 
# 	texi2dvi $<

# coot-reference-manual.html: $(top_srcdir)/doc/coot-reference-manual.texi 
# 	texi2any --html $<

# reference-booklet.ps: reference.dvi
# 	dvips $^ -f | psbook | psnup -2 | psset -t > $@

crib-sheet.ps: crib-sheet.dvi
	dvips -o $@ $^

crib-sheet.pdf: crib-sheet.ps
	ps2pdf $^

coot-keys-and-buttons.pdf: crib-sheet.pdf
	cp $^ $@

crib-sheet.dvi: $(top_srcdir)/doc/crib-sheet.tex
	latex $^

tutorial.dvi: $(top_srcdir)/doc/tutorial.tex
	-latex -interaction=nonstopmode $^
	latex -interaction=nonstopmode $^

# this relies on the image directory being in place with ps images in it:
tutorial.ps: tutorial.dvi
	dvips -o $@ $^

tutorial.pdf: tutorial.ps
	ps2pdf $^


tutorial-2.dvi: $(top_srcdir)/doc/tutorial-2.tex
	-latex -interaction=nonstopmode $^
	latex -interaction=nonstopmode $^

# this relies on the image directory being in place with ps images in it:
tutorial-2.ps: tutorial-2.dvi
	dvips -o $@ $^

tutorial-2.pdf: tutorial-2.ps
	ps2pdf $^

