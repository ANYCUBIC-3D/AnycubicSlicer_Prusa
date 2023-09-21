## AnycubicSlicer

AnycubicSlicer is an open-source, easy-to-use slicing software for FDM 3D printing. It makes 3D printing easier with reliable slicing algorithms and simplified workflow.AnycubicSlicer is based on [PrusaSlicer](https://github.com/prusa3d/PrusaSlicer) by Prusa Research, which is from [Slic3r](https://github.com/Slic3r/Slic3r) by Alessandro Ranellucci and the RepRap community.

### What are AnycubicSlicer's new features?
* New UI interface
* Simplified slice parameters

# linux compile 
## deps
```shell
cmake -S $(pwd)/deps -B  $(pwd)/deps/buildd
cmake --build $(pwd)/deps/buildd -j10
```
## build engine only
``` shell
cmake -S $(pwd) -B buildd -DCMAKE_PREFIX_PATH:PATH=$(pwd)/deps/buildd/destdir/usr/local -DSLIC3R_STATIC:BOOL=ON -DSLIC3R_GUI:BOOL=OFF -DENABLE_CGAL:BOOL=OFF -DCMAKE_INSTALL_PREFIX:PATH=$(pwd)/buildd/install
cmake --build $(pwd)/buildd -j10
```

# windows compile
## build
* vscode execute the following command to enable the compilation toolchain
```psl
Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"; Enter-VsDevShell 3449344a
```

### Automatically create projects
* -v Specify the creation visual studio version number
* -c Specify CMAKE_BUILD_TYPE
```bat
build_win.bat -d out_deps -c Debug -s all  -v 17
```


### Manual creation
* CMAKE_PREFIX_PATH must be an absolute path, otherwise it may not work
```bat
build_win.bat -d out_deps -c Debug -s deps  -v 17
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="\out_deps\usr\local"
```
or
```bat
mkdir deps/build
cd deps/build
set CL=/MP
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="..\..\out_deps" ..
nmake
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="\out_deps\usr\local"
```

### License

AnycubicSlicer is licensed under the GNU Affero General Public License, version 3. AnycubicSlicer is based on PrusaSlicer by PrusaResearch.

PrusaSlicer is licensed under the GNU Affero General Public License, version 3. PrusaSlicer is owned by Prusa Research. PrusaSlicer is originally based on Slic3r by Alessandro Ranellucci.

Slic3r is licensed under the GNU Affero General Public License, version 3. Slic3r was created by Alessandro Ranellucci with the help of many other contributors.

The GNU Affero General Public License, version 3 ensures that if you use any part of this software in any way (even behind a web server), your software must be released under the same license.

