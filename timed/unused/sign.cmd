if exist d:\timed\out.txt del d:\timed\out.txt
cd\pgp
d:\pgp\pgp -sta %1 -o d:\timed\out.txt
cd\timed
copy out.txt timed.msg
pause
