Libraries Used for the Build

- CGAL 5.6.1
- Boost 1.87
- iconv
- muparser
- qt5
- nlohmann_json
- spdlog
- python 3.12
- qtpropertybrowser
  --> https://github.com/bazhenovc/QtPropertyBrowser (for qt5)
  --> https://github.com/AubrCool/qtpropertybrowser (for qt6)

- exprtk
  --> https://github.com/ArashPartow/exprtk


Built in the MSYS2 (Windows) environment using the following commands:
rm -rf build
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

Created qt.conf in the same directory as the executable with the following content:
Content of qt.conf:
[Paths]
Plugins=platforms

DLL auto-copy script
Executed the following command in the directory containing the executable using the MSYS2 MINGW64 console:
for dll in $(ldd bzmagEditor.exe | grep '=>' | awk '{print $3}'); do cp $dll ./platforms/; done

DLL required for UI styles
The following DLL must be placed in the styles directory under the executable directory:
styles\qwindowsvistastyle.dll