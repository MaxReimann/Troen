
LD_LIBRARY_PATH=/da/sw/omegalib/omegalib.dev/build/bin:/da/sw/troen/3rdParty/fmod/api/lib:/da/sw/troen/3rdParty/fmod/fmoddesignerapi/api/lib:/da/sw/troen/3rdParty/libzeug/lib:/da/sw/troen/3rdParty/RakNet/Lib:$LD_LIBRARY_PATH

pushd /da/sw/troen/build

./Troen "$@"

stty sane
