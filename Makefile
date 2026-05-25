# Makefile for vid-tag app

# --------------------
# Remove a dup tag on remote
# git push origin --delete vN.N

# --------------------
# Macros
SHELL = /bin/bash
cVer = 2.3.3

mInstallDir = ~/bin

mDepPkg = \
	ffmpeg \
	imagemagick \
	libimage-exiftool-perl \
	shellcheck \
	tidy

mReqProg = \
	awk \
	convert \
	exiftool \
	ffmpeg \
	sed \
	shfmt \
	shellcheck \
	tidy \
	tr \
	config/bash-fmt \
	config/bash-lint \
	config/rm-trailing-sp \
	./bash-com.inc \
	./bash-com.test \
	./shunit2.1 \
	./vid-tag \
	./vid-tag.inc \
	./vid-tag.test

mExternal = \
	bash-com.inc \
	bash-com.test \
	config/bash-fmt \
	config/bash-lint \
	config/rm-trailing-sp \
	shunit2.1

cRelServer = moria.whyayh.com
cRelDIr = /rel/released/software/own/vid-tag/

# ----------------------------------------
# Main targets

# --------------------
usage :
	@echo 'Usage:'
	@echo 'clean      - remove all backup files'
	@echo 'dist-clean - remove all built files (when done)'
	@echo 'check-ext  - check for newer external files (optional)'
	@echo 'required-prog - get and check for required prog (first-time)'
	@echo 'git-config - Setup git for CI/CD and commit checks (first-time)'
	@echo 'build      - Update in files'
	@echo 'test       - Quick tests (about 10sec)'
	@echo 'test-all   - Test with video files; slow (about 16min)'
	@echo 'get-test   - files needed for test-all (on-time)'
	@echo 'install    - local install (optional)'
	@echo 'package    - create the package zip file'
	@echo 'package-test - create the test package zip file'
	@echo 'get-test   - Get video files for tests 4.4GB'
	@echo 'release    - copy package to release server'
	@echo 'release-test - copy test package to release server'

# --------------------
clean :
	-find . -type f -name '*~' -exec rm {} \;
	-rm pod2htmd.tmp

dist-clean : clean
	rm MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	rm -rf pkg

# --------------------
check-ext : $(mExternal)

bash-com.inc : ~/bin/bash-com.inc
	'cp' -i $? $@

bash-com.test : ~/bin/bash-com.test
	'cp' -i $? $@

shunit2.1 : ~/bin/shunit2.1
	'cp' -i $? $@

config/rm-trailing-sp : ~/bin/rm-trailing-sp
	'cp' -i $? $@

config/bash-fmt : ~/bin/bash-fmt
	'cp' -i $? $@

# --------------------
required-prog : install-prog
	@for i in $(mReqProg); do \
		if ! which $$i >/dev/null 2>&1; then \
			echo "Error: missing $$i"; \
			exit 1; \
		fi; \
	done

install-prog : /usr/local/bin/shfmt
	@for i in $(mDepPkg); do \
		if ! dpkg -l $$i >/dev/null 2>&1; then \
			sudo apt-get install -y $$i; \
		fi; \
	done

/usr/local/bin/shfmt :
	curl -u "guest:guest" -O https://$(cRelServer)//rel/archive/software/ThirdParty/shfmt/shfmt_v3.10.0_linux_amd64
	chmod a+rx shfmt_*
	sudo chown root:root shfmt_*
	sudo mv -f shfmt_* /usr/local/bin
	sudo ln -sf /usr/local/bin/shfmt_v3.10.0_linux_amd64 /usr/local/bin/shfmt

# --------------------
git-config : .gitattributes .git/hooks/pre-commit
	if ! grep -q 'path = ../config/gitconfig.includes' .git/config; then \
		echo '[include]' >>.git/config; \
		echo '    path = ../config/gitconfig.includes' >>.git/config; \
	fi

.gitattributes : config/gitattributes
	cp $? $@

.git/hooks/pre-commit : config/pre-commit
	cp $? $@

# --------------------
build : README.html
	for i in vid-tag vid-tag.inc vid-tag.test README.md README.html; do \
		sed -i -e 's/cVer=[0-9.]*/cVer=$(cVer)/' $$i; \
	done
	#git ci -am Updated

README.html : README.md Makefile
	-markdown $? >$@
	-tidy -m -config ./tidyxhtml.conf $@

README.md : vid-tag vid-tag.inc Makefile
	-./vid-tag -H md >README.md

# --------------------
test : MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	./vid-tag.test -T fast
	./vid-tag -n -e testevent MVI_0107.MP4  MVI_0110.MP4  MVI_0746.MP4
	@echo "Review: vid-tag.conf"
	@echo "Review: vid-tag-example.txt"

test-all : MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	./vid-tag.test -T all

# --------------------
install : build
	cp -i vid-tag vid-tag.inc vid-tag.test bash-com.inc bash-com.test $(mInstallDir)

package : build pkg pkg/vid-tag-$(cVer).zip
	-git push --tags --force origin develop

pkg :
	mkdir -p $@

pkg/vid-tag-$(cVer).zip :
	zip $@ README.html LICENSE vid-tag vid-tag.inc vid-tag.conf bash-com.inc

package-test : package pkg/vid-tag-test-$(cVer).zip pkg/vid-tag-test-input.zip

pkg/vid-tag-test-$(cVer).zip :
	zip $@ vid-tag.test bash-com.test shunit2.1

# --------------------
get-test : pkg
	cd pkg; curl -O https://$(cRelServer)$(cRelDIr)/vid-tag-test-input.zip
	unzip pkg/vid-tag-test-input.zip

pkg/vid-tag-test-input.zip : MVI_0107.MP4 MVI_0110.MP4 MVI_0746.MP4
	zip $@ $^

MVI_0107.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP $(cRelServer):/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0110.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP $(cRelServer):/home/video/ver/video/studio/portfolio/raw/$@ $@

MVI_0746.MP4 :
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP $(cRelServer):/rel/archive/video/project/uucc/2026/2026-03-01/raw/cover/$@ $@

# --------------------
release : package
	git tag -f v$(cVer)
	git push --tags --force origin develop
	git co main
	git merge develop
	git push origin main
	git co develop
	read -p "You must have a user on moria. ^c to quit"
	-ssh $(cRelServer) mkdir --mode=755 -p $(cRelDIr)
	rsync -aP README.html pkg/vid-tag-$(cVer).zip \
		$(cRelServer):$(cRelDIr)

release-test : package-test
	read -p "You must have a user on moria. ^c to quit"
	rsync -aP pkg/vid-tag-test-$(cVer).zip \
		$(cRelServer):$(cRelDIr)

#		pkg/vid-tag-test-input.zip \
