#!/usr/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SOURCE_ROOT=$SCRIPT_DIR/../..
pushd $SCRIPT_DIR

function error_out {
    local code message
    local "${@}"

    echo $message 1>&2
    exit $code
}

TMPDIR=$(mktemp -d MODULEMD_XXXXXX --tmpdir)

function common_finalize {
    exitcode=$?

    rm -Rf $TMPDIR

    return $exitcode
}

trap common_finalize EXIT

hub --version >/dev/null || error_out code=127 message="Install 'hub' to use this script"
jq --version >/dev/null || error_out code=127 message="Install 'jq' to use this script"

# Make sure the current user has logged in before or prompt them for credentials
echo "Logging into Github"
hub api "https://api.github.com/user" || error_out code=1 message="Invalid credentials"

# Get the previous tag for this branch
OLDTAG=$(git describe --first-parent --abbrev=0)

# Configure the build directory
meson --buildtype=release -Dskip_formatters=true -Ddeveloper_build=false $TMPDIR
MMD_SKIP_VALGRIND=True ninja -C $TMPDIR dist

# Get the version that will be tagged
NEWVERSION=$(meson introspect $TMPDIR --projectinfo |jq -r ".version")
NEWTAG=libmodulemd-$NEWVERSION

if [ $NEWTAG = $OLDTAG ]; then
    error_out code=2 message="Version is already tagged. Update meson.build with the new version."
fi

echo ==========
echo Tagging $NEWTAG
echo ==========

echo Tagging $NEWTAG > $TMPDIR/tag_header
echo >> $TMPDIR/tag_header
git shortlog $OLDTAG.. >> $TMPDIR/shortlog || error_out code=3 message="Couldn't find the previous tag"
cat $TMPDIR/tag_header $TMPDIR/shortlog > $TMPDIR/tag_message

git tag -s -F $TMPDIR/tag_message $NEWTAG || error_out code=4 message="Couldn't create new signed tag for the release"

# meson 0.50.0 and later can modify the version field programmatically
if [ $($SCRIPT_DIR/semver compare 0.50.0 $(meson --version)) = -1 ]; then
    bump_version=$($SCRIPT_DIR/semver bump patch $NEWVERSION)
    pushd $SOURCE_ROOT
    meson rewrite kwargs set project / version $bump_version #|| error_out code=11 message="Couldn't bump the version in meson.build"

    git commit -sm "Bump version in meson.build to $bump_version" meson.build
    popd #$SOURCE_ROOT
fi

# Make sure everything is up-to-date on Github
git push --follow-tags || error_out code=5 message="Couldn't push the new tags to Github"

echo "libmodulemd $NEWVERSION" > $TMPDIR/github_message
echo >> $TMPDIR/github_message
sed -E "s/      / * /g" $TMPDIR/shortlog | sed  -E "s/(.*):\$/\# \1/g" >> $TMPDIR/github_message

hub release create -a $TMPDIR/meson-dist/modulemd-$NEWVERSION.tar.xz \
                   -a $TMPDIR/meson-dist/modulemd-$NEWVERSION.tar.xz.sha256sum \
                   -F $TMPDIR/github_message \
                   $NEWTAG || error_out code=10 message="Couldn't publish the release"

