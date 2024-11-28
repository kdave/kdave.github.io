TOPUSH=../to-push

all: build

aio: build commit push sync commitsync pushsync

build:
	jekyll build

commit:
	git add .
	git status
	-git commit -ave -m'Updates'

sync:
	cd $(TOPUSH) && pwd
	cd $(TOPUSH) && rm -rf -- *
	cp -a _site/. $(TOPUSH)

push:
	git push -f gh master-source

commitsync:
	cd $(TOPUSH) && pwd
	cd $(TOPUSH) && git add * && git commit -a -m'sync'

pushsync:
	cd $(TOPUSH) && pwd
	cd $(TOPUSH) && git push -f gh master

local:
	jekyll serve --strict_front_matter --livereload --drafts --unpublished
