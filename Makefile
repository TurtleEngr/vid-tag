# Makefile for vid-tag app

# --------------------
# Macros
cVer = 1.1

# --------------------
# Main targets

usage:
	@echo 'Usage:'
	@echo 'clean      - remove all backup files'
	@echo 'dist-clean - remove all built files'
	@echo 'update     - check for newer dependent files'
	@echo 'build      - Update README and update cVer in files'
	@echo 'package    - create the package zip file'
	@echo 'release    - copy package to release server'
	@echo 'package-test - create the test package zip file'
	@echo 'release-test - copy test package to release server'

clean :
	find . -type f -name '*~' -exec rm {} \;

dist-clean : clean
	rm MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	rm -rf pkg

update : check-bash-com.inc check-bash-com.test check-shunit2.1

build : README.md
	for i in vid-tag vid-tag.inc vid-tag.test; do \
		sed -i -e 's/cVer=[0-9.]*/cVer=$(cVer)/' $$i; \
	done

package : build pkg pkg/vid-tag-$(cVer).zip

package-test : package pkg/vid-tag-test-input.zip

release : package
	@echo "You must have a user on moria"
	git tag -f v$(cVer)
	-ssh moria mkdir --mode=755 -p /rel/released/software/own/vid-tag/
	rsync -aP pkg/vid-tag-$(cVer).zip moria:/rel/released/software/own/vid-tag/

release-test : release package-test
	@echo "You must have a user on moria"
	git tag -f v$(cVer)
	rsync -aP pkg/vid-tag-test-input.zip moria:/rel/released/software/own/vid-tag/

# --------------------
# Single targets

README.md : vid-tag vid-tag.inc
	-./vid-tag -H md >README.md

check-bash-com.inc : ~/bin/bash-com.inc
	-diff bash-com.inc $?

check-bash-com.test : ~/bin/bash-com.test
	-diff bash-com.test $?

check-shunit2.1 : ~/bin/shunit2.1
	-diff shunit2.1 $?

pkg :
	mkdir -p $@

pkg/vid-tag-$(cVer).zip :
	zip $@ vid-tag vid-tag.inc bash-com.inc

pkg/vid-tag-test-$(cVer).zip :
	zip $@ vid-tag.test bash-com.test shunit2.1

pkg/vid-tag-test-input.zip : MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	zip $@ $^

MVI_0107.MP4 :
	@echo "You must have a user on moria"
	rsync -aP moria:/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0110.MP4 :
	@echo "You must have a user on moria"
	rsync -aP moria:/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0746.MP4 :
	@echo "You must have a user on moria"
	rsync -aP moria:/rel/archive/video/project/uucc/2026/2026-03-01/raw/cover/$@ $@
