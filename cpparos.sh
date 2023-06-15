mkdir dr
doas mount /dev/sdb1 dr
doas cp $HOME/dev/ParOS/paros.iso dr
doas umount dr
rm -rf dr
