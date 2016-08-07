

pushd /da/sw/troen/build



D=$1
C=$2
shift
shift

VTKLIBPATH=/da/sw/vtk/install/lib/vtk-5.10

cd $D
# we need to make sure that the paths for this version of omegalib are found first
export PATH=$D:$PATH
LD_LIBRARY_PATH=/da/sw/omegalib/omegalib.dev/build/bin:/da/sw/troen/3rdParty/fmod/api/lib:/da/sw/troen/3rdParty/fmod/fmoddesignerapi/api/lib:/da/sw/troen/3rdParty/libzeug/lib:/da/sw/troen/3rdParty/RakNet/Lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$D:${D}/osg:${D}/osg/osgPlugins-3.3.0:$LD_LIBRARY_PATH
export OMEGA_HOME=$D/../../core

# -r is for remote log
# gdb -x gdbcommand.txt --args  $C -r --log v -platform offscreen --troen-remote-start $*
$C -r --log v -platform offscreen --troen-remote-start $*

echo $C 
# $C $*


stty sane
