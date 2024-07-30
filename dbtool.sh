while getopts "u:p" option; do
case "$option" in
u) unpack="y";;
p) pack="y";;
esac
done
