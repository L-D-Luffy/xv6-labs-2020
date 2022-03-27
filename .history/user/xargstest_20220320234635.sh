mkdir a
echo hello > a/b
mkdir c
echo hello > c/b
echo hello > b
find . b
find . b | xargs echo 
find . b | xargs grep hello
