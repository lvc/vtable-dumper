Vtable Dumper
=============

Vtable-Dumper — a tool to list content of virtual tables in a C++ shared library.

Contents
--------

1. [ About   ](#about)
2. [ Install ](#install)
3. [ Usage   ](#usage)

About
-----

The tool is intended for developers of software libraries and maintainers of Linux distributions who are interested in ensuring backward binary compatibility.

The tool is developed by Andrey Ponomarenko.

Install
-------

    sudo make install prefix=/usr

###### Requires

* libelf
* libdl
* libstdc++

Usage
-----

    vtable-dumper SHLIB

###### Example

    vtable-dumper /usr/lib64/libstdc++.so.6

###### Note

  Make sure that all dependencies of a target library can be found by `ldd`. Otherwise
  `dlopen()` may fail. Add paths to these libraries to `LD_LIBRARY_PATH` in this case.

###### Options

| Option       | Desc                         |
|--------------|------------------------------|
| -mangled     | Show mangled symbol names    |
| -demangled   | Show de-mangled symbol names |
| -help        | Display this help message    |
| -dumpversion | Print the tool version       |

###### Sample output

    Vtable for QIconEnginePlugin
    _ZTV17QIconEnginePlugin: 22 entries
    0     (int (*)(...)) 0
    8     (int (*)(...)) (& _ZTI17QIconEnginePlugin)
    16    (int (*)(...)) QIconEnginePlugin::metaObject() const
    24    (int (*)(...)) QIconEnginePlugin::qt_metacast(char const*)
    32    (int (*)(...)) QIconEnginePlugin::qt_metacall(QMetaObject::Call, int, void**)
    40    (int (*)(...)) QIconEnginePlugin::~QIconEnginePlugin()
    48    (int (*)(...)) QIconEnginePlugin::~QIconEnginePlugin()
    56    (int (*)(...)) QObject::event(QEvent*)
    64    (int (*)(...)) QObject::eventFilter(QObject*, QEvent*)
    72    (int (*)(...)) QObject::timerEvent(QTimerEvent*)
    80    (int (*)(...)) QObject::childEvent(QChildEvent*)
    88    (int (*)(...)) QObject::customEvent(QEvent*)
    96    (int (*)(...)) QObject::connectNotify(char const*)
    104   (int (*)(...)) QObject::disconnectNotify(char const*)
    112   (int (*)(...)) __cxa_pure_virtual
    120   (int (*)(...)) __cxa_pure_virtual
    128   (int (*)(...)) -0x00000000000010
    136   (int (*)(...)) (& _ZTI17QIconEnginePlugin)
    144   (int (*)(...)) _ZThn16_N17QIconEnginePluginD1Ev
    152   (int (*)(...)) _ZThn16_N17QIconEnginePluginD0Ev
    160   (int (*)(...)) __cxa_pure_virtual
    168   (int (*)(...)) __cxa_pure_virtual
