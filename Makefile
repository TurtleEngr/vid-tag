# Makefile for vid-tag app

cVer = 1.1

clean :
	find . -type f -name '*~' -exec rm {} \;

build : README.md
	for i in vid-tag vid-tag.inc vid-tag.test; do \
		sed -i -e 's/cVer=[0-9.]+/cVer=$(cVer)/' $$i; \
	done

package : build pkg pkg/vid-tag-$(cVer).zip

package-test : build pkg pkg/vid-tag-test-$(cVer).zip pkg/vid-tag-test-input.zip

release : package
	git tag -f v$(cVer)

release-test : package-test
	git tag -f v$(cVer)

# --------------------
# Single targets

README.md : vid-tag vid-tag.inc
	-./vid-tag -e testevent -H md >README.md

pkg :
	mkdir -p $@

pkg/vid-tag-$(cVer).zip :
	zip pkg/$@ vid-tag vid-tag.inc bash-com.inc

pkg/vid-tag-test-$(cVer).zip :
	zip pkg/$@ vid-tag.test bash-com.test shunit2.1

pkg/vid-tag-test-input.zip :
	echo TBD

# --------------------
update : bash-com.inc bash-com.test shunit2.1

bash-com.inc : ~/bin/bash-com.inc
	echo "$@ needs to be updated."
	diff $@ $?

bash-com.test :  ~/bin/bash-com.test
	echo "$@ needs to be updated."
	diff $@ $?

shunit2.1 :  ~/bin/shunit2.1
	echo "$@ needs to be updated."
	diff $@ $?
