#!/usr/bin/env bash

usage="Usage: ${FUNCNAME[0]} [-u git_user] [-e git_email] [-m commit_message]"

git_user="github-actions"
git_email="github-actions@github.com"
git_msg="generated"

while getopts "e:m:u:" OPTION; do
case "${OPTION}" in
e)
    git_email=${OPTARG}
    ;;
m)
    git_msg=${OPTARG}
    ;;
u)
    git_user=${OPTARG}
    ;;
*)
    echo "$usage" 1>&2
    exit 1
    ;;
esac
done
shift $((OPTIND-1))

#Exit on failures
set -e
set -x

# Check to see if there are any changes
set +e
git commit -m "$git_msg" --dry-run
err=$?
if [ $err = 0 ]; then
    echo committing and pushing changes
    set -e
    git config user.name "$git_user"
    git config user.email "$git_email"
    git commit -m "$git_msg"
    git show
    git push
fi
set -e

