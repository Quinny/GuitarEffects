git fetch
git reset --hard HEAD
update_status=$(git merge origin/master)
if ["${update_status}" = "Already up to date."]; then
  echo "Already up to date, not rebuilding";
else
  make server
fi
