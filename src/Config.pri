# see http://merkaartor.be/wiki/merkaartor/Compiling

isEmpty(VERSION): VERSION="0.18"

contains(RELEASE,1) {
    DEFINES += RELEASE
    SVNREV="release"
} else {
    isEmpty(SVNREV) {
        SVNREV = $$system(git describe --tags)
        REVISION=SVNREV
    } else {
        REVISION="-git"
    }
}

win32|macx {
    system(echo $${LITERAL_HASH}define SVNREV $${SVNREV} > revision.h )
} else {
    system('echo -n "$${LITERAL_HASH}define SVNREV $${SVNREV}" > revision.h')
}
