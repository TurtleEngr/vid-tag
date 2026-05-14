# Makefile for vid-tag app

# --------------------
# Remove a dup tag on remote
# git push origin --delete vN.N

# --------------------
# Macros
SHELL = /bin/bash
cVer = 1.3.3

# --------------------
# Main targets

usage :
	@echo 'Usage:'
	@echo 'clean      - remove all backup files'
	@echo 'dist-clean - remove all built files'
	@echo 'update     - check for newer dependent files'
	@echo 'build      - Update README and update cVer in files'
	@echo 'test       - Quick tests (about 7sec)'
	@echo 'test-all   - Test with video files; slow (about 16min)'
	@echo 'package    - create the package zip file'
	@echo 'release    - copy package to release server'
	@echo 'package-test - create the test package zip file'
	@echo 'release-test - copy test package to release server'

clean :
	-find . -type f -name '*~' -exec rm {} \;
	-rm pod2htmd.tmp

dist-clean : clean
	rm MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	rm -rf pkg

update check : check-bash-com.inc check-bash-com.test check-shunit2.1

build : README.html
	for i in vid-tag vid-tag.inc vid-tag.test; do \
		sed -i -e 's/cVer=[0-9.]*/cVer=$(cVer)/' $$i; \
	done
	-git ci -am Updated

test :
	./vid-tag.test -T fast
	./vid-tag -n -e testevent MVI_0107.MP4  MVI_0110.MP4  MVI_0746.MP4
	@echo "Review: vid-tag.conf"
	@echo "Review: vid-tag-example.txt"

test-all :
	./vid-tag.test -T all

package : build pkg pkg/vid-tag-$(cVer).zip
	-git push --tags --force origin develop

package-test : package pkg/vid-tag-test-$(cVer).zip pkg/vid-tag-test-input.zip
	-git push --tags origin develop

release : package
	git tag -f v$(cVer)
	git push --tags --force origin develop
	git co main
	git merge develop
	git push origin main
	git co develop
	read -p "You must have a user on moria. ^c to quit"
	-ssh moria mkdir --mode=755 -p /rel/released/software/own/vid-tag/
	rsync -aP README.html pkg/vid-tag-$(cVer).zip \
		moria:/rel/released/software/own/vid-tag/

release-test : package-test
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP pkg/vid-tag-test-$(cVer).zip \
		pkg/vid-tag-test-input.zip \
		moria:/rel/released/software/own/vid-tag/

install : build
	cp -i vid-tag vid-tag.inc vid-tag.test ~/bin/

# --------------------
# Single targets

README.md : vid-tag vid-tag.inc
	-./vid-tag -H md >README.md

README.html : README.md
	-markdown $? >$@
	-tidy -m -config ./tidyxhtml.conf $@

check-bash-com.inc : ~/bin/bash-com.inc
	-diff $? bash-com.inc | grep -v Revision:

check-bash-com.test : ~/bin/bash-com.test
	-diff $? bash-com.test | grep -v Revision:

check-shunit2.1 : ~/bin/shunit2.1
	-diff $? shunit2.1 | grep -v Revision:

pkg :
	mkdir -p $@

pkg/vid-tag-$(cVer).zip :
	zip $@ README.html LICENSE vid-tag vid-tag.inc vid-tag.conf bash-com.inc

pkg/vid-tag-test-$(cVer).zip :
	zip $@ vid-tag.test bash-com.test shunit2.1

pkg/vid-tag-test-input.zip : MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	zip $@ $^

MVI_0107.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP moria:/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0110.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP moria:/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0746.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP moria:/rel/archive/video/project/uucc/2026/2026-03-01/raw/cover/$@ $@

